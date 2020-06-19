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
    GetParam(iParamGate)->InitBool("gate", false);

#if IPLUG_DSP
    mSynth.AddVoice(new NESVoice(*this), 0);
    mTestVoice = new NESVoice(*this);
    mGateOn = false;
#endif
  
#if IPLUG_EDITOR
    mMakeGraphicsFunc = [&]() {
        return MakeGraphics(*this, PLUG_WIDTH, PLUG_HEIGHT, PLUG_FPS, GetScaleForScreen(PLUG_HEIGHT));
    };

    mLayoutFunc = [&](IGraphics* pGraphics) {
        pGraphics->EnableMouseOver(true);
        pGraphics->EnableMultiTouch(true);

        IRECT b = pGraphics->GetBounds().GetPadded(-20);

        IRECT knobs = b.GetFromTop(100.);
        IRECT viz = b.GetAltered(0., 100., 0., -210.);
        pGraphics->AttachCornerResizer(EUIResizerMode::Scale);
        pGraphics->LoadFont("Roboto-Regular", ROBOTO_FN);

        for (int i = 0; i < kNumParams; i++) {
            pGraphics->AttachControl(new IVKnobControl(knobs.GetGridCell(i, 1, kNumParams), i));
        }
    
        pGraphics->AttachPanelBackground(COLOR_GRAY);
        pGraphics->AttachControl(new IVScopeControl<2>(viz, "", DEFAULT_STYLE.WithColor(kBG, COLOR_BLACK).WithColor(kFG, COLOR_GREEN)), kCtrlTagScope);
#ifdef APP_API
        IRECT keyboardBounds = b.GetFromBottom(200);
        pGraphics->AttachControl(new IVKeyboardControl(keyboardBounds), kCtrlTagKeyboard);
        pGraphics->SetQwertyMidiKeyHandlerFunc([pGraphics](const IMidiMsg& msg) {
            dynamic_cast<IVKeyboardControl*>(pGraphics->GetControlWithTag(kCtrlTagKeyboard))->SetNoteFromMidi(msg.NoteNumber(), msg.StatusMsg() == IMidiMsg::kNoteOn);
        });
#endif
       
    };
#endif
}

#if IPLUG_DSP

void NESting::ProcessBlock(sample** inputs, sample** outputs, int nFrames)
{
    sample* faustInputs[] = { mInputBuffer.Get() };
    // Pre-zero output buffers
    zero_buffers(outputs, 2, nFrames);
    mSynth.ProcessBlock(faustInputs, outputs, 1, 2, nFrames);
    if (mGateOn || mTestVoice->GetBusy())
    {
        double oscFreq = GetParam(iParamFreq)->Value();
        double gain = mTestVoice->mADSR.Process(1) * GetParam(iParamGain)->GetNormalized();
        mTestVoice->ProcessSamples(oscFreq, gain, faustInputs, outputs, nFrames);
    }
    memcpy(outputs[1], outputs[0], sizeof(sample) * nFrames);
    mScopeSender.ProcessBlock(outputs, nFrames, kCtrlTagScope);
}

void NESting::ProcessMidiMsg(const IMidiMsg& msg)
{
    TRACE;

    int status = msg.StatusMsg();

    switch (status)
    {
        case IMidiMsg::kNoteOn:
        case IMidiMsg::kNoteOff:
        case IMidiMsg::kPolyAftertouch:
        case IMidiMsg::kControlChange:
        case IMidiMsg::kProgramChange:
        case IMidiMsg::kChannelAftertouch:
        case IMidiMsg::kPitchWheel:
        {
            mSynth.AddMidiMsgToQueue(msg);
            SendMidiMsg(msg);
            break;
        }
        default:
            return;
    }
}

void NESting::OnReset()
{
    mSynth.Reset();
    mSynth.SetSampleRateAndBlockSize(GetSampleRate(), GetBlockSize());
    mTestVoice->SetSampleRateAndBlockSize(GetSampleRate(), GetBlockSize());

    mInputBuffer.Resize(int(GetSampleRate()), true);
    for (int i = 0; i < mInputBuffer.GetSize(); i += 1) {
        mInputBuffer.Get()[i] = 0.f;
    }
}

void NESting::OnParamChange(int paramIdx)
{
    double vNorm = GetParam(paramIdx)->GetNormalized();
    double vReal = GetParam(paramIdx)->Value();

    switch (paramIdx)
    {
    case iParamGate:
        {
            bool newGate = GetParam(paramIdx)->Bool();
            if (newGate != mGateOn)
            {
                if (newGate)
                {
                    mTestVoice->Trigger(1.0, false);
                }
                else
                {
                    mTestVoice->Release();
                }
                mGateOn = newGate;
            }
        }
    }

    for (size_t i = 0; i < mSynth.NVoices(); i += 1) {
        auto voice = dynamic_cast<NESVoice*>(mSynth.GetVoice(i));
        
        switch (paramIdx) {
        case iParamGain:
            voice->mOwnerGain = vNorm;
            break;
        case iParamFreq:
        {
            double fNorm = unlerp(20., 20000., vReal);
            double fNoise = unlerp(0., 10010., vReal);
            voice->mFaustSquare.SetParameterValueNormalised(1, fNorm);
            voice->mFaustTriangle.SetParameterValueNormalised(0, fNorm);
            voice->mNoise.SetParameter(1, vNorm);
            break;
        }
        case iParamDuty:
            voice->mFaustSquare.SetParameterValueNormalised(0, vNorm);
            break;
        case iParamShape:
            voice->mShape = GetParam(paramIdx)->Int();
            break;
        }
    };
}

bool NESting::OnMessage(int msgTag, int ctrlTag, int dataSize, const void* pData)
{
    if (ctrlTag == kCtrlTagBender && msgTag == IWheelControl::kMessageTagSetPitchBendRange)
    {
        const int bendRange = *static_cast<const int*>(pData);
        mSynth.SetPitchBendRange(bendRange);
    }
    return false;
}

void NESting::OnIdle()
{
  mScopeSender.TransmitData(*this);
}
#endif
