#pragma once

// this component is used from within the dertoms urho3d-exporter for fixing wrong rotations
// (urho3d has a different 0|0|0 direction than blender and I couldn't get in right on the exporter-side :D )

#include <Urho3D/Scene/LogicComponent.h>

using namespace Urho3D;

namespace Urho3D{
class AnimationController;
}

class LookToMoveDirection : public LogicComponent
{
    URHO3D_OBJECT(LookToMoveDirection,LogicComponent);
public:
    static void RegisterObject(Context *context);

    LookToMoveDirection(Context* ctx);

    void DelayedStart() override;
    void Update(float timeStep) override;
private:
    Vector3 mLastPosition;
    WeakPtr<AnimationController> mAnimController;


};
