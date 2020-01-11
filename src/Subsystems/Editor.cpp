#include <Urho3D/Urho3DAll.h>

#include "Editor.h"

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
