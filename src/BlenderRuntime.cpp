#include "BlenderRuntime.h"

#include <Urho3D/Urho3DAll.h>

#include "Subsystems/BlenderNetworkEvents.h"
#include "Subsystems/BlenderNetwork.h"
#include <project_options.h>
#include "Subsystems/PackageTool.h"

#include "ViewRenderer.h"

BlenderSession::BlenderSession(Context *ctx, int sessionId)
    : Object(ctx),
      mSessionId(sessionId),
      mLastPing(0)
{
    sessionSettings.renderData = new RenderData(ctx);
}

BlenderSession::~BlenderSession()
{
    URHO3D_LOGINFOF("Destruction %i",mSessionId);
    if (sessionSettings.renderData){
        delete sessionSettings.renderData;
    }
}

ViewRenderer* BlenderSession::GetOrCreateView(int viewID)
{
    if (mSessionRenderers.Contains(viewID)){
        return mSessionRenderers[viewID];
    }

    ViewRenderer* newRenderer = new ViewRenderer(context_,SharedPtr<BlenderSession>(this),viewID);
    mSessionRenderers[viewID] = newRenderer;

    GetSubsystem<BlenderRuntime>()->AddViewRenderer(newRenderer);

    return newRenderer;
}

void BlenderSession::SetExportPath(SharedPtr<BlenderExportPath> exportPath)
{
    if (exportPath == mCurrentExportpath) return;

    mCurrentExportpath = exportPath;

    if (mCurrentExportpath){
        UnsubscribeFromEvents(mCurrentExportpath->GetResourceCache());
    }
}

void BlenderSession::Show(){
    for (auto viewRenderer : mSessionRenderers){
        viewRenderer.second_->Show();
        return;
    }
}

SharedPtr<Scene> BlenderSession::SetScene(String sceneName)
{
    if (mCurrentSceneName == sceneName) nullptr;

    if (mCurrentExportpath){
        mCurrentSceneName = sceneName;
        mCurrentScene = mCurrentExportpath->GetScene(sceneName);
        if (!mCurrentScene){
            URHO3D_LOGERRORF("Could not retrieve scene:%s",sceneName.CString());
            return nullptr;
        }
        auto renderData = mCurrentScene->GetComponent<RenderData>(true);
        if (renderData){
            sessionSettings.renderData->Read(renderData);
        }
        return mCurrentScene;
    }
    return nullptr;
}

void BlenderSession::Ping()
{
    Time* time = GetSubsystem<Time>();
    auto ts = time->GetTimeStamp();
    auto timeStep = time->GetTimeStep();
    mLastPing = time->GetElapsedTime();
}

void BlenderSession::UpdateSessionViewRenderers()
{
    BlenderRuntime* rt = GetSubsystem<BlenderRuntime>();

    for (ViewRenderer* vr: mSessionRenderers.Values()){
        rt->UpdateViewRenderer(vr);
    }
}

// ------------------------- Blender Export Path ----------------------------

BlenderExportPath::BlenderExportPath(Context *ctx, String exportPath, bool createResourceCache)
    : Object(ctx),
      mExportPath(exportPath),
      mRequestedTreeExport(false),
      mRequestTimer(0.0f),
      mComponentExportMode(ExportComponentMode::none)
{
    if (createResourceCache){
        mResourceCache = new ResourceCache(ctx);

        mResourceCache->AddResourceDir(exportPath);
        BlenderRuntime* rt = GetSubsystem<BlenderRuntime>();
        rt->InjectGlobalResourcePaths(mResourceCache);
    } else {
        mResourceCache = GetSubsystem<ResourceCache>();
        mResourceCache->AddResourceDir(exportPath,0);
    }

    mResourceCache->SetAutoReloadResources(true);

    mExportMaterialsPath = mExportPath+"/__blender_material.json";

    mMaterialExporter = new Urho3DNodeTreeExporter(ctx);
    mMaterialExporter->AddMaterialFolder("Materials");
    mMaterialExporter->AddTechniqueFolder("Techniques");
    mMaterialExporter->AddTextureFolder("Textures");
    mMaterialExporter->AddModelFolder("Models");
    mMaterialExporter->AddAnimationFolder("Models");
    mMaterialExporter->AddRenderPathFolder("RenderPaths");
    mMaterialExporter->AddSoundFolder("Sounds");
    mMaterialExporter->AddSoundFolder("Nusic");
    mMaterialExporter->AddParticleFolder("Particle");
    mMaterialExporter->AddSceneFolder("Scenes");
    mMaterialExporter->AddObjectFolder("Objects");
    mMaterialExporter->SetResourceCache(mResourceCache);

    // export materials right aways
    ExportMaterials();

    SubscribeToEvent(mResourceCache,E_FILECHANGED,URHO3D_HANDLER(BlenderExportPath,HandleResourcesChanged));
    SubscribeToEvent(E_UPDATE,URHO3D_HANDLER(BlenderExportPath,HandleUpdate));
}

