
#include "APUNoise.h"
#include "IPlugLogger.h"
#include "math_utils.h"

using iplug::sample;

#define TIMER_PERIOD_SIZE (16)
//static const int TIMER_PERIOD_TABLE[] = { 4, 8, 16, 32, 64, 96, 128, 160, 202, 254, 380, 508, 762, 1016, 2034, 4068 };
static const int TIMER_PERIOD_TABLE[] = { 4068, 2034, 1016, 762, 508, 380, 254, 202, 160, 128, 96, 64, 32, 16, 8, 4 };

APUNoise::APUNoise()
{
	Reset();
}

APUNoise::~APUNoise()
{}

void APUNoise::SetParameter(int paramId, double value)
{
    switch (paramId) {
    // Sample rate
    case 0:
        sampleRate = int(value);
        sampleCount = 0;
        updateTimerPeriod();
        break;
    // Timer mode (0 - 1)
    case 2:
        timerMode = bool(std::roundf(float(value)));
        break;
    }
}

double APUNoise::GetParameter(int paramId)
{
    switch (paramId) {
    case 0: return double(sampleRate);
    case 2: return double(timerMode);
    default: return 0.f;
    }
}

void APUNoise::Reset() {
    sampleRate = 48000;
    timerMode = 0;
    sampleCount = 0;
    updateTimerPeriod();
}

void APUNoise::OnRelease()
{
    sampleCount = 0;
    for (int i = 0; i < TIMER_PERIOD_SIZE; i++) {
        shiftReg = 0x1;
    }
}

void APUNoise::ProcessBlock(sample** inputs, sample** outputs, int nFrames) {
    sample* gain = inputs[0];
    sample* pitch = inputs[1];
    sample* output0 = outputs[0];

    for (int i = 0; i < nFrames; i += 1) {
        // Determine the timer index to use
        int timerIndex = int(HzToMidi(pitch[i])) % 16;
        // Output the next bit of noise
        output0[i] = sample(double( (shiftReg >> 0) & 0x1 ) * gain[i]);
        // Update our sample count which acts as the timer
        sampleCount += 1;
        // If our timer ticked to 0, perform the register shift.
        if (sampleCount >= timerPeriod[timerIndex]) {
            doShiftRegister();
            sampleCount = 0;
        }
    }
}

void APUNoise::updateTimerPeriod() {
    for (int i = 0; i < TIMER_PERIOD_SIZE; i++) {
        float nesApuSpeed = float(sampleRate) / 44750.f;
        timerPeriod[i] = std::max(int(nesApuSpeed * float(TIMER_PERIOD_TABLE[i])), 1);
    }
    // Also reset shiftReg
    shiftReg = 0x1;
}

void APUNoise::doShiftRegister() {
    uint16_t feedback = shiftReg;
    if (timerMode) {
        feedback ^= (shiftReg >> 6);
    }
    else {
        feedback ^= (shiftReg >> 1);
    }
    feedback = (feedback << 14) & 0x4000;
    shiftReg = (shiftReg >> 1) & 0x3FFF;
    shiftReg = shiftReg | feedback;
}
