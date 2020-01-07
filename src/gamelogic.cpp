#include "gamelogic.h"
#include <Urho3D/Resource/ResourceCache.h>
#include <Urho3D/Urho3DAll.h>

#include "gameComponents/TriggerController.h"
#include "dungeon.h"
#include "gameComponents/Character.h"
#include "gameComponents/CharacterController.h"

GameData::GameData(Context* ctx_)
    : Object(ctx_)
    , time(0)
    , timePerChest(10)
    , running(false)
    , currentLevel(1)
    , seeds(100)
    , playedStarter(false)
{
    gamestate = GS_STARTUP;
    for (int i=0;i<100;i++){
        seeds[i] = Random(1000000,2000000);
    }
}




GameLogic::GameLogic(Context* ctx)
    : Object(ctx),
      mCameraNode(nullptr),
      mScene(nullptr),
      mLightNode(nullptr),
      mViewport(nullptr),
      mRenderPhysics(false),
      mSpawner(nullptr),
      mEditor(nullptr)

{
    mGameData = new GameData(ctx);
    ctx->RegisterSubsystem(mGameData);
}

GameLogic::~GameLogic(){
}

void GameLogic::Setup(VariantMap& engineParameters_)
{
    engineParameters_[EP_FULL_SCREEN]=false;
    engineParameters_[EP_WINDOW_RESIZABLE]=true;
    engineParameters_[EP_WINDOW_WIDTH]=1700;
    engineParameters_[EP_WINDOW_HEIGHT]=1000;
    SubscribeToEvents();
}

void GameLogic::Start()
{
    SetupSystems();
    SetupScene();
    SetupViewport();
    SetupInput();
    SetupUI();

    PlayMusic("EvilMarch.ogg");
}

void GameLogic::SetupSystems()
{
    DungeonManager* dungeonManager = new DungeonManager(context_);
    dungeonManager->Generate(50,50,50);
    dungeonManager->_print();
    context_->RegisterSubsystem(dungeonManager);
}

void GameLogic::SetupScene()
{
    mScene = new Scene(context_);

    context_->RegisterSubsystem( mScene );

    // Create scene subsystem components
    LoadFromFile("Scenes/level1.xml");

    mLightNode = mScene->GetChild("Sun",true);
    Light* light = mLightNode->GetComponent<Light>();
    light->SetShadowBias(BiasParameters(0.00025f, 0.5f));
    // Set cascade splits at 10, 50 and 200 world units, fade shadows out at 80% of maximum shadow distance
    light->SetShadowCascade(CascadeParameters(10.0f, 50.0f, 200.0f, 0.0f, 0.8f));

    mSpawner = new Spawner(context_);
    context_->RegisterSubsystem(mSpawner);
    mSpawner->ScanScene();


    Character* character = mScene->GetComponent<Character>(true);
    if (character) {
        mGameData->player = character->GetNode();
    }

//    Node* zoneNode = mScene->CreateChild("Zone");
//    auto* zone = zoneNode->CreateComponent<Zone>();
//    zone->SetAmbientColor(Color(1.0f, 0.15f, 0.15f));
//    zone->SetFogColor(Color(0.5f, 0.5f, 0.7f));handle
//    zone->SetFogStart(300.0f);
//    zone->SetFogEnd(500.0f);
//    zone->SetBoundingBox(BoundingBox(-2000.0f, 2000.0f));

//    mLightNode = mScene->GetChild("Light",true);


      mCameraNode = mScene->GetChild("Camera",true);

      musicSource_ = mScene->CreateComponent<SoundSource>();
      // Set the sound type to music so that master volume control works correctly
      musicSource_->SetSoundType(SOUND_MUSIC);


////    mCameraNode = mScene->CreateChild("cameranode");
//    if (mLightNode)
//    {
//        Node* minicube = mScene->GetChild("minicube",true);
//        minicube->SetPosition(mLightNode->GetPosition());
//        mLightNode->MarkDirty();
//    } else
//    {
//        Node* lightNode = mScene->CreateChild("PointLight");
//        auto* light = lightNode->CreateComponent<Light>();
//        light->SetLightType(LIGHT_POINT);
//        Quaternion quat(0.891352f,0.388712f, 0.233228f, -0.0f);
//        lightNode->SetRotation(quat);
//        light->SetColor(Color::RED);



//        SharedPtr<ValueAnimation> colorAnimation(new ValueAnimation(context_));
//        colorAnimation->SetKeyFrame(0.0f, Color::WHITE);
//        colorAnimation->SetKeyFrame(1.0f, Color::RED);
//        colorAnimation->SetKeyFrame(2.0f, Color::YELLOW);
//        colorAnimation->SetKeyFrame(3.0f, Color::GREEN);
//        colorAnimation->SetKeyFrame(4.0f, Color::WHITE);
//        // Set Light component's color animation

//        SharedPtr<ObjectAnimation> lightAnimation(new ObjectAnimation(context_));
//        lightAnimation->AddAttributeAnimation("@Light/Color", colorAnimation);

//        // Apply light animation to light node
//        lightNode->SetObjectAnimation(lightAnimation);
//    }

//    DungeonManager* dm = GetSubsystem<DungeonManager>();
//    dm->CreateScene();
//    Vector3 tilePos = dm->GetFirstTilePos();
  //  mCameraNode->SetPosition(Vector3(tilePos.x_,10,tilePos.z_));
//    mCameraNode->LookAt(tilePos);


  /*  Node* scarecrow = mScene->CreateChild("scarecrow");
    scarecrow->Translate(Vector3(5,5,5));
    LoadScene("scarecrow.xml",scarecrow);
    mCameraNode->LookAt(scarecrow->GetWorldPosition());*/

}