void BlenderExportPath::SetExportMode(ExportComponentMode mode)
{
    mComponentExportMode = mode;

    mMaterialExporter->ClearFilters();

    if (mode == ExportComponentMode::lite){
        mMaterialExporter->SetExportMode(Urho3DNodeTreeExporter::WhiteList);
        // include all Components that inherit from LogicComponent
        mMaterialExporter->AddSuperComponentHashToFilterList(LogicComponent::GetTypeStatic());
        // explicitly export those components
        mMaterialExporter->AddComponentHashToFilterList(Light::GetTypeStatic());
        mMaterialExporter->AddComponentHashToFilterList(Camera::GetTypeStatic());
        mMaterialExporter->AddComponentHashToFilterList(RigidBody::GetTypeStatic());
        mMaterialExporter->AddComponentHashToFilterList(CollisionShape::GetTypeStatic());
        mMaterialExporter->AddComponentHashToFilterList(Navigable::GetTypeStatic());
        mMaterialExporter->AddComponentHashToFilterList(NavArea::GetTypeStatic());
        mMaterialExporter->AddComponentHashToFilterList(NavigationMesh::GetTypeStatic());
        mMaterialExporter->AddComponentHashToFilterList(CrowdAgent::GetTypeStatic());
        mMaterialExporter->AddComponentHashToFilterList(Obstacle::GetTypeStatic());
        mMaterialExporter->AddComponentHashToFilterList(Octree::GetTypeStatic());
        mMaterialExporter->AddComponentHashToFilterList(PhysicsWorld::GetTypeStatic());
        mMaterialExporter->AddComponentHashToFilterList(DebugRenderer::GetTypeStatic());
        mMaterialExporter->AddComponentHashToFilterList(Zone::GetTypeStatic());
        mMaterialExporter->AddComponentHashToFilterList(LuaScriptInstance::GetTypeStatic());
        mMaterialExporter->AddComponentHashToFilterList(AnimationController::GetTypeStatic());
    }
    else if (mode == ExportComponentMode::all) {
        mMaterialExporter->SetExportMode(Urho3DNodeTreeExporter::BlackList);
    }
    // set whitelist-mode to tell the exporter what components exactly to include for export

    RequestExport();
}

void BlenderExportPath::RequestExport()
{
    mRequestTimer=1.0f;
    mRequestedTreeExport=true;
}

