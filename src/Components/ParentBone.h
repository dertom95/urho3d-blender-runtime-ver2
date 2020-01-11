#pragma once

// this component is used from within the dertom's urho3d-exporter for bone-parenting

#include <Urho3D/Urho3DAll.h>

class ParentBone : public LogicComponent
{
    URHO3D_OBJECT(ParentBone,LogicComponent);
public:
    static void RegisterObject(Context *context);

    ParentBone(Context* ctx);

    virtual void DelayedStart() override;

private:
    String boneName;
};


