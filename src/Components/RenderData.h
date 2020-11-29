#pragma once

#include <Urho3D/Urho3DAll.h>

// this component is used from within the dertom's urho3d-exporter for collection-instances.
// on load the corresponding collections are loaded and added to this node
class RenderData : public LogicComponent
{
    URHO3D_OBJECT(RenderData,LogicComponent);
public:
    static void RegisterObject(Context *context);

    RenderData(Context* ctx);

    void SetRenderPathOnViewport(Viewport* vp);

    String mRenderPath;
    bool mEnabledGammaCorrection;
    bool mEnabledBloom;
    bool mEnabledFXAA2;
    bool mEnabledSRGB;
    bool mEnabledHDR;

};