void BlenderExportPath::SetupSceneInternals(Scene* scene)
{
    // set animations to current frame

//    PODVector<Node*> nodes;
//    scene->GetNodesWithTag(nodes,"__runtime_nodeanim");
//    for (Node* node : nodes){
//        Node* addAnimationTo = node;
//        if (node->HasComponent<AnimatedModel>()){
//            addAnimationTo = node->GetParent()->CreateChild(node->GetName()+"_anim_parent");
//            node->SetParent(addAnimationTo);
//        }

//        auto animationVar = node->GetVar("__runtime_nodeanimation");
//        if (!animationVar.IsEmpty()){
//            String animationName = animationVar.GetString();
//            float animationTime = node->GetVar("__runtime_animation_time").GetFloat();

//            AnimationController* animControl = addAnimationTo->GetOrCreateComponent<AnimationController>();

//            animControl->PlayExclusive(animationName,0,false,0);
//            auto animations = animControl->GetAnimations();


//            animControl->SetTime(animationName,animationTime);
//        }
//    }

    PODVector<AnimatedModel*> animationComponents;
    scene->GetComponents<AnimatedModel>(animationComponents,true);
    for (auto animComp : animationComponents){
        auto node = animComp->GetNode();

        auto shapekeysVar = node->GetVar("__runtime_shapekeys");

        if (!shapekeysVar.IsEmpty()){
            for (auto split : shapekeysVar.GetString().Split('|')){
                Vector<String> shapeKeyValues = split.Split('~');
                String key = shapeKeyValues[0];
                float value = ToFloat(shapeKeyValues[1]);
                animComp->SetMorphWeight(key,value);
            }
        }

        auto animationVar = node->GetVar("__runtime_animation");
        if (!animationVar.IsEmpty()){
            String animationName = animationVar.GetString();
            float animationTime = node->GetVar("__runtime_animation_time").GetFloat();

            AnimationController* animControl = node->GetComponent<AnimationController>(false);
            bool animControlPresent = animControl != nullptr;
            if (!animControlPresent){
                animControl = node->CreateComponent<AnimationController>();
            }

            animControl->PlayExclusive(animationName,0,false,0);
            auto animations = animControl->GetAnimations();

            animControl->SetTime(animationName,animationTime);
        }
    }

}

void BlenderExportPath::HandleResourcesChanged(StringHash eventType, VariantMap &eventdata)
{
    using namespace FileChanged;
    String filename = eventdata[P_FILENAME].GetString();
    String resName = eventdata[P_RESOURCENAME].GetString();

    context_->RegisterSubsystem(mResourceCache);
    if (resName.StartsWith("Scenes")){
        if (mScenes.Contains(resName)){
            SharedPtr<File> file = mResourceCache->GetFile(resName);

            Scene* scene =mScenes[resName];
            scene->LoadXML(*file);
            SetupSceneInternals(scene);
            scene->Update(0);

            using namespace BlenderSceneUpdated;
            VariantMap data;
            data[P_SCENE_NAME]=resName;
            data[P_SCENE]=MakeCustomValue(scene);
            SendEvent(E_BLENDER_SCENE_UPDATED,data);
        }
    }
    else if (resName.StartsWith("Materials")){
        mResourceCache->ReloadResourceWithDependencies(resName);

        using namespace BlenderSceneUpdated;
        for (auto _sceneKV : mScenes){
            VariantMap data;
            data[P_SCENE_NAME]=_sceneKV.first_;
            data[P_SCENE]=MakeCustomValue(_sceneKV.second_);
            SendEvent(E_BLENDER_SCENE_UPDATED,data);
        }
    }

    if (!knownResources.Contains(resName)){
        knownResources.Insert(resName);
        RequestExport();
    }

//    if (resName.EndsWith("png") || resName.EndsWith("jpg") || resName.EndsWith("dds"))
//    {
//        // textures changed

//    }
}

SharedPtr<Scene> BlenderExportPath::GetScene(String sceneName)
{
    if (mScenes.Contains(sceneName)){
        return mScenes[sceneName];
    }

    if (!mResourceCache->Exists(sceneName)){
        URHO3D_LOGERRORF("Unknown Scene in for ExportPath(%s): %s",mExportPath.CString(),sceneName.CString());
        return nullptr;
    }

    Scene* newScene = new Scene(context_);

    // load scene and change to the exportPath's ResourceCache first
    context_->RegisterSubsystem(mResourceCache);

    URHO3D_LOGINFOF("##__## LOAD SCENE %s",sceneName.CString());

    SharedPtr<File> file = mResourceCache->GetFile(sceneName);
    //XMLFile* file = mResourceCache->GetResource<XMLFile>(sceneName);
    newScene->LoadXML(*file);
    SetupSceneInternals(newScene);
    newScene->Update(0);

    mScenes[sceneName]=newScene;
    newScene->SetName(sceneName);
    mResourceCache->ReloadResourceWithDependencies(sceneName);

    auto navMesh = newScene->GetDerivedComponent<NavigationMesh>(true);
    if (navMesh){
        navMesh->Build();
    }

    using namespace BlenderSceneUpdated;
    VariantMap data;
    data[P_SCENE_NAME]=sceneName;
    data[P_SCENE]=MakeCustomValue(newScene);
    SendEvent(E_BLENDER_SCENE_UPDATED,data);

    return SharedPtr<Scene>(newScene);
}

