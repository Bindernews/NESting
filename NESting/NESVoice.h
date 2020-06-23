#pragma once
#ifdef IPLUG_DSP

#include "MidiSynth.h"
#include "APUNoise.h"
#include "ADSREnvelope.h"
#include "FaustSquare.hpp"
#include "FaustTriangle.hpp"
#include "IPlugFaustGen.h"
#include "LFOGraph.h"

using namespace iplug;

class NESting;

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

private:
    int mShape = 0;
    NESting& mOwner;
    FAUST_BLOCK(Square, mFaustSquare, "", 1, 1);
    FAUST_BLOCK(Triangle, mFaustTriangle, "", 1, 1);
    APUNoise mNoise;
    ADSREnvelope<sample> mADSR;
    float mSustain;

    LFOGraph mGainGraph;
    LFOGraph mPitchGraph;
    LFOGraph mFinePitchGraph;
    LFOGraph mDutyGraph;

    WDL_TypedBuf<sample> mGainBuf;
    WDL_TypedBuf<sample> mPitchBuf;
    WDL_TypedBuf<sample> mFinePitchBuf;
    WDL_TypedBuf<sample> mDutyBuf;

	friend class NESting;
};

#endif