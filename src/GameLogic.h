#pragma once

#include <project_options.h>

#include <Urho3D/Core/Object.h>
#include <Urho3D/Scene/Scene.h>
#include <Urho3D/Graphics/Viewport.h>
#include <Urho3D/Container/Str.h>

#include "Subsystems/Editor.h"

using namespace Urho3D;

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

    void SetUIText(String text);

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

#ifdef GAME_ENABLE_DEBUG_TOOLS
    void HandleConsoleInput(StringHash eventType, VariantMap& eventData);
#endif

    Node* mCameraNode;
    Scene* mScene;
    Viewport* mViewport;

    SoundSource* musicSource_;

    bool mRenderPhysics;

    SharedPtr<Window> window_;
    /// The UI's root UIElement.
    SharedPtr<UIElement> uiRoot_;
};
