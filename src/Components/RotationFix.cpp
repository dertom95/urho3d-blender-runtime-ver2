#include "RotationFix.h"

#include <Urho3D/Urho3DAll.h>

using namespace Urho3D;


RotationFix::RotationFix(Context *ctx)
    : LogicComponent(ctx)
{}

void RotationFix::RegisterObject(Context *context)
{
    context->RegisterFactory<RotationFix>();
    URHO3D_ACCESSOR_ATTRIBUTE("Is Enabled", IsEnabled, SetEnabled, bool, true, AM_DEFAULT);

}

void RotationFix::DelayedStart()
{
    node_->Rotate(Quaternion(90,0,90));
    SetEnabled(false);
}


