#pragma once

#include <Urho3D/Core/Object.h>
#include <Urho3D/Scene/Scene.h>
#include <Urho3D/Graphics/Viewport.h>
#include <Urho3D/Container/Str.h>

using namespace Urho3D;

class GameLogic : public Object
{
    URHO3D_OBJECT(GameLogic,Object);

public:
    GameLogic(Context* ctx);
    ~GameLogic();

    void Setup(VariantMap& engineParameters_);
    void LoadScene(String sceneName);
    void Start();

private:
    void SetupViewport();
    void SetupScene();
    void SetupInput();

    Node* mCameraNode;
    Scene* mScene;
    Node* mLightNode;
    Viewport* mViewport;

};
