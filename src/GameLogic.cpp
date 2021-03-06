#include "GameLogic.h"
#include <Urho3D/Resource/ResourceCache.h>
#include <Urho3D/Urho3DAll.h>
#include "BlenderRuntime.h"

GameLogic::GameLogic(Context* ctx)
    : Object(ctx),
      mCameraNode(nullptr),
      mScene(nullptr),
      mViewport(nullptr),
      mRenderPhysics(false)
{
}

GameLogic::~GameLogic(){
}

void GameLogic::Setup(VariantMap& engineParameters_)
{
    engineParameters_[EP_FULL_SCREEN]=false;
    engineParameters_[EP_WINDOW_RESIZABLE]=true;
    engineParameters_[EP_WINDOW_WIDTH]=640;
    engineParameters_[EP_WINDOW_HEIGHT]=480;
    SubscribeToEvents();
}

void GameLogic::Start()
{
    SetupSystems();
    SetupScene();
    SetupViewport();
    SetupInput();
  //  SetupUI();
}

void GameLogic::SetupSystems()
{
    context_->RegisterSubsystem(new BlenderRuntime(context_));
}

void GameLogic::SetupScene()
{
    mScene = new Scene(context_);
    context_->RegisterSubsystem( mScene );

    mCameraNode = mScene->CreateChild("cameranode");
    Camera* camera = mCameraNode->CreateComponent<Camera>();
}

void GameLogic::SetupInput()
{
    Input* input = GetSubsystem<Input>();
    input->SetMouseMode(MM_FREE);
    input->SetMouseVisible(true);
}

void GameLogic::SetupViewport()
{
    Renderer* renderer = GetSubsystem<Renderer>();
    mViewport = new Viewport(context_, mScene, mCameraNode->GetComponent<Camera>());
    renderer->SetViewport(0,mViewport);
    context_->RegisterSubsystem(mViewport);
}

void GameLogic::LoadFromFile(String sceneName, Node* loadInto)
{
    auto cache = GetSubsystem<ResourceCache>();
    XMLFile* file = cache->GetResource<XMLFile>(sceneName);
    if (file){
        loadInto->LoadXML(file->GetRoot());
    } else {
        URHO3D_LOGERRORF("no scene %s",sceneName.CString());
    }
}

void GameLogic::LoadFromFile(String sceneName, Scene* loadInto)
{
    auto cache = GetSubsystem<ResourceCache>();
    XMLFile* file = cache->GetResource<XMLFile>(sceneName);
    if (file){
        if (loadInto){
            loadInto->LoadXML(file->GetRoot());
        } else {
            mScene->LoadXML(file->GetRoot());
        }
    } else {
        URHO3D_LOGERRORF("no scene: %s",sceneName.CString());
    }
}

void GameLogic::SubscribeToEvents()
{
    SubscribeToEvent(E_UPDATE, URHO3D_HANDLER(GameLogic, HandleUpdate));
    SubscribeToEvent(E_POSTRENDERUPDATE, URHO3D_HANDLER(GameLogic, HandlePostRenderUpdate));
    SubscribeToEvent(E_KEYDOWN, URHO3D_HANDLER(GameLogic, HandleKeyDown));
#ifdef GAME_ENABLE_DEBUG_TOOLS
    SubscribeToEvent(E_CONSOLECOMMAND, URHO3D_HANDLER(GameLogic, HandleConsoleInput));
#endif
}

void GameLogic::HandleUpdate(StringHash eventType, VariantMap &eventData)
{
    using namespace Update;
    float dt = eventData[P_TIMESTEP].GetFloat();

    Input* input = GetSubsystem<Input>();

    input->SetMouseVisible(!input->GetMouseButtonDown(MOUSEB_RIGHT));

    if (input->GetKeyPress(KEY_F3)){
        mRenderPhysics = !mRenderPhysics;
    }
}

void GameLogic::HandlePostRenderUpdate(StringHash eventType, VariantMap &eventData)
{
    if (mRenderPhysics) {
        mScene->GetComponent<PhysicsWorld>()->DrawDebugGeometry(false);
    }
}

void GameLogic::HandleKeyDown(StringHash eventType, VariantMap &eventData)
{
    using namespace KeyDown;
    // Check for pressing ESC. Note the engine_ member variable for convenience access to the Engine object
    int key = eventData[P_KEY].GetInt();

#ifdef GAME_ENABLE_DEBUG_TOOLS
    if (key == KEY_F1){
        mScene->SetUpdateEnabled(!mScene->IsUpdateEnabled());
    }
    else if (key == KEY_F11){
        File saveFile(context_, "./scene.write.xml",FILE_WRITE);
        mScene->SaveXML(saveFile);
        File saveFileBin(context_, "./scene.bin",FILE_WRITE);
        mScene->Save(saveFileBin);
    }
    else if (key == KEY_F12){
        Editor* editor = GetSubsystem<Editor>();
        if (!editor){
            editor = new Editor(context_);
            editor->InitEditor();
            context_->RegisterSubsystem(editor);
        }
    }
#endif
}

