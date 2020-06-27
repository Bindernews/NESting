#include "FaustSquare.hpp"
#include "FaustTriangle.hpp"
#include "IPlugFaustGen.h"

#include "NESVoice.h"
#include "NESting.h"
#include "math_utils.h"
#include "constants.h"

#if IPLUG_DSP
NESVoice::NESVoice(NESting& owner) 
    : mOwner(owner), mADSR("gain", [&]() { mNoise.OnRelease(); }),
      mFaustSquare(*new Faust_Square("Square", "", 1, 1)),
      mFaustTriangle(*new Faust_Triangle("Triangle", "", 1, 1)),
      mGainGraph(MAX_LFO_GRAPH_STEPS, DEFAULT_ENV_VOLUME),
      mPitchGraph(MAX_LFO_GRAPH_STEPS, DEFAULT_ENV_PITCH),
      mDutyGraph(MAX_LFO_GRAPH_STEPS, DEFAULT_ENV_DUTY),
      mFinePitchGraph(MAX_LFO_GRAPH_STEPS, DEFAULT_ENV_FINE_PITCH)
{
  mSustain = 1.0f;

  mFaustTriangle.SetMaxChannelCount(mOwner.MaxNChannels(ERoute::kInput), mOwner.MaxNChannels(ERoute::kOutput));
  mFaustTriangle.Init();

  mFaustSquare.SetMaxChannelCount(mOwner.MaxNChannels(ERoute::kInput), mOwner.MaxNChannels(ERoute::kOutput));
  mFaustSquare.Init();

  mNoise.Reset();
}

NESVoice::~NESVoice()
{
  delete& mFaustSquare;
  delete& mFaustTriangle;
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

    mGainGraph.Trigger(level, isRetrigger);
    mPitchGraph.Trigger(level, isRetrigger);
    mDutyGraph.Trigger(level, isRetrigger);
}

void NESVoice::Release()
{
    mADSR.Release();

    mGainGraph.Release();
    mPitchGraph.Release();
    mDutyGraph.Release();
}

void NESVoice::ProcessSamplesAccumulating(sample** inputs, sample** outputs, int nInputs, int nOutputs, int startIdx, int nFrames)
{
    double pitch = mInputs[kVoiceControlPitch].endValue;
    double pitchBend = mInputs[kVoiceControlPitchBend].endValue;

    // First we generate the LFO buffer values using data from mOwner.
    bool useLFOGraphs = mOwner.GetParam(iParamUseAutomationGraphs)->Bool();
    if (useLFOGraphs) {
        mGainGraph.ProcessBlock(mGainBuf.Get(), nFrames);
        mPitchGraph.ProcessBlock(mPitchBuf.Get(), nFrames);
        mDutyGraph.ProcessBlock(mDutyBuf.Get(), nFrames);
    }
    else {
        set_buffer(mGainBuf.Get(), nFrames, sample(mOwner.GetParam(iParamVolumeEnvelope)->GetNormalized()));
        double pitchVal = mOwner.GetParam(iParamPitchEnvelope)->Value() 
            + mOwner.GetParam(iParamFineEnvelope)->Value();
        set_buffer(mPitchBuf.Get(), nFrames, sample(pitchVal));
        set_buffer(mDutyBuf.Get(), nFrames, sample(mOwner.GetParam(iParamDutyEnvelope)->GetNormalized()));
    }

    // Now update our buffers with values from the ADSR envelope and MIDI data
    sample gain = sample(mADSR.Process(mSustain) * mGain);
    sample* gainBuf = mGainBuf.Get();
    double pitchOffset = pitch + pitchBend;
    sample* pitchBuf = mPitchBuf.Get();
    PARALLEL_FOR_LOOP
    for (int i = 0; i < nFrames; i++) {
        gainBuf[i] = gainBuf[i] * gain;
        pitchBuf[i] = sample(PitchToHz(pitchBuf[i] + pitchOffset));
    }

    sample* inputs2[] = { gainBuf, pitchBuf, mDutyBuf.Get() };
    sample* outputs2[] = { outputs[0] + startIdx, outputs[1] + startIdx };

    switch (mShape) {
    case 0:
        mFaustSquare.ProcessBlock(inputs2, outputs2, nFrames);
        break;
    case 1:
        mFaustTriangle.ProcessBlock(inputs2, outputs2, nFrames);
        break;
    case 2:
        mNoise.ProcessBlock(inputs2, outputs2, nFrames);
        break;
    }
}

void NESVoice::SetSampleRateAndBlockSize(double sampleRate, int blockSize)
{
    mFaustSquare.SetSampleRate(sampleRate);
    mFaustTriangle.SetSampleRate(sampleRate);
    mNoise.SetParameter(0, sampleRate);
    mADSR.SetSampleRate((sample)sampleRate);

    mGainGraph.SetSampleRateAndBlockSize(sampleRate, blockSize);
    mPitchGraph.SetSampleRateAndBlockSize(sampleRate, blockSize);
    mDutyGraph.SetSampleRateAndBlockSize(sampleRate, blockSize);
    mGainBuf.Resize((int)blockSize);
    mPitchBuf.Resize((int)blockSize);
    mDutyBuf.Resize(int(blockSize));
}

#endif

