#include "StartupApplication.h"

#include <Urho3D/Input/InputEvents.h>
#include <Urho3D/Engine/EngineDefs.h>
#include <Urho3D/Urho3DAll.h>

#include "Components/ComponentsActivator.h"

URHO3D_DEFINE_APPLICATION_MAIN(StartupApplication)

StartupApplication::StartupApplication(Context* ctx) : Application(ctx)
{
    setbuf(stdout, NULL);
    SubscribeToEvents();
}

void StartupApplication::Setup()
{
    FileSystem* fs = GetSubsystem<FileSystem>();
    auto resourcePath = fs->GetProgramDir() + "Data";

    ResourceCache* cache=GetSubsystem<ResourceCache>();
    cache->AddResourceDir(resourcePath);
#ifdef GAME_ENABLE_COMPONENT_EXPORTER
    SetupComponentExporter();
#endif
    // register game
    game_ = new GameLogic(context_);
    context_->RegisterSubsystem(game_);
    // setup game
    game_->Setup(engineParameters_);
    ComponentsActivator::RegisterComponents(context_);
}

void StartupApplication::Start()
{
    game_->Start();

#ifdef GAME_ENABLE_COMPONENT_EXPORTER
    // export registered components
    ExportComponents(String(PROJECT_NAME)+"_components.json");
#endif

#ifdef GAME_ENABLE_DEBUG_TOOLS
    // Get default style
    ResourceCache* cache = GetSubsystem<ResourceCache>();
    XMLFile* xmlFile = cache->GetResource<XMLFile>("UI/DefaultStyle.xml");
    // Create console
    Console* console = engine_->CreateConsole();
    console->SetDefaultStyle(xmlFile);
    console->GetBackground()->SetOpacity(0.8f);

    // Create debug HUD.
    DebugHud* debugHud = engine_->CreateDebugHud();
    debugHud->SetDefaultStyle(xmlFile);
#endif
}

void StartupApplication::Stop()
{
}

void StartupApplication::SubscribeToEvents()
{
    // Called after engine initialization. Setup application & subscribe to events here
    SubscribeToEvent(E_KEYDOWN, URHO3D_HANDLER(StartupApplication, HandleKeyDown));
}

void StartupApplication::HandleKeyDown(StringHash eventType, VariantMap& eventData)
{
    using namespace KeyDown;
    // Check for pressing ESC. Note the engine_ member variable for convenience access to the Engine object
    int key = eventData[P_KEY].GetInt();

    if (key == KEY_ESCAPE)
        engine_->Exit();

#ifdef GAME_ENABLE_DEBUG_TOOLS
    if (key == KEY_F10)
        GetSubsystem<DebugHud>()->ToggleAll();

    if (key == KEY_F9){

        Console* console = GetSubsystem<Console>();
        console->Toggle();
    }

#endif
}

#ifdef GAME_ENABLE_COMPONENT_EXPORTER
void StartupApplication::SetupComponentExporter()
{
    auto exporter = new Urho3DNodeTreeExporter(context_);
    context_->RegisterSubsystem(exporter);

    // set whitelist-mode to tell the exporter what components exactly to include for export
    exporter->SetExportMode(Urho3DNodeTreeExporter::WhiteList);

    // include all Components that inherit from LogicComponent
    exporter->AddSuperComponentHashToFilterList(LogicComponent::GetTypeStatic());
    // explicitly export those components
    exporter->AddComponentHashToFilterList(Light::GetTypeStatic());
    exporter->AddComponentHashToFilterList(Camera::GetTypeStatic());

    exporter->AddMaterialFolder("Materials");
    exporter->AddTechniqueFolder("Techniques");
    exporter->AddTextureFolder("Textures");
    exporter->AddModelFolder("Models");
    exporter->AddAnimationFolder("Models");

    exporter->AddComponentHashToFilterList(RigidBody::GetTypeStatic());
    exporter->AddComponentHashToFilterList(CollisionShape::GetTypeStatic());
    exporter->AddComponentHashToFilterList(Navigable::GetTypeStatic());
    exporter->AddComponentHashToFilterList(NavArea::GetTypeStatic());
    exporter->AddComponentHashToFilterList(NavigationMesh::GetTypeStatic());
    exporter->AddComponentHashToFilterList(Octree::GetTypeStatic());
    exporter->AddComponentHashToFilterList(PhysicsWorld::GetTypeStatic());
    exporter->AddComponentHashToFilterList(DebugRenderer::GetTypeStatic());
    exporter->AddComponentHashToFilterList(Zone::GetTypeStatic());
    exporter->AddComponentHashToFilterList(AnimationController::GetTypeStatic());
}

void StartupApplication::ExportComponents(const String& outputPath)
{
    Urho3DNodeTreeExporter* exporter = GetSubsystem<Urho3DNodeTreeExporter>();
    exporter->Export(outputPath,true,false);
}
#endif
