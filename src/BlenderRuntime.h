#pragma once

#include <Urho3D/Core/Object.h>
#include <Urho3D/Resource/ResourceCache.h>
#include <Urho3D/Scene/Scene.h>
#include <Urho3D/Graphics/Texture2D.h>

#include "Components/RenderData.h"

#include <Urho3D/UI/Window.h>

#include "Subsystems/LoaderTools/ComponentExporter.h"

class ViewRenderer;

class BlenderNetwork;

using namespace Urho3D;


URHO3D_EVENT(E_BLENDER_SCENE_UPDATED, BlenderSceneUpdated)
{
    URHO3D_PARAM(P_SCENE_NAME, SceneName);  //string
    URHO3D_PARAM(P_SCENE, Scene); // ptr
}

enum class ExportComponentMode {
    none=0,lite=1,all=2
};

struct SessionSettings {
    bool showPhysics;
    bool showPhysicsDepth;
    bool activatePhysics;

    RenderData* renderData;

    ExportComponentMode exportComponentMode;

};


class BlenderExportPath : public Object {
    URHO3D_OBJECT(BlenderExportPath,Object)
public:
    BlenderExportPath(Context* ctx, String exportPath, bool createResourceCache=true);
    inline SharedPtr<ResourceCache> GetResourceCache() { return mResourceCache; }

    SharedPtr<Scene> GetScene(String sceneName);

    Urho3DNodeTreeExporter* mMaterialExporter;

    void SetExportMode(ExportComponentMode mode);
    void RequestExport();
    inline String GetExportPath() { return mExportPath; }
private:
    void HandleResourcesChanged(StringHash eventType,VariantMap& eventdata);
    void HandleUpdate(StringHash eventType,VariantMap& eventdata);

    void ExportMaterials();

    ExportComponentMode mComponentExportMode;
    String mExportPath;
    String mExportMaterialsPath;
    HashMap<String,SharedPtr<Scene>> mScenes;
    SharedPtr<ResourceCache> mResourceCache;
    HashSet<String> knownResources;
    bool mRequestedTreeExport;
    float mRequestTimer;
};


// one session = one blender instance
class BlenderSession : public Object{
    URHO3D_OBJECT(BlenderSession,Object)
public:
    BlenderSession(Context* ctx, int sessionId);
    ~BlenderSession();

    ViewRenderer* GetOrCreateView(int viewId);
    inline int GetSessionId() { return mSessionId; }
    void SetExportPath(SharedPtr<BlenderExportPath> exportPath);
    void Ping();
    inline float GetLastPing(){ return mLastPing;}
    void UpdateSessionViewRenderers();

    SharedPtr<Scene> SetScene(String sceneName);

    SessionSettings sessionSettings;
    SharedPtr<BlenderExportPath> mCurrentExportpath;
private:
    int mSessionId;
    String mCurrentSceneName;
    SharedPtr<Scene> mCurrentScene;
    HashMap<int,ViewRenderer*> mSessionRenderers; // every window that should be rendered with Urho3D-Renderer
    float mLastPing;
};

class BlenderRuntime : public Object {
    URHO3D_OBJECT(BlenderRuntime,Object)
public:
    BlenderRuntime(Context* ctx);
    ~BlenderRuntime();
    SharedPtr<BlenderExportPath> GetOrCreateExportPath(String path);
    void UpdateViewRenderer(ViewRenderer* renderer);
    void AddViewRenderer(ViewRenderer* renderer);
//    void UpdateAllViewRenderers(Scene* scene=nullptr);
    void InjectGlobalResourcePaths(ResourceCache* resCache);
private:
    void HandleBlenderMessage(StringHash eventType,VariantMap& eventData);
    void HandleConsoleInput(StringHash eventType, VariantMap& eventData);
    void HandleAfterRender(StringHash eventType, VariantMap& eventData);
    void HandleMiscEvent(StringHash eventType, VariantMap& eventData);
    void HandleUpdate(StringHash eventType, VariantMap& eventData);

    void InitNetwork();
    void InitUI();

    void CheckSessions();

    void ProcessDataChange(JSONObject& dataChange);

    SharedPtr<BlenderSession> GetSession(int sessionID);
    SharedPtr<BlenderSession> GetOrCreateSession(int sessionID);

    HashMap<String,SharedPtr<BlenderExportPath>> mExportPaths;
    HashMap<int,SharedPtr<BlenderSession>> mSessions;

    int mCurrentVisualViewRendererId;
    Vector<ViewRenderer*> mViewRenderers;

    HashMap<Texture2D*, ViewRenderer* > mUpdatedRenderers;

    SharedPtr<BlenderNetwork> mBlenderNetwork;
    JSONFile mJsonfile;

    SharedPtr<Window> mWindow;
    /// The UI's root UIElement.
    SharedPtr<UIElement> mUiRoot;
    SharedPtr<ResourceCache> mGlobalResourceCache;

    float mSessionCleanUpCheckTimer;
    float mUpdateTicker;

    bool mSendHello;
};
