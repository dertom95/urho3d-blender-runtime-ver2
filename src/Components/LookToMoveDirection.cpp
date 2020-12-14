#include "LookToMoveDirection.h"

#include <Urho3D/Urho3DAll.h>

using namespace Urho3D;


LookToMoveDirection::LookToMoveDirection(Context *ctx)
    : LogicComponent(ctx)
{
    SetUpdateEventMask(USE_UPDATE);
}

void LookToMoveDirection::RegisterObject(Context *context)
{
    context->RegisterFactory<LookToMoveDirection>();
    URHO3D_ACCESSOR_ATTRIBUTE("Is Enabled", IsEnabled, SetEnabled, bool, true, AM_DEFAULT);
}

void LookToMoveDirection::DelayedStart()
{
    mLastPosition = node_->GetWorldPosition();
    mAnimController = node_->GetComponent<AnimationController>(true);
}

void LookToMoveDirection::Update(float timeStep)
{
    Vector3 currentWorldPosition = node_->GetWorldPosition();
    Vector3 direction = currentWorldPosition - mLastPosition;

    float length = direction.Length();

    //URHO3D_LOGINFOF("LEGNTH:%f",length);

    if (mAnimController && mAnimController->GetAnimations().Size()>0){
        auto animControl = mAnimController->GetAnimations()[0];
        if (length < 0.01){
            animControl.setTime_=0;
            mAnimController->SetEnabled(false);
        }
        else {
            if (!mAnimController->IsEnabled())
                mAnimController->SetEnabled(true);

            animControl.speed_=length*2;
        }
    }

    Quaternion rot;
    rot.FromLookRotation(direction,Vector3::RIGHT);
    node_->SetRotation(rot);

    mLastPosition = currentWorldPosition;
}



