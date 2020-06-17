/*
 * This file specifies a DSP to generate noise, similar to the NES 2A03 sound chip.
 */
#pragma once
#include <cstdint>
#include <cmath>
#include <algorithm>

typedef float FLOAT32;


static float unlerp(float low, float high, float x) { return (x - low) / (high - low); }
static float lerp(float low, float high, float x) { return (x * (high - low)) + low; }
static float clampf(float low, float high, float x) {
    if (x < low) { return low; }
    if (x > high) { return high; }
    return x;
}

class APUNoise {
public:
    APUNoise() {
        reset();
    }

    static const int TIMER_PERIOD_TABLE = { 4, 8, 16, 32, 64, 96, 128, 160, 202, 254, 380, 508, 762, 1016, 2034, 4068 };

    void setParameter(int paramId, float value) {
        switch (paramId) {
        case 0:
            sampleRate = int(value);
            sampleCount = 0;
            updateTimerPeriod();
            break;
        case 1:
            timerPeriodIndex = int(clampf(lerp(0.f, 15.f, value), 0.f, 15.f));
            sampleCount = 0;
            updateTimerPeriod();
            break;
        case 2:
            timerMode = std::roundf(value);
            sampleCount = 0;
            break;
        }
    }

    float getParameter(int paramId) {
        switch (paramId) {
        case 0: return float(sampleRate);
        case 1: return unlerp(0.f, 15.f, float(timerPeriodIndex));
        case 2: return float(timerMode);
        default: return 0.f;
        }
    }

    void reset() {
        sampleRate = 48000;
        timerPeriodIndex = 0;
        timerMode = 1;
        updateTimerPeriod();

        sampleCount = 0;
        shiftReg = 0x1;
    }

    void compute(int count, FLOAT32** inputs, FLOAT32** outputs) {
        FLOAT32* input0 = inputs[0];
        FLOAT32* output0 = outputs[0];

        for (int i = 0; i < count; i += 1) {
            // Output the next bit of noise
            output0[i] = FLOAT32(shiftReg & 0x1);
            // Update our sample count which acts as the timer
            sampleCount = (sampleCount + 1) % timerPeriod;
            // If our timer ticked to 0, perform the register shift.
            if (sampleCount == 0) {
                doShiftRegister();
            }
        }
    }

private:

    void updateTimerPeriod() {
        timerPeriod = (sampleRate / 60) * TIMER_PERIOD_TABLE[timerPeriodIndex];
    }

    void doShiftRegister() {
        uint16_t feedback = shiftReg ^ (timerMode ? (shiftReg >> 6) : (shiftReg >> 1));
        feedback = (feedback << 14) & 0x4000;
        shiftReg = ((shiftReg >> 1) | feedback) & 0x7FFF;
    }

private:
    /// How many samples we've currently processed (aka. current timer value)
    int sampleCount;
    /// Current state of the shift register
    uint16_t shiftReg;
    /// Number of samples per timer tick (60 Hz * TIMER_PERIOD_TABLE value)
    int timerPeriod;

    //*******************
    // Params
    //*******************

    int sampleRate;
    int timerPeriodIndex;
    bool timerMode;
};

