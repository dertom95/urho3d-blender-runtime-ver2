#pragma once

#include <Urho3D/Engine/Engine.h>
#include <Urho3D/Engine/Application.h>

#include "gamelogic.h"

using namespace Urho3D;

class TheGameMain : public Application {
    URHO3D_OBJECT(TheGameMain, Application);

public:
    TheGameMain(Context* context);

    void Setup() override;
    void Start() override;
    void Stop() override;
    void SubscribeToEvents();
    void HandleKeyDown(StringHash eventType, VariantMap& eventData);
private:
    SharedPtr<GameLogic> game_;
};