void GameLogic::SetupInput()
{
    Input* input = GetSubsystem<Input>();
    //input->SetMouseMode(MM_WRAP);

}

void GameLogic::SetupViewport()
{
    Renderer* renderer = GetSubsystem<Renderer>();
    mViewport = new Viewport(context_, mScene, mCameraNode->GetComponent<Camera>());
    renderer->SetViewport(0,mViewport);
}

void GameLogic::LoadFromFile(String sceneName, Node* loadInto)
{
    FileSystem* fs = GetSubsystem<FileSystem>();
    auto resourcePath = fs->GetProgramDir() + "Data";
    auto exists = fs->DirExists(resourcePath);

    ResourceCache* cache=GetSubsystem<ResourceCache>();
    cache->AddResourceDir(resourcePath);

    XMLFile* file = cache->GetResource<XMLFile>(sceneName);
    if (file){
        if (loadInto != nullptr) {
            loadInto->LoadXML(file->GetRoot());
        } else {
            mScene->LoadXML(file->GetRoot());
        }
    } else {
        URHO3D_LOGERROR("no scene");
    }
}

void GameLogic::SubscribeToEvents()
{
    SubscribeToEvent(E_UPDATE, URHO3D_HANDLER(GameLogic, HandleUpdate));
    SubscribeToEvent(E_POSTRENDERUPDATE, URHO3D_HANDLER(GameLogic, HandlePostRenderUpdate));
    SubscribeToEvent(E_KEYDOWN, URHO3D_HANDLER(GameLogic, HandleInput));
}

void GameLogic::ResetPlayer()
{
    CharacterController* cc = mGameData->player->GetParentComponent<CharacterController>(true);
    if (cc) cc->ResetStartposition();
}

void GameLogic::HandleUpdate(StringHash eventType, VariantMap &eventData)
{
    Input* input = GetSubsystem<Input>();

    input->SetMouseVisible(input->GetMouseButtonDown(MOUSEB_RIGHT));

    if (input->GetKeyPress(KEY_F3)){
        mRenderPhysics = !mRenderPhysics;
    }

    using namespace Update;
    float dt = eventData[P_TIMESTEP].GetFloat();


    if (mGameData->gamestate == GameData::GS_LOST || mGameData->gamestate == GameData::GS_WON || mGameData->gamestate == GameData::GS_STARTUP) {
        if (mGameData->gamestate == GameData::GS_STARTUP) {
            mGameData->status="Welcome! Pick up all chests to promote to next level. Control: AWSD+Mouse SPACE Mouse-Wheel  Start in ";
            mGameData->time = 7;
        }
        if (mGameData->gamestate == GameData::GS_LOST) {
            GetSubsystem<GameLogic>()->PlaySound("death.ogg");
            ResetPlayer();
            mGameData->currentLevel = mGameData->currentLevel > 1 ? mGameData->currentLevel-1 : 1;

            mGameData->status="Lost! Downgrade Level! Start in ";
            mGameData->time = 5;
        }
        else if (mGameData->gamestate == GameData::GS_WON) {
            PlaySound("won.ogg");
            ResetPlayer();
            mGameData->playedStarter=true;

            mGameData->currentLevel++;
            mGameData->status="Won! Promoted to nex Level! Start in ";
            mGameData->time = 5;
        }

        for (Node* enemy : mGameData->mAllEnemies){
            enemy->Remove();
        }
        mGameData->mAllEnemies.Clear();

        for (Node* coin : mGameData->mAllCoins){
            coin->Remove();
        }
        mGameData->mAllCoins.Clear();

        mGameData->gamestate = GameData::GS_COUNTDOWN;
        mGameData->playedStarter = false;



    }

    if (mGameData->gamestate == GameData::GS_COUNTDOWN){
        if (!mGameData->playedStarter && mGameData->time <= 3){
            PlaySound("start.ogg");
            mGameData->playedStarter=true;
        }

        if (mGameData->time <= 0){
            mGameData->gamestate = GameData::GS_PLAYING;
            mGameData->time = mGameData->timePerChest * 5;
            int seed = mGameData->seeds[mGameData->currentLevel];
            URHO3D_LOGINFOF("Setting seed for level %i => %i",mGameData->currentLevel,seed);
            SetRandomSeed(seed);
            mSpawner->SpawnCoins(mGameData->currentLevel * 4);
            mSpawner->SpawnEnemies(mGameData->currentLevel * 2);
        }
    }

    if (mGameData->gamestate == GameData::GS_PLAYING) {
        if (mGameData->mAllCoins.Size() == 0){
            mGameData->gamestate = GameData::GS_WON;
        }

        if (mGameData->time <= 0){
            mGameData->gamestate = GameData::GS_LOST;
        }

        mGameData->status = String("Level ")+String(mGameData->currentLevel)+" Chests:"+String(mGameData->mAllCoins.Size())+" - Death in ";
    }

    SetUIText(mGameData->status + String( ceil(mGameData->time))  );

    mGameData->time -= dt;


}

