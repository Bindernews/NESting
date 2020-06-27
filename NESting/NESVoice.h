#pragma once
#ifdef IPLUG_DSP

#include "MidiSynth.h"
#include "APUNoise.h"
#include "ADSREnvelope.h"
#include "LFOGraph.h"

using namespace iplug;

class NESting;
class Faust_Square;
class Faust_Triangle;

class NESVoice : public SynthVoice
{
public:
    NESVoice(NESting& owner);
    ~NESVoice();

    bool GetBusy() const override;
    void Trigger(double level, bool isRetrigger) override;
    void Release() override;

    /// Inputs: { }
    /// Outputs: { samples (mono) }
    void ProcessSamplesAccumulating(sample** inputs, sample** outputs, int nInputs, int nOutputs, int startIdx, int nFrames) override;
    void SetSampleRateAndBlockSize(double sampleRate, int blockSize) override;

    LFOGraph mGainGraph;
    LFOGraph mPitchGraph;
    LFOGraph mFinePitchGraph;
    LFOGraph mDutyGraph;

private:
    int mShape = 0;
    NESting& mOwner;
    Faust_Square& mFaustSquare;
    Faust_Triangle& mFaustTriangle;
    APUNoise mNoise;
    ADSREnvelope<sample> mADSR;
    float mSustain;

    WDL_TypedBuf<sample> mGainBuf;
    WDL_TypedBuf<sample> mPitchBuf;
    WDL_TypedBuf<sample> mFinePitchBuf;
    WDL_TypedBuf<sample> mDutyBuf;
    WDL_TypedBuf<sample> mOutBuf;

	friend class NESting;
};

#endif