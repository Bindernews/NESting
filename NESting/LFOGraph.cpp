#include "LFOGraph.h"
#include "math_utils.h"
#include <algorithm>

LFOGraph::LFOGraph(int maxSteps, float defaultValue)
	: mSampleIdx(0), mSampleRate(48000), mNumSteps(1), mLoopPoint(0), mStepSamples(1)
{
	mValues.resize(maxSteps, defaultValue);
}

void LFOGraph::Trigger(double level, bool isRetrigger)
{
	mSampleIdx = 0;
}

void LFOGraph::Release()
{
}

void LFOGraph::SetSampleRateAndBlockSize(double sampleRate, int blockSize)
{
	mSampleRate = int(sampleRate);
}

void LFOGraph::ProcessBlock(sample* output, int nFrames)
{
	// We need to know how many loop points there are
	int loopRange = mNumSteps - mLoopPoint;
	SIMD_FOR_LOOP
	for (int i = 0; i < nFrames; i++) {
		int stepIdx = (mSampleIdx + i) / mStepSamples;
		int loopedStep = (stepIdx % loopRange) + mLoopPoint;
		// If stepIdx is less than the loop point, it'll be stepIdx, else it'll be loopedStep
		stepIdx = std::min(stepIdx, loopedStep);
		output[i] = mValues[stepIdx];
	}
	mSampleIdx += nFrames;
}

int LFOGraph::GetStepSamples() const
{
	return mStepSamples;
}

void LFOGraph::SetStepSamples(int v)
{
	mStepSamples = std::max(v, 1);
}

bn::slice<float> LFOGraph::GetValues() const
{
	return bn::slice(mValues.data(), mValues.size());
}

void LFOGraph::SetValues(bn::slice<float> values)
{
	for (int i = 0; i < values.len(); i++) {
		mValues[i] = values[i];
	}
}
