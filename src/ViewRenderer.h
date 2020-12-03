#pragma once

#include <Urho3D/Core/Object.h>
#include <Urho3D/Resource/ResourceCache.h>
#include <Urho3D/Scene/Scene.h>
#include <Urho3D/Graphics/Texture2D.h>

#include "BlenderRuntime.h"

using namespace Urho3D;

class ViewRenderer : public Object{
    URHO3D_OBJECT(ViewRenderer,Object)
public:
    ViewRenderer(Context* ctx,SharedPtr<BlenderSession> _parent, int id);
    void SetData(Scene* initialScene, int width,int height,float fov);
    void SetSize(int width,int height,float fov);
    void SetScene(SharedPtr<Scene> scene);
    void SetViewMatrix(const Matrix4& vmat);
    void SetViewMatrix(const Vector3& t,const Vector3& r,const Vector3& s);
    void SetOrthoMode(const Matrix4& vmat,float size_);
    void SetPerspMode(const Matrix4& vmat);
    void SetViewData(bool orthoMode,const Vector3& pos,const Vector3& dir,const Vector3& up,float orthosize, float fov);
    inline SharedPtr<Texture2D> GetRenderTexture(){ return renderTexture_;}
    inline int GetId() { return viewId_;}
    inline SharedPtr<Scene> GetScene() { return currentScene_; }
    inline SharedPtr<Camera> GetCamera() { return viewportCamera_;}
    inline SharedPtr<Viewport> GetViewport() { return viewport_;}
    const String& GetNetId() { return netId; }
    void SetRenderPath(String renderPath) {renderPath_=renderPath;}
    void RequestRender();
    void Show();
    void SetSceneTime(float f);
    float fov_;
    SharedPtr<BlenderSession> parent;

private:
    void HandleSceneUpdate(StringHash eventType, VariantMap& eventdata);

    //WeakPtr<BlenderSession> parent;

    String netId;
    int viewId_;
    int width_;
    int height_;
    float orthosize_;
    bool orthoMode_;
    String renderPath_;

    Context* ctx_;
    SharedPtr<Scene> currentScene_;
    SharedPtr<RenderSurface> renderSurface_;
    SharedPtr<Texture2D> renderTexture_;
    SharedPtr<Viewport> viewport_;
    SharedPtr<Node> viewportCameraNode_;
    SharedPtr<Camera> viewportCamera_;
    SharedPtr<RenderPath> nonPBRPath_;
};
