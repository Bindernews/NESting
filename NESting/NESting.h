#pragma once

#include "IPlug_include_in_plug_hdr.h"
#include <heapbuf.h>

#if IPLUG_DSP
#include "NESVoice.h"
#endif

#include "IControls.h"

#ifndef DSP_FILE
#define DSP_FILE ""
#endif

enum EControlTags
{
  kCtrlTagScope = 0,
  kCtrlTagBender,
  kCtrlTagKeyboard,
  kCtrlTagFreqOut,
  kNumCtrlTags
};


enum EParam
{
	iParamGain = 0,
	iParamFreq,
	iParamDuty,
	iParamShape,
	iParamGate,
	kNumParams,
};

using namespace iplug;
using namespace igraphics;

class NESting final : public Plugin
{
public:
  NESting(const InstanceInfo& info);

#if IPLUG_DSP
  void ProcessBlock(sample** inputs, sample** outputs, int nFrames) override;
  void ProcessMidiMsg(const IMidiMsg& msg) override;
  void OnReset() override;
  void OnParamChange(int paramIdx) override;
  void OnIdle() override;
  bool OnMessage(int msgTag, int ctrlTag, int dataSize, const void* pData) override;

private:
	MidiSynth mSynth{ VoiceAllocator::kPolyModePoly, MidiSynth::kDefaultBlockSize };
	WDL_TypedBuf<sample> mInputBuffer;
	IBufferSender<2> mScopeSender;
	NESVoice* mTestVoice;
	bool mGateOn;

	friend class NESVoice;
#endif
};
