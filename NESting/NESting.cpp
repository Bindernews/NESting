#include "NESting.h"
#include "IPlug_include_in_plug_src.h"
#include "math_utils.h"

NESting::NESting(const InstanceInfo& info)
: Plugin(info, MakeConfig(kNumParams, 1))
{
    GetParam(iParamGain)->InitGain("gain");
    GetParam(iParamFreq)->InitFrequency("freq", 440);
    GetParam(iParamDuty)->InitInt("duty", 2, 0, 4);
    GetParam(iParamShape)->InitInt("shape", 0, 0, 2);

#if IPLUG_DSP
    mFaustTriangle.SetMaxChannelCount(MaxNChannels(ERoute::kInput), MaxNChannels(ERoute::kOutput));
    mFaustTriangle.Init();

    mFaustSquare.SetMaxChannelCount(MaxNChannels(ERoute::kInput), MaxNChannels(ERoute::kOutput));
    mFaustSquare.Init();
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
    int shape = GetParam(iParamShape)->Int();
    switch (shape) {
    case 0:
        mFaustSquare.ProcessBlock(faustInputs, outputs, nFrames);
        break;
    case 1:
        mFaustTriangle.ProcessBlock(faustInputs, outputs, nFrames);
        break;
    case 2:
        mNoise.Compute(faustInputs, outputs, nFrames);
        break;
    }
    mScopeSender.ProcessBlock(outputs, nFrames, kCtrlTagScope);
}

void NESting::OnReset()
{
    mFaustSquare.SetSampleRate(GetSampleRate());
    mFaustTriangle.SetSampleRate(GetSampleRate());
    mNoise.SetParameter(0, GetSampleRate());

    mInputBuffer.Resize(int(GetSampleRate()), true);
    for (int i = 0; i < mInputBuffer.GetSize(); i += 1) {
        mInputBuffer.Get()[i] = 0.f;
    }
}

void NESting::OnParamChange(int paramIdx)
{
    double vNorm = GetParam(paramIdx)->GetNormalized();
    double vReal = GetParam(paramIdx)->Value();
    switch (paramIdx) {
    case iParamGain:
        mFaustSquare.SetParameterValue("gain", vNorm);
        mFaustTriangle.SetParameterValue("gain", vNorm);
        mNoise.SetParameter(3, vNorm);
        break;
    case iParamFreq:
        {
            double fNorm = unlerp(20, 20000, vReal);
            double fNoise = unlerp(0, 10010, vReal);
            mFaustSquare.SetParameterValueNormalised(1, fNorm);
            mFaustTriangle.SetParameterValueNormalised(0, fNorm);
            mNoise.SetParameter(1, vNorm);
            break;
        }    
    case iParamDuty:
        mFaustSquare.SetParameterValueNormalised(0, vNorm);
        break;
    case iParamShape:
        break;
    }
}

void NESting::OnIdle()
{
  mScopeSender.TransmitData(*this);
}
#endif
