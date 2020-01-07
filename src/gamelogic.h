#pragma once

#include <Urho3D/Core/Object.h>
#include <Urho3D/Scene/Scene.h>
#include <Urho3D/Graphics/Viewport.h>
#include <Urho3D/Container/Str.h>

#include "gameSubsystems/Editor.h"

using namespace Urho3D;

class Spawner;



class GameData : public Object
{
    URHO3D_OBJECT(GameData,Object);
public:
    enum GameState {
      GS_COUNTDOWN,GS_PAUSED,GS_PLAYING,GS_LOST,GS_WON,GS_STARTUP
    };

    explicit GameData(Context* ctx_);

    Vector<int> seeds;
    PODVector<Node*> mAllCoins;
    PODVector<Node*> mAllEnemies;
    int currentLevel;
    float time;
    bool running;
    float timePerChest;
    String status;
    bool playedStarter;

    Node* player;

    GameState gamestate;

};

class GameLogic : public Object
{
    URHO3D_OBJECT(GameLogic,Object);

public:
    GameLogic(Context* ctx);
    ~GameLogic();

    void Setup(VariantMap& engineParameters_);
    void LoadFromFile(String sceneName,Node* node=nullptr);
    void Start();
    inline Scene* GetScene() { return mScene; }

    void PlaySound(String soundFile);
    void PlayMusic(String musicFile);

    GameData* mGameData;

    void SetUIText(String text);
    void ResetPlayer();

private:
    void SubscribeToEvents();
    void SetupSystems();
    void SetupViewport();
    void SetupScene();
    void SetupInput();
    void SetupUI();

    void HandleUpdate(StringHash eventType, VariantMap &eventData);
    void HandlePostRenderUpdate(StringHash eventType, VariantMap &eventData);
    void HandleInput(StringHash eventType, VariantMap& eventData);
    void HandleControlClicked(StringHash eventType, VariantMap& eventData);

    Node* mCameraNode;
    Scene* mScene;
    Node* mLightNode;
    Viewport* mViewport;

    SoundSource* musicSource_;

    Spawner* mSpawner;

    bool mRenderPhysics;

    Editor* mEditor;

    SharedPtr<Window> window_;
    /// The UI's root UIElement.
    SharedPtr<UIElement> uiRoot_;


};

class Spawner : public Object
{
public:
    URHO3D_OBJECT(Spawner,Object);

    Spawner(Context* ctx);

    void ScanScene();
    void SpawnCoins(int amount);
    void SpawnEnemies(int amount);
    GameLogic* gameLogic;

private:
    void Spawn(PODVector<Node*>& spawnPoints, String spawnFilename, int amount, PODVector<Node*>& result);
    PODVector<Node*> mCoinSpawner;
    PODVector<Node*> mEnemySpawner;


};
