#include "NESting.h"
#include "IPlug_include_in_plug_src.h"
#include "ADSREnvelope.h"
#include "math_utils.h"

#if IPLUG_DSP
class MyVoice : public SynthVoice
{
public:
    MyVoice(NESting& owner) : mOwner(owner), mADSR("gain", [&]() { mNoise.OnRelease(); })
    {
        mFaustTriangle.SetMaxChannelCount(mOwner.MaxNChannels(ERoute::kInput), mOwner.MaxNChannels(ERoute::kOutput));
        mFaustTriangle.Init();

        mFaustSquare.SetMaxChannelCount(mOwner.MaxNChannels(ERoute::kInput), mOwner.MaxNChannels(ERoute::kOutput));
        mFaustSquare.Init();

        mNoise.Reset();
        mOwnerGain = 0.5;
    }

    bool GetBusy() const override
    {
        return mADSR.GetBusy();
    }

    void Trigger(double level, bool isRetrigger) override
    {
        if (isRetrigger) {
            mADSR.Retrigger(sample(level));
        }
        else {
            mADSR.Start(sample(level));
        }
    }

    void Release() override
    {
        mADSR.Release();

        if (mOwner.mGraphics != nullptr)
        {
            auto pGraphics = static_cast<IGraphics*>(mOwner.mGraphics);
            auto lbl = dynamic_cast<IVLabelControl*>(pGraphics->GetControlWithTag(kCtrlTagFreqOut));
            WDL_String msg(0, 0);
            msg.AppendFormatted(20, "Freq: %.3f", mOscFreq);
            lbl->SetStr(msg.Get());
            lbl->SetValueStr(msg.Get());
            lbl->SetDirty(true);
        }
    }

    void ProcessSamplesAccumulating(sample** inputs, sample** outputs, int nInputs, int nOutputs, int startIdx, int nFrames) override
    {
        double pitch = mInputs[kVoiceControlPitch].endValue;
        double pitchBend = mInputs[kVoiceControlPitchBend].endValue;

        // Convert midi pitch into actual frequency
        double oscFreq = 440. * pow(2., pitch + pitchBend);
        
        mOscFreq = oscFreq;

        double gain = mADSR.Process(1) * mGain * mOwnerGain;

        sample* outputs2[2];
        sample_offset(outputs, outputs2, 2, startIdx);

        ProcessSamples(oscFreq, gain, inputs, outputs2, nFrames);
    }

    void ProcessSamples(double oscFreq, double gain, sample** inputs, sample** outputs, int nFrames)
    {
        // Normalize for triangle and square
        double freqNorm = unlerp(20., 20000., oscFreq);
        // Normalize "frequency" for the noise generator.
        double freqNoise = double(mKey % 16) / 16.0;

        switch (mShape) {
        case 0:
            mFaustSquare.SetParameterValueNormalised(1, freqNorm);
            mFaustSquare.SetParameterValue("gain", gain);
            mFaustSquare.ProcessBlock(inputs, outputs, nFrames);
            break;
        case 1:
            mFaustTriangle.SetParameterValueNormalised(0, freqNorm);
            mFaustTriangle.SetParameterValue("gain", gain);
            mFaustTriangle.ProcessBlock(inputs, outputs, nFrames);
            break;
        case 2:
            mNoise.SetParameter(1, freqNoise);
            mNoise.SetParameter(3, gain);
            mNoise.ProcessBlock(inputs, outputs, nFrames);
            break;
        }
    }

    void SetSampleRateAndBlockSize(double sampleRate, int blockSize)
    {
        mFaustSquare.SetSampleRate(sampleRate);
        mFaustTriangle.SetSampleRate(sampleRate);
        mNoise.SetParameter(0, sampleRate);
        mADSR.SetSampleRate((sample)sampleRate);
    }

    sample mOwnerGain;
    double mOscFreq;
    int mShape = 0;
    NESting& mOwner;
    FAUST_BLOCK(Square, mFaustSquare, DSP_FILE, 1, 1);
    FAUST_BLOCK(Triangle, mFaustTriangle, DSP_FILE, 1, 1);
    APUNoise mNoise;
    ADSREnvelope<sample> mADSR;
};
#endif


NESting::NESting(const InstanceInfo& info)
: Plugin(info, MakeConfig(kNumParams, 1))
{
    GetParam(iParamGain)->InitGain("gain");
    GetParam(iParamFreq)->InitFrequency("freq", 440);
    GetParam(iParamDuty)->InitInt("duty", 2, 0, 4);
    GetParam(iParamShape)->InitInt("shape", 0, 0, 2);
    GetParam(iParamGate)->InitBool("gate", false);
    mGraphics = nullptr;

#if IPLUG_DSP
    mSynth.AddVoice(new MyVoice(*this), 0);
    mTestVoice = new MyVoice(*this);
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
        IRECT keyboardBounds = b.GetFromBottom(200);
        pGraphics->AttachControl(new IVKeyboardControl(keyboardBounds), kCtrlTagKeyboard);
        pGraphics->SetQwertyMidiKeyHandlerFunc([pGraphics](const IMidiMsg& msg) {
            dynamic_cast<IVKeyboardControl*>(pGraphics->GetControlWithTag(kCtrlTagKeyboard))->SetNoteFromMidi(msg.NoteNumber(), msg.StatusMsg() == IMidiMsg::kNoteOn);
        });

        IRECT freqLabelBounds = b.GetFromTLHC(110.f, 120.f);
        pGraphics->AttachControl(new IVLabelControl(freqLabelBounds, ""), kCtrlTagFreqOut);

        mGraphics = pGraphics;
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
        auto voice = dynamic_cast<MyVoice*>(mSynth.GetVoice(i));
        
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
