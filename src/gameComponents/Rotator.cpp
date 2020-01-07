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

#include "Rotator.h"

#include <Urho3D/Urho3DAll.h>


Rotator::Rotator(Context* context) :
    LogicComponent(context),
    speed_(DEFAULT_ROTATOR_SPEED)
{
    // Only the physics update event is needed: unsubscribe from the rest for optimization
    SetUpdateEventMask(USE_UPDATE);
}

void Rotator::RegisterObject(Context* context)
{
    context->RegisterFactory<Rotator>("Sample Component");

    // These macros register the class attributes to the Context for automatic load / save handling.
    // We specify the Default attribute mode which means it will be used both for saving into file, and network replication
    URHO3D_ATTRIBUTE("Axis", Vector3, axis_ , Vector3::ZERO, AM_FILE);
    URHO3D_ATTRIBUTE("Speed", float, speed_, DEFAULT_ROTATOR_SPEED, AM_FILE);
}

void Rotator::DelayedStart()
{
    // init whatever you want. at this point all nodes are already handled
    URHO3D_LOGINFO("STARTED");
    int a=0;
}

void Rotator::Update(float timeStep)
{
    node_->Rotate(Quaternion(axis_.x_*speed_*timeStep,axis_.y_*speed_*timeStep,axis_.z_*speed_*timeStep));
    // do the logic here

}
