#include "LFOGraph.h"
#include "math_utils.h"
#include <algorithm>

LFOGraph::LFOGraph(int maxSteps, float defaultValue)
	: mSampleIdx(0), mSampleRate(48000), mNumSteps(1), mLoopStart(0), mLoopEnd(0), mStepSamples(1), mState(kSDone)
{
	mValues.Resize(maxSteps);
	mDefaultValue = defaultValue;
	mZeroValue = defaultValue;
	set_buffer<float>(mValues.Get(), maxSteps, defaultValue);
	mLow = 0.0f;
	mHigh = 1.0f;
}

void LFOGraph::Trigger(double level, bool isRetrigger)
{
	mSampleIdx = 0;
	mState = kSAttack;
}

void LFOGraph::Release()
{
	// If we're currently sustaining, fix mSampleIdx
	if (mState == kSSustain) {
		mSampleIdx += (mLoopStart * mStepSamples);
	}
	mState = kSRelease;
}

void LFOGraph::SetSampleRateAndBlockSize(double sampleRate, int blockSize)
{
	mSampleRate = int(sampleRate);
}

bool LFOGraph::GetBusy() const
{
	return mState != kSDone;
}

void LFOGraph::ProcessBlock(sample* output, int nFrames)
{
	if (mState == kSAttack) {
		// TODO simd
		for (int i = 0; i < nFrames; i++) {
			int stepIdx = (mSampleIdx + i) / mStepSamples;
			output[i] = mValues.GetFast()[stepIdx];
		}
		// Update our sample index and check if we should move to sustain state
		mSampleIdx += nFrames;
		int startSample = std::max(mStepSamples * mLoopStart, 1);
		if (mSampleIdx >= startSample) {
			mState = kSSustain;
			mSampleIdx %= startSample;
		}
	}
	else if (mState == kSSustain) {
		// In here, mSampleIdx is relative to the start of mLoopStart
		// This allows us to correctly release.
		int loopSamples = (mLoopEnd + 1 - mLoopStart) * mStepSamples;
		// TODO simd
		for (int i = 0; i < nFrames; i++) {
			int sampleIdx = (mSampleIdx + i) % loopSamples;
			int stepIdx = mLoopStart + (sampleIdx / mStepSamples);
			output[i] = mValues.GetFast()[stepIdx];
		}
		mSampleIdx = (mSampleIdx + nFrames) % loopSamples;
	}
	else if (mState == kSRelease) {
		// Here mSampleIdx is once more relative to the start of the graph.

		// TODO simd
		for (int i = 0; i < nFrames; i++) {
			int stepIdx = std::min((mSampleIdx + i) / mStepSamples, mNumSteps - 1);
			output[i] = mValues.GetFast()[stepIdx];
		}
		// Update our sample index and check if our release phase is finished.
		mSampleIdx += nFrames;
		if (mSampleIdx >= (mNumSteps * mStepSamples)) {
			mState = kSDone;
		}
	}
	else {
		// Default, set value to default
		set_buffer(output, nFrames, mZeroValue);
	}

	// Denormalize our values
	for (int i = 0; i < nFrames; i++) {
		output[i] = lerp(mLow, mHigh, output[i]);
	}
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
	return bn::slice(mValues.Get(), mValues.GetSize());
}

void LFOGraph::SetValues(bn::slice<float> values)
{
	for (int i = 0; i < values.len(); i++) {
		mValues.Get()[i] = values[i];
	}
}
