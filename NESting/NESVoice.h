#pragma once
#ifdef IPLUG_DSP

#include "MidiSynth.h"
#include "APUNoise.h"
#include "ADSREnvelope.h"
#include "FaustSquare.hpp"
#include "FaustTriangle.hpp"
#include "IPlugFaustGen.h"

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
    void ProcessSamplesAccumulating(sample** inputs, sample** outputs, int nInputs, int nOutputs, int startIdx, int nFrames) override;
    void SetSampleRateAndBlockSize(double sampleRate, int blockSize) override;

    void ProcessSamples(double oscFreq, double gain, sample** inputs, sample** outputs, int nFrames);

private:
    sample mOwnerGain;
    int mShape = 0;
    NESting& mOwner;
    FAUST_BLOCK(Square, mFaustSquare, "", 1, 1);
    FAUST_BLOCK(Triangle, mFaustTriangle, "", 1, 1);
    APUNoise mNoise;
    ADSREnvelope<sample> mADSR;

	friend class NESting;
};

#endif