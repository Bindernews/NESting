#pragma once

template<typename T>
static inline T unlerp(T low, T high, T x) { return (x - low) / (high - low); }

template<typename T>
static inline T lerp(T low, T high, T x) { return (x * (high - low)) + low; }

template<typename T>
static inline T clampf(T low, T high, T x) {
    if (x < low) { return low; }
    if (x > high) { return high; }
    return x;
}

template<typename T>
void sample_offset(T** inputs, T** outputs, int nInputs, int startIdx)
{
    for (int i = 0; i < nInputs; i++) {
        outputs[i] = &inputs[i][startIdx];
    }
}

template<typename T>
void zero_buffers(T** buffers, int nBuffers, int nFrames)
{
    for (int i = 0; i < nBuffers; i += 1) {
        memset(buffers[i], 0, sizeof(T) * nFrames);
    }
}