void BlenderExportPath::HandleUpdate(StringHash eventType,VariantMap& data)
{
    if (mRequestedTreeExport){
        using namespace Update;
        float dt = data[P_TIMESTEP].GetFloat();
        mRequestTimer -= dt;

        if (mRequestTimer < 0){
            ExportMaterials();
            mRequestedTreeExport=false;
        }
    }
}

void BlenderExportPath::ExportMaterials()
{

    mMaterialExporter->Export(mExportMaterialsPath,mComponentExportMode>ExportComponentMode::none,true);
}

// ---------------------------- Runtime ------------------------------------

BlenderRuntime::BlenderRuntime(Context *ctx)
    : Object(ctx),
      mJsonfile(ctx),
      mViewRenderers(10),
      mCurrentVisualSessionIdx(0),
      mSessionCleanUpCheckTimer(0),
      mUpdateTicker(0),
      mSendHello(false)
{
    mGlobalResourceCache = GetSubsystem<ResourceCache>();
    InitNetwork();
//    SubscribeToEvent(E_ENDALLVIEWSRENDER, URHO3D_HANDLER(BlenderRuntime, HandleAfterRender));
    SubscribeToEvent(E_ENDVIEWRENDER, URHO3D_HANDLER(BlenderRuntime, HandleAfterRender));
    SubscribeToEvent(E_CONSOLECOMMAND, URHO3D_HANDLER(BlenderRuntime, HandleConsoleInput));
    SubscribeToEvent(E_KEYDOWN, URHO3D_HANDLER(BlenderRuntime, HandleMiscEvent));
    SubscribeToEvent(E_UPDATE, URHO3D_HANDLER(BlenderRuntime, HandleUpdate));
}


BlenderRuntime::~BlenderRuntime()
{
    mBlenderNetwork->Close();
}

void BlenderRuntime::InjectGlobalResourcePaths(ResourceCache *resCache)
{
    for (String resPath : mGlobalResourceCache->GetResourceDirs()){
        if (resPath.Contains("CoreData")){
            resCache->AddResourceDir( resPath );
        }
    }
}

void BlenderRuntime::InitNetwork()
{
    if (!mBlenderNetwork){
        mBlenderNetwork = new BlenderNetwork(context_);
        context_->RegisterSubsystem(mBlenderNetwork);

        SubscribeToEvent(E_BLENDER_MSG, URHO3D_HANDLER(BlenderRuntime,HandleBlenderMessage));
    }

    mBlenderNetwork->InitNetwork();
}

SharedPtr<BlenderExportPath> BlenderRuntime::GetOrCreateExportPath(String path)
{
    if (mExportPaths.Contains(path)){
        return mExportPaths[path];
    }

    SharedPtr<BlenderExportPath> newPath(new BlenderExportPath(context_,path, mExportPaths.Size()!=0));
    mExportPaths[path]=newPath;
    return newPath;
}

