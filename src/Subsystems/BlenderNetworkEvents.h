#pragma once

#include <Urho3D/Core/Object.h>

URHO3D_EVENT(E_PUBSUB_MSG, NSPubSubMessage)
{
    URHO3D_PARAM(P_TOPIC, Topic);  //string
    URHO3D_PARAM(P_MSG, Message); // string
}

URHO3D_EVENT(E_BLENDER_MSG, BlenderConnect)
{
    URHO3D_PARAM(P_TOPIC, Topic);  //string
    URHO3D_PARAM(P_SUBTYPE, SubType); // string
    URHO3D_PARAM(P_DATATYPE, DataType); // string
    URHO3D_PARAM(P_DATA, Data);  //Custom
    URHO3D_PARAM(P_META, Meta); // string
}


