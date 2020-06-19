#include "NESVoice.h"
#include "NESting.h"
#include "math_utils.h"

#if IPLUG_DSP
NESVoice::NESVoice(NESting& owner) : mOwner(owner), mADSR("gain", [&]() { mNoise.OnRelease(); })
{
    mFaustTriangle.SetMaxChannelCount(mOwner.MaxNChannels(ERoute::kInput), mOwner.MaxNChannels(ERoute::kOutput));
    mFaustTriangle.Init();

    mFaustSquare.SetMaxChannelCount(mOwner.MaxNChannels(ERoute::kInput), mOwner.MaxNChannels(ERoute::kOutput));
    mFaustSquare.Init();

    mNoise.Reset();
    mOwnerGain = 0.5;
}

NESVoice::~NESVoice()
{
}

bool NESVoice::GetBusy() const
{
    return mADSR.GetBusy();
}

void NESVoice::Trigger(double level, bool isRetrigger)
{
    if (isRetrigger) {
        mADSR.Retrigger(sample(level));
    }
    else {
        mADSR.Start(sample(level));
    }
}

void NESVoice::Release()
{
    mADSR.Release();
}

void NESVoice::ProcessSamplesAccumulating(sample** inputs, sample** outputs, int nInputs, int nOutputs, int startIdx, int nFrames)
{
    double pitch = mInputs[kVoiceControlPitch].endValue;
    double pitchBend = mInputs[kVoiceControlPitchBend].endValue;

    // Convert midi pitch into actual frequency
    double oscFreq = 440. * pow(2., pitch + pitchBend);

    double gain = mADSR.Process(1) * mGain * mOwnerGain;

    sample* outputs2[2];
    sample_offset(outputs, outputs2, 2, startIdx);

    ProcessSamples(oscFreq, gain, inputs, outputs2, nFrames);
}

void NESVoice::ProcessSamples(double oscFreq, double gain, sample** inputs, sample** outputs, int nFrames)
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

void NESVoice::SetSampleRateAndBlockSize(double sampleRate, int blockSize)
{
    mFaustSquare.SetSampleRate(sampleRate);
    mFaustTriangle.SetSampleRate(sampleRate);
    mNoise.SetParameter(0, sampleRate);
    mADSR.SetSampleRate((sample)sampleRate);
}

#endif

