//
// Copyright (c) 2008-2019 the Urho3D project.
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.
//

#include "BlenderNetwork.h"

#include <project_options.h>

#include <zmq.hpp>
#include <zmq_addon.hpp>
#include <Urho3D/Core/CoreEvents.h>
#include <Urho3D/IO/Log.h>
#include <Urho3D/Resource/JSONFile.h>

#include "BlenderNetworkEvents.h"


BlenderNetwork::BlenderNetwork(Context* context) :
    Object(context)
    ,running_(false)
    ,initialized_(false)
{
    SubscribeToEvent(E_BEGINFRAME, URHO3D_HANDLER(BlenderNetwork, HandleBeginFrame));
}

void BlenderNetwork::RegisterObject(Context* context)
{
    //context->RegisterFactory<Network>("Sample Component");

//    // These macros register the class attributes to the Context for automatic load / save handling.
//    // We specify the Default attribute mode which means it will be used both for saving into file, and network replication
//    URHO3D_ATTRIBUTE("BOOL VALUE", bool, boolValue_, DEFAULT_BOOL_VALUE, AM_FILE);
//    URHO3D_ATTRIBUTE("INT VALUE", int, intValue_, DEFAULT_INT_VALUE, AM_FILE);
//    URHO3D_ATTRIBUTE("FLOAT VALUE", float, floatValue_, DEFAULT_FLOAT_VALUE, AM_FILE);
//    URHO3D_ATTRIBUTE("STRING VALUE", String, stringValue_, DEFAULT_STRING_VALUE, AM_FILE);
//    //URHO3D_ATTRIBUTE("Anim Run", String, animRun_, "", AM_DEFAULT);
//    URHO3D_MIXED_ACCESSOR_ATTRIBUTE("Animation", GetAnimation, SetAnimation, ResourceRef, ResourceRef(Animation::GetTypeStatic()), AM_DEFAULT);
//    URHO3D_MIXED_ACCESSOR_ATTRIBUTE("Texture", GetTexture, SetTexture, ResourceRef, ResourceRef(Texture2D::GetTypeStatic()), AM_DEFAULT);
}

void BlenderNetwork::InitNetwork()
{
    inSocket_ = zmq::socket_t(ctx, zmq::socket_type::sub);
    inSocket_.connect("tcp://localhost:5560");
    String topic("blender");
    inSocket_.setsockopt(ZMQ_SUBSCRIBE, topic.CString(),topic.Length());
    inSocket_.setsockopt(ZMQ_RCVTIMEO, 50);

    outSocket_ = zmq::socket_t(ctx, zmq::socket_type::pub);
    outSocket_.connect("tcp://localhost:5559");

}

void BlenderNetwork::CheckNetwork()
{
    zmq::multipart_t multipart;

    auto ok = multipart.recv(inSocket_);
    zmq::message_t zmq_msg;
    if (ok){
        int mS = multipart.size();

        if (mS != 3){
            URHO3D_LOGERRORF("BlenderNetwork: WRONG AMOUNT OF MULTIPART_MSGs %i",mS);
            return;
        }

        auto _topic = multipart.popstr();
        auto _meta = multipart.popstr();

        Vector<String> topicSplit = String(_topic.c_str()).Split(' ');

        if (topicSplit.Size() != 3){
            URHO3D_LOGERRORF("BlenderNetwork: WRONG TOPIC-FORMAT! %s",_topic);
            return;
        }

        auto topic = topicSplit[0];
        auto subtype = topicSplit[1];
        auto datatype = topicSplit[2];
        auto meta = String(_meta.c_str());

        using namespace BlenderConnect;
        VariantMap map;
        map[P_TOPIC]=topic;
        map[P_SUBTYPE]=subtype;
        map[P_DATATYPE]=datatype;

        if (meta!=""){
            JSONFile metaJson(context_);
            metaJson.FromString(meta);
            auto root = metaJson.GetRoot().GetObject();
            map[P_META]=MakeCustomValue(root);
        }

        zmq_msg = multipart.pop();
        if (datatype == "text"){
            std::string data(zmq_msg.data<char>(),zmq_msg.size());
            map[P_DATA] = String(data.c_str());
            SendEvent(E_BLENDER_MSG,map);
        }
        else if (datatype == "json"){
            std::string data(zmq_msg.data<char>(),zmq_msg.size());

            JSONFile jsonFile(context_);
#ifdef GAME_DEBUGGING
            //URHO3D_LOGINFOF("json-data: %s",data.c_str());
#endif
            jsonFile.FromString(Urho3D::String(data.c_str()));
            auto root = jsonFile.GetRoot().GetObject();
           // CustomVariantValueImpl<JSONFile> val(jsonFile);
            if (root.Contains("view_matrix")){
                auto v = root["view_matrix"];
                int a=0;
            }
            map[P_DATA]=MakeCustomValue(root);
        } else {
            URHO3D_LOGERRORF("BlenderNetwork: unsupported datatype:%s",datatype.CString());
        }
        SendEvent(E_BLENDER_MSG,map);



      //  URHO3D_LOGINFOF("NETWORK:%s\n",recv_msg.c_str());
        //URHO3D_LOGINFO("---NET--");
    } else {
       //URHO3D_LOGINFO("NETWORK: no result");
    }
}

void BlenderNetwork::HandleBeginFrame(StringHash eventType, VariantMap &eventData)
{
    CheckNetwork();
}

void BlenderNetwork::Close()
{
    inSocket_.close();
    outSocket_.close();
    ctx.close();
}

void BlenderNetwork::Send(const String& topic,const String& subtype, void *buffer,int length, const String& meta)
{
    zmq::multipart_t multipart;
    multipart.addstr((topic+" "+subtype+" bin").CString());
    multipart.addstr(meta.CString());
    multipart.add(zmq::message_t(buffer,length));
    multipart.send(outSocket_);
}

void BlenderNetwork::Send(const String& topic,const String& subtype, const String& txtData, const String& meta)
{
    zmq::multipart_t multipart;
//    multipart.push(zmq::message_t(topic.CString(),topic.Length()));
//    multipart.push(zmq::message_t(txtData.CString(),topic.Length()));
    multipart.addstr((topic+" "+subtype+" text").CString());
    multipart.addstr(meta.CString());
    multipart.addstr(txtData.CString());
    multipart.send(outSocket_);
}

//void BlenderNetwork::CreateScreenshot()
//{
//    if (additionalResourcePath=="") return;

//    Graphics* graphics = GetSubsystem<Graphics>();
//    Image screenshot(context_);
//    graphics->TakeScreenShot(screenshot);
//    // Here we save in the Data folder with date and time appended
////    screenshot.SavePNG(GetSubsystem<FileSystem>()->GetProgramDir() + "Data/Screenshot_" +
////        Time::GetTimeStamp().Replaced(':', '_').Replaced('.', '_').Replaced(' ', '_') + ".png");
//    String screenshotPath = additionalResourcePath+"/Screenshot.png";
//    screenshot.SavePNG(screenshotPath);
//   // URHO3D_LOGINFOF("Save screenshot to %s",screenshotPath.CString());
//}







