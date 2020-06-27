#pragma once

#include <IPlugConstants.h>
#include <slice.hpp>
#include <heapbuf.h>

using iplug::sample;

class LFOGraph
{
public:

	enum EState {
		kSAttack,
		kSSustain,
		kSRelease,
		kSDone,
	};

	LFOGraph(int maxSteps, float defaultValue);

	void Trigger(double level, bool isRetrigger);
	void Release();
	void SetSampleRateAndBlockSize(double sampleRate, int blockSize);
	void ProcessBlock(sample* output, int nFrames);
	bool GetBusy() const;

	int GetStepSamples() const;
	void SetStepSamples(int v);

	void SetValues(bn::slice<float> values);
	bn::slice<float> GetValues() const;

	inline void SetRange(float low, float high) { mLow = low; mHigh = high; }

	/// The default value for mValues
	float mDefaultValue;
	/// A value that won't change the output signal. Used for processing when in kSDone state.
	float mZeroValue;
	float mLow;
	float mHigh;
	int mSampleIdx;
	int mSampleRate;
	int mNumSteps;
	int mLoopStart;
	int mLoopEnd;

private:
	int mState;
	int mStepSamples;
	WDL_TypedBuf<float> mValues;
};