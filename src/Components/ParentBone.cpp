#include "ParentBone.h"
#include <Urho3D/Urho3DAll.h>

ParentBone::ParentBone(Context *ctx)
    : LogicComponent(ctx)
{}

void ParentBone::RegisterObject(Context *context)
{
    context->RegisterFactory<ParentBone>("Sample Component");

    URHO3D_ATTRIBUTE("boneName", String, boneName, "", AM_DEFAULT);
}


void ParentBone::DelayedStart()
{
    if (boneName.Empty()) return;

    Node* bone = node_->GetParent()->GetChild(boneName,true);

    if (bone){
        node_->SetParent(bone);
    }
}