void GameLogic::PlaySound(String soundFile)
{
    auto* cache = GetSubsystem<ResourceCache>();
    auto* sound = cache->GetResource<Sound>("Sounds/"+soundFile);

    auto* soundSource = mScene->CreateComponent<SoundSource>();
    // Component will automatically remove itself when the sound finished playing
    soundSource->SetAutoRemoveMode(REMOVE_COMPONENT);
    soundSource->Play(sound->GetDecoderStream());
    // In case we also play music, set the sound volume below maximum so that we don't clip the output
    soundSource->SetGain(0.75f);
}

void GameLogic::PlayMusic(String musicFile)
{
    auto* cache = GetSubsystem<ResourceCache>();
    auto* music = cache->GetResource<Sound>("Sounds/"+musicFile);
    // Set the song to loop
    music->SetLooped(true);
    mMusicSource->SetGain(0.35f);
    mMusicSource->Play(music);
}

void GameLogic::SetupUI()
{
    mUiRoot = GetSubsystem<UI>()->GetRoot();
    // Create the Window and add it to the UI's root node
    // Load XML file containing default UI style sheet
    auto* cache = GetSubsystem<ResourceCache>();
    auto* style = cache->GetResource<XMLFile>("UI/DefaultStyle.xml");
    // Set the loaded style as default style
    mUiRoot->SetDefaultStyle(style);


    mWindow = new Window(context_);
    mUiRoot->AddChild(mWindow);

    // Set Window size and layout settings
    mWindow->SetMinWidth(784);
    mWindow->SetLayout(LM_VERTICAL, 6, IntRect(6, 6, 6, 6));
    mWindow->SetAlignment(HA_LEFT, VA_TOP);
    mWindow->SetName("Window");

    // Create Window 'titlebar' container
    auto* titleBar = new UIElement(context_);
    titleBar->SetMinSize(0, 24);
    titleBar->SetVerticalAlignment(VA_TOP);
    titleBar->SetLayoutMode(LM_HORIZONTAL);

    // Create the Window title Text
    auto* windowTitle = new Text(context_);
    windowTitle->SetName("WindowTitle");

    windowTitle->SetText("Hello GUI!");

    // Add the controls to the title bar
    titleBar->AddChild(windowTitle);

    // Add the title bar to the Window
    mWindow->AddChild(titleBar);

    // Apply styles
    mWindow->SetStyleAuto();
    windowTitle->SetStyleAuto();
    windowTitle->SetFontSize(18);
    // Subscribe to buttonClose release (following a 'press') events
 //   SubscribeToEvent(buttonClose, E_RELEASED, URHO3D_HANDLER(GameLogic, HandleClosePressed));

    // Subscribe also to all UI mouse clicks just to see where we have clicked
    SubscribeToEvent(E_UIMOUSECLICK, URHO3D_HANDLER(GameLogic, HandleControlClicked));
}

void GameLogic::HandleControlClicked(StringHash eventType, VariantMap& eventData)
{
    // Get the Text control acting as the Window's title
    auto* windowTitle = mWindow->GetChildStaticCast<Text>("WindowTitle", true);

    // Get control that was clicked
    auto* clicked = static_cast<UIElement*>(eventData[UIMouseClick::P_ELEMENT].GetPtr());

    String name = "...?";
    if (clicked)
    {
        // Get the name of the control that was clicked
        name = clicked->GetName();
    }

    // Update the Window's title text
    windowTitle->SetText("Hello " + name + "!");
}

#ifdef GAME_ENABLE_DEBUG_TOOLS
void GameLogic::HandleConsoleInput(StringHash eventType, VariantMap& eventData)
{
    using namespace ConsoleCommand;
    String command = eventData[P_COMMAND].GetString();
    String id = eventData[P_ID].GetString();

    if (command == "GIT_HASH"){
        URHO3D_LOGINFOF("GIT-Hash: %s",String(GIT_HASH).CString());
    }
}
#endif

void GameLogic::SetUIText(String text)
{
    auto* windowTitle = mWindow->GetChildStaticCast<Text>("WindowTitle", true);
    windowTitle->SetText(text);
}