void GameLogic::HandlePostRenderUpdate(StringHash eventType, VariantMap &eventData)
{
    if (mRenderPhysics) mScene->GetComponent<PhysicsWorld>()->DrawDebugGeometry(false);
}

void GameLogic::HandleInput(StringHash eventType, VariantMap &eventData)
{
    using namespace KeyDown;
    // Check for pressing ESC. Note the engine_ member variable for convenience access to the Engine object
    int key = eventData[P_KEY].GetInt();

    if (key == KEY_F12){
        if (!mEditor){
            mEditor = new Editor(context_);
            mEditor->InitEditor();
        }
    }

    if (key == KEY_F4) {
        mSpawner->SpawnCoins(10);
        mSpawner->SpawnEnemies(10);
    }

    if (key == KEY_F10){
        File saveFile(context_, "./scene.write.xml",FILE_WRITE);
        mScene->SaveXML(saveFile);
        File saveFileBin(context_, "./scene.bin",FILE_WRITE);
        mScene->Save(saveFileBin);
    }
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
    musicSource_->SetGain(0.35f);
    musicSource_->Play(music);
}

void GameLogic::SetupUI()
{
    uiRoot_ = GetSubsystem<UI>()->GetRoot();
    // Create the Window and add it to the UI's root node
    // Load XML file containing default UI style sheet
    auto* cache = GetSubsystem<ResourceCache>();
    auto* style = cache->GetResource<XMLFile>("UI/DefaultStyle.xml");
    // Set the loaded style as default style
    uiRoot_->SetDefaultStyle(style);


    window_ = new Window(context_);
    uiRoot_->AddChild(window_);

    // Set Window size and layout settings
    window_->SetMinWidth(784);
    window_->SetLayout(LM_VERTICAL, 6, IntRect(6, 6, 6, 6));
    window_->SetAlignment(HA_LEFT, VA_TOP);
    window_->SetName("Window");

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
    window_->AddChild(titleBar);

    // Apply styles
    window_->SetStyleAuto();
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
    auto* windowTitle = window_->GetChildStaticCast<Text>("WindowTitle", true);

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

void GameLogic::SetUIText(String text)
{
    auto* windowTitle = window_->GetChildStaticCast<Text>("WindowTitle", true);
    windowTitle->SetText(text);
}

Spawner::Spawner(Context* ctx_)
    : Object(ctx_)
    , gameLogic(nullptr)
{
    gameLogic = GetSubsystem<GameLogic>();
}

void Spawner::ScanScene()
{
    Scene* scene = GetSubsystem<Scene>();
    mCoinSpawner.Clear();
    scene->GetChildrenWithTag(mCoinSpawner,"spawn_coin",true);
    mEnemySpawner.Clear();
    scene->GetChildrenWithTag(mEnemySpawner,"spawn_enemy",true);
}

void Spawner::Spawn(PODVector<Node *> &spawnPoints, String spawnFilename, int amount,PODVector<Node*>& result)
{
    if (spawnPoints.Size() == 0) return;

    Node* templateNode = GetSubsystem<Scene>()->CreateChild("coin")->CreateChild();
    Node* currentNode = templateNode;
    gameLogic->LoadFromFile(spawnFilename, templateNode);


    for (int i=0; i < amount; i++){
        Node* spawner = spawnPoints[Random(0,spawnPoints.Size())];

        if (i>0) {
            currentNode = templateNode->Clone();
            spawner->GetScene()->AddChild(currentNode);
        }

        auto spPos = spawner->GetWorldPosition();
        currentNode->SetWorldPosition(spPos);
        TriggerController* tc =  currentNode->GetDerivedComponent<TriggerController>(true);
        if (tc) {
            tc->SetUserNode(currentNode);
        }
        result.Push(currentNode);
    }

    URHO3D_LOGINFOF("Created %i elements",result.Size());
}

void Spawner::SpawnCoins(int amount)
{
    GameData* gd = GetSubsystem<GameData>();
    Spawn(mCoinSpawner,"Objects/col_CoinChest.xml",amount,gd->mAllCoins);
}


void Spawner::SpawnEnemies(int amount)
{
    GameData* gd = GetSubsystem<GameData>();
    Spawn(mCoinSpawner,"Objects/col_Enemy.xml",amount,gd->mAllEnemies);
    //Spawn(mEnemySpawner,"Objects/col_Player.xml",amount,mAllEnemies);
}

