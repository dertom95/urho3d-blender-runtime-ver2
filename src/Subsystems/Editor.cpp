#include <Urho3D/Urho3DAll.h>

#include "Editor.h"

static StringHash E_EDITOR_TOGGLE = "E_EDITOR_TOGGLE";
static StringHash P_EDITOR_HIDDEN = "P_EDITOR_HIDDEN";

Editor::Editor(Context* ctx_)
    : Object(ctx_)
    , initialized_(false)
{}

void Editor::InitEditor()
{
    if (initialized_) return;

    ResourceCache* cache = GetSubsystem<ResourceCache>();
    bool foundEditorResources = cache->AddPackageFile("EditorData.pak");

    if (!foundEditorResources){
        URHO3D_LOGERROR("Could not init edito because file  'Editor.pak' was not found");
        return;
    }

    Scene* scene = GetSubsystem<Scene>();
    Renderer* renderer = GetSubsystem<Renderer>();
    auto numViews = renderer->GetNumViews();
    auto numViewports = renderer->GetNumViewports();
    auto sceneViewport = renderer->GetViewportForScene(scene,0);
    auto currentView = renderer->GetViewport(0);
    initialized_ = true;

    if (!context_->GetSubsystem<Script>()){
        // Instantiate and register the AngelScript subsystem
        context_->RegisterSubsystem(new Script(context_));
        context_->RegisterSubsystem(new LuaScript(context_));

        // Hold a shared pointer to the script file to make sure it is not unloaded during runtime
        scriptFile_ = GetSubsystem<ResourceCache>()->GetResource<ScriptFile>("Scripts/Editor.as");
    }


    /// \hack If we are running the editor, also instantiate Lua subsystem to enable editing Lua ScriptInstances
    // If script loading is successful, proceed to main loop
    if (scriptFile_ && scriptFile_->Execute("void Start()"))
    {
        SubscribeToEvent(E_EDITOR_TOGGLE,URHO3D_HANDLER(Editor, HandleEditorEvents));

        // Subscribe to script's reload event to allow live-reload of the application
        SubscribeToEvent(scriptFile_, E_RELOADSTARTED, URHO3D_HANDLER(Editor, HandleScriptReloadStarted));
        SubscribeToEvent(scriptFile_, E_RELOADFINISHED, URHO3D_HANDLER(Editor, HandleScriptReloadFinished));
        SubscribeToEvent(scriptFile_, E_RELOADFAILED, URHO3D_HANDLER(Editor, HandleScriptReloadFailed));
        return;
    }

}


void Editor::HandleScriptReloadStarted(StringHash eventType, VariantMap& eventData)
{
#ifdef URHO3D_ANGELSCRIPT
    if (scriptFile_->GetFunction("void Stop()"))
        scriptFile_->Execute("void Stop()");
#endif
}

void Editor::HandleScriptReloadFinished(StringHash eventType, VariantMap& eventData)
{
#ifdef URHO3D_ANGELSCRIPT
    // Restart the script application after reload
    if (!scriptFile_->Execute("void Start()"))
    {
        scriptFile_.Reset();
        ErrorExit();
    }
#endif
}

void Editor::HandleScriptReloadFailed(StringHash eventType, VariantMap& eventData)
{
#ifdef URHO3D_ANGELSCRIPT
    scriptFile_.Reset();
    ErrorExit();
#endif
}

void Editor::HandleEditorEvents(StringHash eventType, VariantMap &eventData)
{
    URHO3D_LOGINFO("EDITOR EXIT!");
    if (eventType == E_EDITOR_TOGGLE){
        Renderer* renderer = GetSubsystem<Renderer>();

        bool hidden = eventData[P_EDITOR_HIDDEN].GetBool();

        if (hidden) {
            // check if we already got
            editorViewports_.Clear();

            // keep a ref on the editor-viewport(s)
            int viewportAmount = renderer->GetNumViewports();
            for (int i=0;i<viewportAmount;i++){
                editorViewports_.Push(renderer->GetViewport(i));
            }

            // reset the game-viewport
            Scene* scene = GetSubsystem<Scene>();
            renderer->SetNumViewports(1);
            renderer->SetViewport(0,GetSubsystem<Viewport>());
            scene->SetUpdateEnabled(true);
        } else {
            // stop the scene-update(as it would have been done on editor-start
            Scene* scene = GetSubsystem<Scene>();
            scene->SetUpdateEnabled(false);
            // reset editor-viewports
            renderer->SetNumViewports(editorViewports_.Size());
            for(int i=0;i<editorViewports_.Size();i++){
                renderer->SetViewport(i,editorViewports_[i]);
            }
        }
    }
}
