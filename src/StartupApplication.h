#pragma once

#include <project_options.h>

#include <Urho3D/Engine/Engine.h>
#include <Urho3D/Engine/Application.h>

#include "Gamelogic.h"

#ifdef ENABLE_COMPONENT_EXPORTER
 #include "Subsystems/LoaderTools/ComponentExporter.h"
#endif

using namespace Urho3D;

class StartupApplication : public Application {
    URHO3D_OBJECT(StartupApplication, Application);

public:
    StartupApplication(Context* context);

    void Setup() override;
    void Start() override;
    void Stop() override;
    void SubscribeToEvents();
    void HandleKeyDown(StringHash eventType, VariantMap& eventData);

#ifdef ENABLE_COMPONENT_EXPORTER
    void SetupComponentExporter();
    void ExportComponents(const String& outputPath);
#endif

private:
    SharedPtr<GameLogic> game_;

};
