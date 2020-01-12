#pragma once

#include <Urho3D/Urho3DAll.h>

using namespace Urho3D;


class Editor : public Object
{
  URHO3D_OBJECT(Editor,Object);

public:
    explicit Editor(Context* context);

    static void RegisterObject(Context* context);

    void InitEditor();


private:
    /// Handle reload start of the script file.
    void HandleScriptReloadStarted(StringHash eventType, VariantMap& eventData);
    /// Handle reload success of the script file.
    void HandleScriptReloadFinished(StringHash eventType, VariantMap& eventData);
    /// Handle reload failure of the script file.
    void HandleScriptReloadFailed(StringHash eventType, VariantMap& eventData);

    /// Handle events sends by the editor
    void HandleEditorEvents(StringHash eventType, VariantMap& eventData);

    bool initialized_;
    SharedPtr<ScriptFile> scriptFile_;

    PODVector<Viewport*> editorViewports_;
};
