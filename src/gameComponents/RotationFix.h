#pragma once

#include <Urho3D/Scene/LogicComponent.h>

using namespace Urho3D;

class RotationFix : public LogicComponent
{
    URHO3D_OBJECT(RotationFix,LogicComponent);
public:
    static void RegisterObject(Context *context);

    RotationFix(Context* ctx);

    void DelayedStart() override;

};
