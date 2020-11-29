#include "RenderData.h"

RenderData::RenderData(Context *ctx)
    : LogicComponent(ctx)
    , mRenderPath("RenderPaths/Forward.xml")
    , mEnabledGammaCorrection(false)
    , mEnabledBloom(false)
    , mEnabledFXAA2(false)
    , mEnabledSRGB(false)
    , mEnabledHDR(false)
{
}

void RenderData::RegisterObject(Context *context)
{
    context->RegisterFactory<RenderData>();

   URHO3D_ACCESSOR_ATTRIBUTE("Is Enabled", IsEnabled, SetEnabled, bool, true, AM_DEFAULT);
   URHO3D_ATTRIBUTE("RenderPath", String, mRenderPath, "RenderPaths/Forward.xml", AM_DEFAULT);
   URHO3D_ATTRIBUTE("HDR", bool, mEnabledHDR, false, AM_DEFAULT);
   URHO3D_ATTRIBUTE("Gamma", bool, mEnabledGammaCorrection, false, AM_DEFAULT);
   URHO3D_ATTRIBUTE("Bloom", bool, mEnabledBloom, false, AM_DEFAULT);
   URHO3D_ATTRIBUTE("FXAA2", bool, mEnabledFXAA2, false, AM_DEFAULT);
   URHO3D_ATTRIBUTE("sRGB", bool, mEnabledSRGB, false, AM_DEFAULT);
}


void RenderData::SetRenderPathOnViewport(Viewport* vp)
{
    RenderPath* rp = new RenderPath();
    auto cache = context_->GetSubsystem<ResourceCache>();
    rp->Load(cache->GetResource<XMLFile>(mRenderPath));

    if (!rp){
        URHO3D_LOGERRORF("RenderDataComponent: Unknown Renderpath:%s",mRenderPath.CString());
        return;
    }

    auto graphics = context_->GetSubsystem<Graphics>();
    graphics->SetSRGB(mEnabledSRGB);

    auto renderer = context_->GetSubsystem<Renderer>();
    renderer->SetHDRRendering(mEnabledHDR);

    if (mEnabledGammaCorrection){
        rp->Append(cache->GetResource<XMLFile>("PostProcess/GammaCorrection.xml"));
        rp->SetEnabled("GammaCorrection",true);
    }

    if (mEnabledBloom){
        if (mEnabledHDR){
            rp->Append(cache->GetResource<XMLFile>("PostProcess/BloomHDR.xml"));
            rp->SetEnabled("BloomHDR",true);
        } else {
            rp->Append(cache->GetResource<XMLFile>("PostProcess/Bloom.xml"));
            rp->SetEnabled("Bloom",true);
            rp->SetShaderParameter("BloomMix",Vector2(0.9f, 0.6f));
            rp->SetShaderParameter("BloomThreshold",0.1f);
        }
    }

    if (mEnabledFXAA2){
        rp->Append(cache->GetResource<XMLFile>("PostProcess/FXAA2.xml"));
        rp->SetEnabled("FXAA2",true);
    }
    vp->SetRenderPath(rp);
}
