#include "gamelogic.h"
#include <Urho3D/Resource/ResourceCache.h>
#include <Urho3D/Urho3DAll.h>

GameLogic::GameLogic(Context* ctx)
    : Object(ctx),
      mCameraNode(nullptr),
      mScene(nullptr),
      mLightNode(nullptr),
      mViewport(nullptr)
{
}

GameLogic::~GameLogic(){
}

void GameLogic::Setup(VariantMap& engineParameters_)
{
    engineParameters_[EP_FULL_SCREEN]=false;
    engineParameters_[EP_WINDOW_RESIZABLE]=true;
    engineParameters_[EP_WINDOW_WIDTH]=840;
    engineParameters_[EP_WINDOW_HEIGHT]=480;
}

void GameLogic::Start()
{
    SetupScene();
    SetupViewport();
    SetupInput();
}

void GameLogic::SetupScene(){
    mScene = new Scene(context_);
    LoadScene("Scene.xml");
    mCameraNode = mScene->CreateChild("cameranode");
    mCameraNode->CreateComponent<Camera>();

    mLightNode = mScene->CreateChild("light");
    Node* node = mScene->GetChild("Cube",true);
    mCameraNode->SetPosition(Vector3(0,10,0));
    mCameraNode->LookAt(node->GetWorldPosition());
    mLightNode->LookAt(node->GetWorldPosition());

    mLightNode = mScene->CreateChild("DirectionalLight");
    //lightNode->SetDirection(Vector3(0.6f, -1.0f, 0.8f)); // The direction vector does not need to be normalized
    mLightNode->SetRotation(Quaternion(18.0f,55.0f,-17.0f));
    Light* light = mLightNode->CreateComponent<Light>();
    light->SetLightType(LIGHT_DIRECTIONAL);
    light->SetCastShadows(true);
    light->SetShadowBias(BiasParameters(0.00025f, 0.5f));
    light->SetShadowCascade(CascadeParameters(10.0f, 50.0f, 200.0f, 0.0f, 0.8f));
    light->SetSpecularIntensity(0.5f);
}

void GameLogic::SetupInput()
{
    Input* input = GetSubsystem<Input>();
    input->SetMouseVisible(true);
}

void GameLogic::SetupViewport()
{
    Renderer* renderer = GetSubsystem<Renderer>();
    mViewport = new Viewport(context_, mScene, mCameraNode->GetComponent<Camera>());
    renderer->SetViewport(0,mViewport);
}

void GameLogic::LoadScene(String sceneName)
{
    FileSystem* fs = GetSubsystem<FileSystem>();
    auto resourcePath = fs->GetProgramDir() + "Data";
    auto exists = fs->DirExists(resourcePath);

    ResourceCache* cache=GetSubsystem<ResourceCache>();
    cache->AddResourceDir(resourcePath);

    SharedPtr<File> file = cache->GetFile("Scenes/"+sceneName);
    if (!file.Null()){
        mScene->LoadXML(*file);
    } else {
        URHO3D_LOGERROR("no scene");
    }
}
