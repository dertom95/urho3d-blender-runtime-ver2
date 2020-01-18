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

#pragma once

#include <Urho3D/Input/Controls.h>
#include <Urho3D/Scene/LogicComponent.h>
#include <Urho3D/Graphics/AnimationController.h>
#include <3rd/cppzmq/zmq.hpp>

using namespace Urho3D;


// TODO: Rebuild to be base on PubSubNetwork

/// Network component, responsible for physical movement according to controls, as well as animation.
class BlenderNetwork : public Object
{
    URHO3D_OBJECT(BlenderNetwork, Object);

public:
    /// Construct.
    explicit BlenderNetwork(Context* context);

    /// Register object factory and attributes.
    static void RegisterObject(Context* context);

    void InitNetwork();
    void CheckNetwork();
    void Close();
    /// Handle begin frame event.
    void HandleBeginFrame(StringHash eventType, VariantMap& eventData);
    void Send(const String& topic,const String& subtype,const String& txtData, const String& meta="");
    void Send(const String& topic,const String& subtype,void* buffer,int length, const String& meta="");
private:
    bool running_;
    zmq::socket_t  inSocket_;
    zmq::socket_t  outSocket_;
    bool initialized_;
    zmq::context_t ctx;

};