void BlenderRuntime::HandleBlenderMessage(StringHash eventType, VariantMap &eventData)
{
    using namespace BlenderConnect;
    auto topic = eventData[P_TOPIC].GetString();
    auto subtype = eventData[P_SUBTYPE].GetString();
    auto datatype = eventData[P_DATATYPE].GetString();


#ifdef GAME_DEBUGGING
    URHO3D_LOGINFOF("Got message from blender: %s - %s - %s",topic.CString(),subtype.CString(),datatype.CString());
#endif

    if (topic == "blender") {
        // we got message from blender
        if (subtype == "data_change"){
            auto d = eventData[P_DATA];
            JSONObject data  =  d.GetCustom<JSONObject>();
            ProcessDataChange(data);
        }
        else if (subtype == "settings") {
            auto d = eventData[P_DATA];
            JSONObject json  =  d.GetCustom<JSONObject>();

            int session_id = json["session_id"].GetInt();
            BlenderSession* session = GetOrCreateSession(session_id);

            bool exportPathChanged = json.Contains("export_path") && (!session->mCurrentExportpath || session->mCurrentExportpath->GetExportPath()!=json["export_path"].GetString());
            if (exportPathChanged){
                String exportPath = json["export_path"].GetString();
                SharedPtr<BlenderExportPath> bExportPath = GetOrCreateExportPath(exportPath);
                session->SetExportPath(bExportPath);
            }

            session->sessionSettings.showPhysics = json["show_physics"].GetBool();
            session->sessionSettings.showPhysicsDepth = json["show_physics_depth"].GetBool();
            session->sessionSettings.activatePhysics = json["activate_physics"].GetBool();

            int componentExportMode = json["export_component_mode"].GetInt(0);
            session->sessionSettings.exportComponentMode = (ExportComponentMode)componentExportMode;

            if (session->mCurrentExportpath && session->mCurrentExportpath->mMaterialExporter){
                session->mCurrentExportpath->SetExportMode(session->sessionSettings.exportComponentMode);
            }

            if (exportPathChanged){
                session->mCurrentExportpath->RequestExport();
            }

            session->UpdateSessionViewRenderers();

        }
        else if (subtype == "packagetool"){
            auto d = eventData[P_DATA];
            JSONObject json  =  d.GetCustom<JSONObject>();

            String packageFolder = json["package_folder"].GetString();
            String packageOutput = json["package_name"].GetString();

            auto packageTool = context_->GetSubsystem<PackageTool>();
            //packageTool->WritePackageFile(packageOutput,packageFolder);
            packageTool->Run({packageFolder,packageOutput,"-c"});
        }
        else if (subtype == "ping") {
            BlenderNetwork* bN = GetSubsystem<BlenderNetwork>();
            auto d = eventData[P_DATA];
            JSONObject inJson  =  d.GetCustom<JSONObject>();
            int session_id = inJson["session_id"].GetInt();

            BlenderSession* session = GetSession(session_id);
            if (session){
                session->Ping();
            }


            mJsonfile.GetRoot().Clear();
            mJsonfile.GetRoot().Set("session_id",session_id);

            if (!mSendHello){
                bN->Send("runtime","hello","");
                mSendHello = true;
            }

            bN->Send("runtime","pong","ping response",mJsonfile.ToString());
            // tell the network that we are on and ready to work

        }

    }
}

#ifdef GAME_ENABLE_DEBUG_TOOLS
void BlenderRuntime::HandleConsoleInput(StringHash eventType, VariantMap& eventData)
{
    using namespace ConsoleCommand;
    String command = eventData[P_COMMAND].GetString();
    String id = eventData[P_ID].GetString();

    if (command == "list_exportpaths"){
        URHO3D_LOGINFO("Export:");
        for (String exports : mExportPaths.Keys()){
            URHO3D_LOGINFOF("%s",exports.CString());
        }

    }
}
#endif



void BlenderRuntime::HandleAfterRender(StringHash eventType, VariantMap& eventData)
{
    using namespace EndViewRender;

    Texture2D* rtTexture = static_cast<Texture2D*>(eventData[P_TEXTURE].GetPtr());

    if (rtTexture && mUpdatedRenderers.Contains(rtTexture)){
        ViewRenderer* view = mUpdatedRenderers[rtTexture];

        int imageSize = rtTexture->GetDataSize(rtTexture->GetWidth(), rtTexture->GetHeight());
        unsigned char* _ImageData = new unsigned char[imageSize];
        rtTexture->GetData(0, _ImageData);

        JSONObject json;
        json["width"]=rtTexture->GetWidth();
        json["height"]=rtTexture->GetHeight();
        mJsonfile.GetRoot().Set("resolution",json);
        mJsonfile.GetRoot().Set("fov",view->GetCamera()->GetFov());
        mJsonfile.GetRoot().Set("initial-fov",view->fov_);

        BlenderNetwork* bN = GetSubsystem<BlenderNetwork>();
        bN->Send(view->GetNetId(),"draw",_ImageData,imageSize, mJsonfile.ToString());

        //_pImage->SavePNG(additionalResourcePath+"/Screenshot"+String(view->GetId())+".png");

        delete[] _ImageData;
        mUpdatedRenderers.Erase(rtTexture);
    } else {
        int unknown=1;
    }

//    if (rtRenderRequested && screenshotTimer <= 0 ){

//        BlenderNetwork* bN = GetSubsystem<BlenderNetwork>();
//        bN->Send("blender","renderready");

//        //HandleRequestFromEngineToBlender();
//    }

}



