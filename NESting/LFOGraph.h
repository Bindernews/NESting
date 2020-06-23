#pragma once

#include <IPlugConstants.h>
#include <vector>
#include <slice.hpp>

using iplug::sample;

class LFOGraph
{
public:
	LFOGraph(int maxSteps, float defaultValue);

	void Trigger(double level, bool isRetrigger);
	void Release();
	void SetSampleRateAndBlockSize(double sampleRate, int blockSize);
	void ProcessBlock(sample* output, int nFrames);

	int GetStepSamples() const;
	void SetStepSamples(int v);

	void SetValues(bn::slice<float> values);
	bn::slice<float> GetValues() const;

	int mSampleIdx;
	int mSampleRate;
	int mNumSteps;
	int mLoopPoint;

private:
	int mStepSamples;
	std::vector<float> mValues;
};