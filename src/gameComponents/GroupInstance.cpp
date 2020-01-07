#include "GroupInstance.h"

GroupInstance::GroupInstance(Context *ctx)
    : Component(ctx), groupRoot(0)
{}

void GroupInstance::RegisterObject(Context *context)
{
    context->RegisterFactory<GroupInstance>();

    URHO3D_ACCESSOR_ATTRIBUTE("groupFilename", GetGroupFilename, SetGroupFilename, String, String::EMPTY, AM_DEFAULT);
    URHO3D_ACCESSOR_ATTRIBUTE("groupOffset", GetGroupOffset, SetGroupOffset, Vector3, Vector3(0,0,0), AM_DEFAULT);
}

void GroupInstance::SetGroupFilename(const String &groupFilename)
{
    if (groupFilename == this->groupFilename) return;

    this->groupFilename=groupFilename;

    FileSystem* fs = GetSubsystem<FileSystem>();

    ResourceCache* cache = GetSubsystem<ResourceCache>();
    XMLFile* file = cache->GetResource<XMLFile>(groupFilename);
    groupRoot = node_->CreateChild("group_root");
    groupRoot->LoadXML(file->GetRoot());
//    for (auto child : groupRoot->GetChildren()){
//        node_->AddChild(child);
//    }
//    groupRoot->Remove();
}

void GroupInstance::SetGroupOffset(const Vector3& groupOffset){
    for (Node* node : groupRoot->GetChildren()){
        Vector3 gOffset = groupOffset * -1;
        float z = gOffset.z_;
        gOffset.z_ = gOffset.y_;
        gOffset.y_ = z;
      //  node->Translate(gOffset);
    }
    this->groupOffset = groupOffset;
}