Vector4 JSON2Vec4(const JSONObject& v){
    return Vector4(v["x"]->GetFloat(),v["y"]->GetFloat(),v["z"]->GetFloat(),v["w"]->GetFloat());
}
Vector3 JSON2Vec3(const JSONObject& v){
    return Vector3(v["x"]->GetFloat(),v["y"]->GetFloat(),v["z"]->GetFloat());
}

Matrix4 JSON2Matrix(const JSONArray& mat4Vecs)
{
    Vector4 v21 = JSON2Vec4(mat4Vecs[0].GetObject());
    Vector4 v22 = JSON2Vec4(mat4Vecs[1].GetObject());
    Vector4 v23 = JSON2Vec4(mat4Vecs[2].GetObject());
    Vector4 v24 = JSON2Vec4(mat4Vecs[3].GetObject());

    Matrix4 vmat(v21.x_,v21.y_,v21.z_,v21.w_,
                v22.x_,v22.y_,v22.z_,v22.w_,
                v23.x_,v23.y_,v23.z_,v23.w_,
                v24.x_,v24.y_,v24.z_,v24.w_);
    return vmat;
}



void BlenderRuntime::ProcessDataChange(JSONObject &json)
{
    int sessionID = json["session_id"].GetInt();
    int viewID = json["view_id"].GetInt();

    auto session = GetOrCreateSession(sessionID);

    if (json.Contains("export_path")){
        String exportPath = json["export_path"].GetString();
        SharedPtr<BlenderExportPath> bExportPath = GetOrCreateExportPath(exportPath);
        session->SetExportPath(bExportPath);
    }

    ViewRenderer* view = session->GetOrCreateView(viewID);

    if (json.Contains("scene_name")){
        String sceneName = json["scene_name"].GetString();
        String sceneResPath = "Scenes/"+sceneName+".xml";
        SharedPtr<Scene> scene = session->SetScene(sceneResPath);
        view->SetScene(scene);
    }

    if (!view->GetScene()){
        return;
    }

    if (json.Contains("resolution")){
        auto resolution = json["resolution"].GetObject();
        int width = resolution["width"].GetInt();
        int height = resolution["height"].GetInt();
        float fov = json["fov"].GetFloat();
        view->SetSize(width,height,fov);
        UpdateViewRenderer(view);
    }

    if (json.Contains("scene_time")){
        float newTime = json["scene_time"].GetFloat();
        view->SetSceneTime(newTime);
    }

    if (json.Contains("clip_start")){
        Camera* viewCam = view->GetCamera();
        float clip_start = json["clip_start"].GetFloat();
        float clip_end = json["clip_end"].GetFloat();
        viewCam->SetNearClip(clip_start);
        viewCam->SetFarClip(clip_end);
    }

    if (json.Contains("view_matrix")){
        float fov = json["fov"].GetFloat();

        Matrix4 vmat(JSON2Matrix(json["view_matrix"].GetArray()));
        Matrix4 pmat(JSON2Matrix(json["perspective_matrix"].GetArray()));

        Vector3 pos(JSON2Vec3(json["view_position"].GetObject()));
        Vector3 dir(JSON2Vec3(json["view_direction"].GetObject()));
        Vector3 up(JSON2Vec3(json["view_up"].GetObject()));

        String perspectiveType = json["view_perspective_type"].GetString();

        auto view_distance = json["view_distance"].GetFloat();

        auto vmat_t_mat = vmat.Translation();
        auto vmat_r_mat = vmat.Rotation().EulerAngles();
        auto vmat_s_mat = vmat.Scale();

        auto pmat_t_mat = pmat.Translation();
        auto pmat_r_mat = pmat.Rotation().EulerAngles();
        auto pmat_s_mat = pmat.Scale();

        bool isOrthoMode = perspectiveType == "ORTHO";
        view->SetViewData(isOrthoMode,pos,dir,up,view_distance,fov);
        UpdateViewRenderer(view);
    }
}

