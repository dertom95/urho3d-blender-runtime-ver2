#pragma once

// this component is used from within the dertoms urho3d-exporter for fixing wrong rotations
// (urho3d has a different 0|0|0 direction than blender and I couldn't get in right on the exporter-side :D )

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
