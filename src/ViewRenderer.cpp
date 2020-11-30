#include "ViewRenderer.h"

#include<Urho3D/Urho3DAll.h>


ViewRenderer::ViewRenderer(Context* ctx, SharedPtr<BlenderSession> parent_, int id)
    : Object(ctx),
      fov_(0),
      viewId_(id),
      width_(0),
      height_(0),
      currentScene_(nullptr),
      renderTexture_(nullptr),
      orthosize_(0),
      orthoMode_(false),
      renderPath_("RenderPaths/Forward.xml"),
      ctx_(ctx),
      parent(parent_)
{
    //netId = "runtime-"+String(id);
    netId = "runtime-"+String(parent->GetSessionId())+"-"+String(id);
    SubscribeToEvent(E_BLENDER_SCENE_UPDATED,URHO3D_HANDLER(ViewRenderer,HandleSceneUpdate));
}

void ViewRenderer::SetScene(SharedPtr<Scene> scene)
{
    if (scene == currentScene_){
        // nothing to do
        return;
    }

    String cameraNodeName = "Camera-"+netId;
    viewportCameraNode_ = scene->GetChild(cameraNodeName,true);

    if (viewportCameraNode_){
        viewportCamera_ = viewportCameraNode_->GetComponent<Camera>();
    } else {
        viewportCameraNode_ = new Node(ctx_);
        viewportCameraNode_ = scene->CreateChild(cameraNodeName);
        viewportCamera_ = viewportCameraNode_->CreateComponent<Camera>();
        viewportCamera_->SetFarClip(500.0f);
    }

    currentScene_ = scene;
    currentScene_->SetUpdateEnabled(false);
    if (viewport_){
        viewport_->SetScene(scene);
        //renderSurface_->QueueUpdate();
    }
}

void ViewRenderer::SetOrthoMode(const Matrix4& vmat,float size_)
{
    auto t = vmat.Translation();
    auto r = vmat.Rotation().EulerAngles();
    auto s = vmat.Scale();
    SetViewMatrix(t,r,s);
  //  auto invDirection = viewportCameraNode_->GetDirection()*-2;
//    viewportCameraNode_->Translate(invDirection);
    viewportCamera_->SetOrthographic(true);
    viewportCamera_->SetOrthoSize(size_);
}

void ViewRenderer::SetPerspMode(const Matrix4 &vmat)
{
    if (viewportCamera_->IsOrthographic()){
        viewportCamera_->ResetToDefault();
    }
    SetViewMatrix(vmat);
}

void ViewRenderer::SetViewData(bool ortho,const Vector3 &pos, const Vector3 &dir, const Vector3 &up, float orthosize, float fov)
{
    if (ortho){
        viewportCamera_->SetOrthographic(true);
        viewportCamera_->SetFov(fov);
        viewportCamera_->SetOrthoSize(orthosize);
        viewportCamera_->SetFarClip(10000.0f);
        orthosize_ = orthosize;
        orthoMode_ = true;
    } else if (viewportCamera_->IsOrthographic() ){
        orthoMode_ = false;
        viewportCamera_->ResetToDefault();
        viewportCamera_->SetFov(fov);
    }
    viewportCameraNode_->SetPosition(Vector3(-pos.y_,pos.z_,pos.x_));
    Quaternion quat;
    quat.FromLookRotation(Vector3(-dir.y_,dir.z_,dir.x_),Vector3(-up.y_,up.z_,up.x_));
    viewportCameraNode_->SetRotation(quat);

    auto _pos = viewportCameraNode_->GetPosition();
    auto _rot = viewportCameraNode_->GetRotation().EulerAngles();

}

void ViewRenderer::SetViewMatrix(const Matrix4 &vmat)
{
    viewportCamera_->SetOrthographic(false);
    auto t = vmat.Translation();
    auto r = vmat.Rotation().EulerAngles();
    auto s = vmat.Scale();
    SetViewMatrix(t,r,s);
}



void ViewRenderer::SetViewMatrix(const Vector3& t,const Vector3& r,const Vector3& s)
{
    if (r.y_>-10 && r.y_<10){
        viewportCameraNode_->SetPosition(Vector3(0,0,0));
        viewportCameraNode_->SetRotation(Quaternion(r.x_+90,r.z_-90,-r.y_));
        viewportCameraNode_->Translate(Vector3(-t.x_,-t.y_,t.z_));
    } else {
       viewportCameraNode_->SetPosition(Vector3(0,0,0));
       viewportCameraNode_->SetRotation(Quaternion(r.x_-90,r.z_-90,-r.y_));
       viewportCameraNode_->Translate(Vector3(-t.x_,-t.y_,t.z_));
    }

    auto rot = viewportCameraNode_->GetRotation().EulerAngles();
    URHO3D_LOGINFOF("ROTATION:%s",rot.ToString().CString());

  //  renderSurface_->QueueUpdate();
}

void ViewRenderer::SetSize(int width, int height, float fov)
{
    if (!renderTexture_) {
        renderTexture_ = new Texture2D(ctx_);
    }
    width_ = width;
    height_ = height;
    fov_ = fov;

    viewportCamera_->SetFov(fov);

    bool result = renderTexture_->SetSize(width,height,Graphics::GetRGBAFormat(), TEXTURE_RENDERTARGET);
    renderTexture_->SetFilterMode(FILTER_BILINEAR);

    renderSurface_ = renderTexture_->GetRenderSurface();

    // TODO: memory-leak? what happens with the viewport. Do I need to delete this on my own?
    viewport_ = new Viewport(ctx_, currentScene_, viewportCamera_);
    renderSurface_->SetViewport(0, viewport_);
    renderSurface_->SetUpdateMode(SURFACE_MANUALUPDATE);
  //  renderSurface_->QueueUpdate();
}

void ViewRenderer::HandleSceneUpdate(StringHash eventType, VariantMap &eventdata)
{
    using namespace BlenderSceneUpdated;
    String sceneName = eventdata[P_SCENE_NAME].GetString();
    Scene* scene = eventdata[P_SCENE].GetCustom<Scene*>();

    if (currentScene_ == scene || (currentScene_ && currentScene_->GetName()==sceneName)) {
        auto navMesh = currentScene_->GetDerivedComponent<NavigationMesh>(true);
        if (navMesh){
            navMesh->Build();
        }
        GetSubsystem<BlenderRuntime>()->UpdateViewRenderer(this);
    }
}

void ViewRenderer::RequestRender()
{
    if (parent->sessionSettings.showPhysics){
        PhysicsWorld* pw = currentScene_->GetOrCreateComponent<PhysicsWorld>();
        auto dr = currentScene_->GetOrCreateComponent<DebugRenderer>();
        pw->DrawDebugGeometry(parent->sessionSettings.showPhysicsDepth);

        auto navigationMesh = currentScene_->GetDerivedComponent<NavigationMesh>(true);
        if (navigationMesh){
            navigationMesh->DrawDebugGeometry(dr,false);
        }
    }

    if (parent->sessionSettings.renderData){
        context_->RegisterSubsystem(parent->mCurrentExportpath->GetResourceCache());
        parent->sessionSettings.renderData->SetRenderPathOnViewport(viewport_);
    }

    renderSurface_->QueueUpdate();
}

void ViewRenderer::Show()
{
    Renderer* renderer = ctx_->GetSubsystem<Renderer>();
    Viewport* viewport = renderer->GetViewport(0);
    viewport->SetScene(currentScene_);
    viewport->SetCamera(viewportCamera_);
}
