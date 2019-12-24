#include "gamelogic.h"
#include <Urho3D/Resource/ResourceCache.h>
#include <Urho3D/Urho3DAll.h>

GameLogic::GameLogic(Context* ctx)
    :Object(ctx)
{
    scene = new Scene(ctx);
}

GameLogic::~GameLogic(){
}

void GameLogic::Setup(VariantMap& engineParameters_)
{
    engineParameters_[EP_FULL_SCREEN]=false;
    engineParameters_[EP_WINDOW_RESIZABLE]=true;
    engineParameters_[EP_WINDOW_WIDTH]=840;
    engineParameters_[EP_WINDOW_HEIGHT]=480;
    engineParameters_[EP_RESOURCE_PATHS]="Data";
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
        scene->LoadXML(*file);
    } else {
        URHO3D_LOGERROR("no scene");
    }

    Node* node = scene->GetChild("thecube",true);

    Animation * main_layers = cache->GetResource<Animation>("Models/master_layer.ani");
    auto triggers = main_layers->GetTriggers();
    for (AnimationTriggerPoint tp : triggers){
        auto data = tp.data_;
        auto dt = tp.time_;
        int a=0;
    }
    int a=0;


}
