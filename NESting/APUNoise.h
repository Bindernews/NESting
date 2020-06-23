/*
 * This file specifies a DSP to generate noise, similar to the NES 2A03 sound chip.
 */
#pragma once
#include <cstdint>
#include <cmath>
#include <algorithm>
#include "IPlugConstants.h"

class APUNoise {
public:
    APUNoise();
    ~APUNoise();

    void SetParameter(int paramId, double value);
    double GetParameter(int paramId);
    void Reset();
    void OnRelease();

    /// Process a block of samples.
    /// Inputs: { gain (norm), pitch (Hz) }
    /// Outputs: { samples }
    void ProcessBlock(iplug::sample** inputs, iplug::sample** outputs, int nFrames);

private:
    void updateTimerPeriod();
    void doShiftRegister();

private:
    /// How many samples we've currently processed (aka. current timer value).
    /// This would overflow if you ran for about 3 million years at a 48 kHz sample rate.
    uint64_t sampleCount;
    /// Current state of the shift register for each timer
    uint16_t shiftReg;
    /// Number of samples per timer tick (60 Hz * TIMER_PERIOD_TABLE value)
    int timerPeriod[16];

    //*******************
    // Params
    //*******************
    int sampleRate;
    bool timerMode;
};

