#pragma once

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


