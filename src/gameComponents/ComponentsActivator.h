
#include <Urho3D/Core/Object.h>
#include "Rotator.h"
#include "GroupInstance.h"
#include "ParentBone.h"
#include "RotationFix.h"

using namespace Urho3D;

class ComponentsActivator{
public:
    static void RegisterComponents(Context* context);
};

void ComponentsActivator::RegisterComponents(Context *context)
{
    // mandataory for some exporter features
    ParentBone::RegisterObject(context);
    GroupInstance::RegisterObject(context);
    Rotator::RegisterObject(context);

    // custom components
    RotationFix::RegisterObject(context);
}
