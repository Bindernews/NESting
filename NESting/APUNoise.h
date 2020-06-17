/*
 * This file specifies a DSP to generate noise, similar to the NES 2A03 sound chip.
 */
#pragma once
#include <cstdint>
#include <cmath>
#include <algorithm>
#include "IPlugConstants.h"


static inline float unlerp(float low, float high, float x) { return (x - low) / (high - low); }
static inline float lerp(float low, float high, float x) { return (x * (high - low)) + low; }
static inline float clampf(float low, float high, float x) {
    if (x < low) { return low; }
    if (x > high) { return high; }
    return x;
}

class APUNoise {
public:
    APUNoise();
    ~APUNoise();

    void SetParameter(int paramId, double value);
    double GetParameter(int paramId);
    void Reset();
    void Compute(iplug::sample** inputs, iplug::sample** outputs, int nFrames);

private:
    void updateTimerPeriod();
    void doShiftRegister();

private:
    /// Our location in the 15-bit LFSR
    int lfsrIndex;
    /// How many samples we've currently processed (aka. current timer value)
    int sampleCount;
    /// Current state of the shift register
    uint16_t shiftReg;
    /// Number of samples per timer tick (60 Hz * TIMER_PERIOD_TABLE value)
    int timerPeriod;
    /// Gain (0 - 1) of the noise.
    double gain;

    //*******************
    // Params
    //*******************

    int sampleRate;
    int timerPeriodIndex;
    bool timerMode;
};

