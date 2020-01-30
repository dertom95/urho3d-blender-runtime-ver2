#include "PlayAnimation.h"
#include <Urho3D/Urho3DAll.h>

PlayAnimation::PlayAnimation(Context *ctx)
    : LogicComponent(ctx),animControl(0),speed(1)
{}

void PlayAnimation::RegisterObject(Context *context)
{
    context->RegisterFactory<PlayAnimation>("Sample Component");

    URHO3D_MIXED_ACCESSOR_ATTRIBUTE("Animation", GetAnimation, SetAnimation, ResourceRef, ResourceRef(Animation::GetTypeStatic()), AM_DEFAULT);
    //URHO3D_ACCESSOR_ATTRIBUTE("animationFile", GetAnimationFile, SetAnimationFile, String, String::EMPTY, AM_DEFAULT);
    URHO3D_ATTRIBUTE("speed", float, speed, 1.0f, AM_DEFAULT);
}

void PlayAnimation::SetAnimationFile(const String &animFile)
{
    if (animFile == this->animationFile) return;

    this->animationFile=animFile;
    Start();
}



void PlayAnimation::DelayedStart()
{
    if (animationFile.Empty()) return;
//    AnimatedModel* amodel = node_->GetComponent<AnimatedModel>();
  //  auto model = amodel->GetModel();
//    node_->RemoveComponent<AnimatedModel>();
//    amodel = node_->CreateComponent<AnimatedModel>();
//    amodel->SetModel(model);

    if (!animControl){
        animControl = node_->CreateComponent<AnimationController>();
    }

    // example of using different data with custommade ui in which you can
    // choose the animation in the node or an absolute path. THe selection results
    // in a not valid file so we need to create it here (better of course in the exporter)
    String animFile = animationFile.EndsWith(".ani")?animationFile : "Models/"+animationFile+".ani";

    animControl->PlayExclusive(animFile,0, true, 0.0f);
    animControl->SetSpeed(animFile,speed);
    animControl->SetAnimationEnabled(true);
}

ResourceRef PlayAnimation::GetAnimation() const
{
    return ResourceRef(Animation::GetTypeStatic(),animationFile);
}

void PlayAnimation::SetAnimation(const ResourceRef &value)
{
    animationFile = value.name_;
}
