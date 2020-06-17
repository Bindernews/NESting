#include "NESting.h"
#include "IPlug_include_in_plug_src.h"
#include "FaustCode.hpp"

NESting::NESting(const InstanceInfo& info)
: Plugin(info, MakeConfig(kNumParams, 1))
{
  InitParamRange(0, kNumParams-1, 0, "Param %i", 0., 0., 1., 0.01, "", IParam::kFlagsNone); // initialize kNumParams generic iplug params
  
#if IPLUG_DSP
  mFaustProcessor.SetMaxChannelCount(MaxNChannels(ERoute::kInput), MaxNChannels(ERoute::kOutput));
  mFaustProcessor.Init();
  mFaustProcessor.CreateIPlugParameters(this); // in order to create iplug params, based on faust .dsp params, uncomment this
#endif
  
#if IPLUG_EDITOR
  mMakeGraphicsFunc = [&]() {
    return MakeGraphics(*this, PLUG_WIDTH, PLUG_HEIGHT, PLUG_FPS, GetScaleForScreen(PLUG_HEIGHT));
  };
  
  mLayoutFunc = [&](IGraphics* pGraphics) {
    IRECT b = pGraphics->GetBounds().GetPadded(-20);

    IRECT knobs = b.GetFromTop(100.);
    IRECT viz = b.GetReducedFromTop(100);
    pGraphics->AttachCornerResizer(EUIResizerMode::Scale);
    pGraphics->LoadFont("Roboto-Regular", ROBOTO_FN);

    for (int i = 0; i < kNumParams; i++) {
      pGraphics->AttachControl(new IVKnobControl(knobs.GetGridCell(i, 1, kNumParams), i));
    }
    
    pGraphics->AttachPanelBackground(COLOR_GRAY);
    pGraphics->AttachControl(new IVScopeControl<2>(viz, "", DEFAULT_STYLE.WithColor(kBG, COLOR_BLACK).WithColor(kFG, COLOR_GREEN)), kCtrlTagScope);
  };
#endif
}

#if IPLUG_DSP

void NESting::ProcessBlock(sample** inputs, sample** outputs, int nFrames)
{
  sample* faustInputs[] = { mInputBuffer.Get()  };
  mFaustProcessor.ProcessBlock(faustInputs, outputs, nFrames);
  mScopeSender.ProcessBlock(outputs, nFrames, kCtrlTagScope);
}

void NESting::OnReset()
{
  mFaustProcessor.SetSampleRate(GetSampleRate());

  mInputBuffer.Resize(int(GetSampleRate()), true);
  for (int i = 0; i < mInputBuffer.GetSize(); i += 1) {
      mInputBuffer.Get()[i] = 0.f;
  }
}

void NESting::OnParamChange(int paramIdx)
{
  mFaustProcessor.SetParameterValueNormalised(paramIdx, GetParam(paramIdx)->GetNormalized());
}

void NESting::OnIdle()
{
  mScopeSender.TransmitData(*this);
}
#endif