//void BlenderRuntime::UpdateAllViewRenderers(Scene* scene)
//{
//    for (ViewRenderer* view : viewRenderers.Values()){
//        if (!scene || view->GetScene() == scene){
//            view->RequestRender();
//            UpdateViewRenderer(view);
//            PhysicsWorld* pw = view->GetScene()->GetComponent<PhysicsWorld>();
//            pw->SetUpdateEnabled(settings.activatePhysics);
//        }f
//    }
//}

void BlenderRuntime::HandleMiscEvent(StringHash eventType, VariantMap &eventData)
{
    if (eventType == E_KEYDOWN){
        using namespace KeyDown;
        int key = eventData[P_KEY].GetInt();
        if (key == KEY_SPACE){
            int amountSessions = mSessions.Size();

            if (amountSessions == 0) return;

            if (mCurrentVisualSessionIdx >= amountSessions){
                mCurrentVisualSessionIdx=0;
            }

            int key = mSessions.Keys()[mCurrentVisualSessionIdx];

            auto visSession = mSessions[key];

            visSession->Show();
            mCurrentVisualSessionIdx++;
        }
    }
}

void BlenderRuntime::HandleUpdate(StringHash eventType, VariantMap &eventData)
{
    using namespace Update;
    float time = eventData[P_TIMESTEP].GetFloat();
    mSessionCleanUpCheckTimer -= time;
    if (mSessionCleanUpCheckTimer < 0){
        CheckSessions();
        mSessionCleanUpCheckTimer = 10.0f;
    }

    mUpdateTicker -= time;
    if (mUpdateTicker <= 0){
        for (auto kv : mUpdatedRenderers){
            kv.second_->RequestRender();
        }
        mUpdateTicker = 0.05f;
    }
}

void BlenderRuntime::UpdateViewRenderer(ViewRenderer *view)
{
//    Renderer* renderer = GetSubsystem<Renderer>();
//    if (view->GetViewport() != renderer->GetViewport(0)){
//        renderer->SetViewport(0,view->GetViewport());
//    }

    mUpdatedRenderers[view->GetRenderTexture()]=view;
}

SharedPtr<BlenderSession> BlenderRuntime::GetSession(int sessionID)
{
    if (mSessions.Contains(sessionID)){
        auto session = mSessions[sessionID];
        return session;
    }
    return nullptr;
}

SharedPtr<BlenderSession> BlenderRuntime::GetOrCreateSession(int sessionID)
{
    SharedPtr<BlenderSession> result = GetSession(sessionID);

    if (result) return result;

    SharedPtr<BlenderSession> newSession(new BlenderSession(context_,sessionID));
    mSessions[sessionID] = newSession;
    return newSession;
}


void BlenderRuntime::AddViewRenderer(ViewRenderer *renderer)
{
    mViewRenderers.Push(renderer);
    auto getIt = mViewRenderers[mViewRenderers.Size()-1];
    int a=0;
}

void BlenderRuntime::CheckSessions()
{
#ifdef GAME_DEBUGGING
    URHO3D_LOGINFO("Checking sessions");
#endif
    Time* time = GetSubsystem<Time>();
    float minimumPing = time->GetElapsedTime() - 15.0f;
    for (int key : mSessions.Keys()){
        BlenderSession* session = mSessions[key];
        if (session->GetLastPing() < minimumPing ){
            mSessions.Erase(key);
#ifdef GAME_DEBUGGING
            URHO3D_LOGINFOF("Remove session %i",key);
#endif
        }
    }
}
