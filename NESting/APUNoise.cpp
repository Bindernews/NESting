
#include "APUNoise.h"
#include "IPlugLogger.h"
#include "math_utils.h"

using iplug::sample;

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
        shiftReg = 0x1;
        lfsrIndex = 0;
        updateTimerPeriod();
        break;
    // Timer period
    case 1:
        timerPeriodIndex = int(clampf(0., 15., lerp(0., 15., value)));
        updateTimerPeriod();
        break;
    // Timer mode (0 - 1)
    case 2:
        timerMode = std::roundf(float(value));
        sampleCount = 0;
        lfsrIndex = 0;
        break;
    case 3:
        gain = value;
        break;
    }
}

double APUNoise::GetParameter(int paramId)
{
    switch (paramId) {
    case 0: return double(sampleRate);
    case 1: return unlerp(0.f, 15.f, float(timerPeriodIndex));
    case 2: return double(timerMode);
    case 3: return gain;
    default: return 0.f;
    }
}

void APUNoise::Reset() {
    sampleRate = 48000;
    timerPeriodIndex = 0;
    timerMode = 0;
    lfsrIndex = 0;
    updateTimerPeriod();

    sampleCount = 0;
    shiftReg = 0x1;
}

void APUNoise::OnRelease()
{
    lfsrIndex = 0;
    shiftReg = 0x1;
    sampleCount = 0;
}

void APUNoise::ProcessBlock(sample** inputs, sample** outputs, int nFrames) {
    sample* input0 = inputs[0];
    sample* output0 = outputs[0];

    for (int i = 0; i < nFrames; i += 1) {
        // Output the next bit of noise
        output0[i] = sample(double( (shiftReg >> 0) & 0x1 ) * gain);
        // Update the lfsrIndex
        lfsrIndex = (lfsrIndex + 1) % 15;
        // Update our sample count which acts as the timer
        sampleCount += 1;
        
        // If our timer ticked to 0, perform the register shift.
        if (sampleCount >= timerPeriod) {
            sampleCount %= timerPeriod;
            doShiftRegister();
        }
    }
}

void APUNoise::updateTimerPeriod() {
    float nesApuSpeed = float(sampleRate) / 44750.f;
    timerPeriod = int(nesApuSpeed * float(TIMER_PERIOD_TABLE[timerPeriodIndex]));
    if (timerPeriod < 1) {
        timerPeriod = 1;
    }
    // timerPeriod = TIMER_PERIOD_TABLE[timerPeriodIndex];
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
