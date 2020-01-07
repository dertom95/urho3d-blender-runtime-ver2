#include "thegame.h"

#include <Urho3D/Input/InputEvents.h>
#include <Urho3D/Engine/EngineDefs.h>
#include <Urho3D/Urho3DAll.h>

#include "gameComponents/ComponentsActivator.h"

URHO3D_DEFINE_APPLICATION_MAIN(TheGameMain)

TheGameMain::TheGameMain(Context* ctx) : Application(ctx)
{
    setbuf(stdout, NULL);
    SubscribeToEvents();
}

void TheGameMain::Setup()
{
    // TODO: needed?
    FileSystem* fs = GetSubsystem<FileSystem>();
    auto resourcePath = fs->GetProgramDir() + "Data";
    auto exists = fs->DirExists(resourcePath);

    ResourceCache* cache=GetSubsystem<ResourceCache>();
    cache->AddResourceDir(resourcePath);
    //-------------------------------------------------

    context_->RegisterSubsystem(new Urho3DNodeTreeExporter(context_,Urho3DNodeTreeExporter::WhiteList));
    // register game
    game_ = new GameLogic(context_);
    context_->RegisterSubsystem(game_);
    // setup game
    game_->Setup(engineParameters_);
    ComponentsActivator::RegisterComponents(context_);
}

void TheGameMain::Start()
{
    game_->Start();

    ExportComponents("minimal-components.json");

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

}

void TheGameMain::Stop()
{
}

void TheGameMain::SubscribeToEvents()
{
    // Called after engine initialization. Setup application & subscribe to events here
    SubscribeToEvent(E_KEYDOWN, URHO3D_HANDLER(TheGameMain, HandleKeyDown));
}

void TheGameMain::HandleKeyDown(StringHash eventType, VariantMap& eventData)
{
    using namespace KeyDown;
    // Check for pressing ESC. Note the engine_ member variable for convenience access to the Engine object
    int key = eventData[P_KEY].GetInt();

    if (key == KEY_ESCAPE)
        engine_->Exit();

    if (key == KEY_F2)
        GetSubsystem<DebugHud>()->ToggleAll();
}

void TheGameMain::ExportComponents(const String& outputPath)
{
    Urho3DNodeTreeExporter* exporter = GetSubsystem<Urho3DNodeTreeExporter>();
    // set whitelist-mode to tell the exporter what components to include for export
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
    //exporter->AddComponentHashToFilterList(StaticModel::GetTypeStatic());

    // only export the components (not the materials, which are handled by the urho-sceneloader)
    exporter->Export(outputPath,true,false);
}
