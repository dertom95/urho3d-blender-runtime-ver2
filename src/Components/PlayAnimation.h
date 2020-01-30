#pragma once

#include <Urho3D/Urho3DAll.h>

class PlayAnimation : public LogicComponent
{
    URHO3D_OBJECT(PlayAnimation,LogicComponent);
public:
    static void RegisterObject(Context *context);

    PlayAnimation(Context* ctx);

    void SetAnimationFile(const String& animationFile);
    const String& GetAnimationFile() const { return animationFile; }
    virtual void DelayedStart() override;

    void SetAnimation(const ResourceRef& value);
    ResourceRef GetAnimation() const;

private:
    String animationFile;
    float speed;
    AnimationController* animControl;
};


