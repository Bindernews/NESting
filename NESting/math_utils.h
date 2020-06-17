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
