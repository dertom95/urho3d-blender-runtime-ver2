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

using namespace Urho3D;

const float DEFAULT_ROTATOR_SPEED = 1.0f;

/// Rotator component, responsible for physical movement according to controls, as well as animation.
class Rotator : public LogicComponent
{
    URHO3D_OBJECT(Rotator, LogicComponent);

public:
    /// Construct.
    explicit Rotator(Context* context);

    /// Register object factory and attributes.
    static void RegisterObject(Context* context);

    /// Handle startup. Called by LogicComponent base class.
    void DelayedStart() override;

    /// Handle physics world update. Called by LogicComponent base class.
    void Update(float timeStep) override;

private:
    float speed_;
    Vector3 axis_;
};
