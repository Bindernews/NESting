#pragma once
#include <cmath>

#ifndef NO_OPENMP
#define PARALLEL_FOR_LOOP __pragma(omp parallel for)
//#define SIMD_FOR_LOOP __pragma(omp declare simd)
#define SIMD_FOR_LOOP
#else
#define PARALLEL_FOR_LOOP
#define SIMD_FOR_LOOP
#endif

template<typename T>
static inline T unlerp(T low, T high, T x) { return (x - low) / (high - low); }

template<typename T>
static inline T lerp(T low, T high, T x) { return (x * (high - low)) + low; }

template<typename T>
static inline T clampf(T low, T high, T x)
{
    if (x < low) { return low; }
    if (x > high) { return high; }
    return x;
}

inline static double make_stepped(double value, double step)
{
    return std::round(value / step) * step;
}

/** @brief Convert a normalized double into an integer such that all integer values are (mostly) equally represented.
 * @param low The lowest possible value
 * @param high The highest possible value PLUS ONE
 * @param norm A double in the range [0, 1]
 * @return An integer in the range [low, high)
 */
inline static int NormToInt(int low, int high, double norm)
{
    int diff = high - low;
    double mul = double(diff) - 0.001;
    return int(norm * mul) + low;
}

inline static double TimeToSamples(double seconds, double sampleRate)
{
    return seconds * double(sampleRate);
}

inline static double PitchToHz(double pitch)
{
    return 440. * pow(2., pitch);
}

inline static double HzToMidi(double hz)
{
    return 69 + (12 * log2(hz / 440.));
}

inline static double BeatToSamples(double sampleRate, double bpm)
{
    return (60. / bpm) * sampleRate;
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

template<typename T>
void set_buffer(T* buffer, int size, T value)
{
    SIMD_FOR_LOOP
    for (int i = 0; i < size; i++) {
        buffer[i] = value;
    }
}
