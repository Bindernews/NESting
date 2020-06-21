#pragma once

#include "IPlug_include_in_plug_hdr.h"
#include <heapbuf.h>
#include <atomic>

#if IPLUG_DSP
#include "NESVoice.h"
#endif
#if IPLUG_EDITOR
#include <IControls.h>
#endif

#ifndef DSP_FILE
#define DSP_FILE ""
#endif

#define AUTOMATION_PANEL_HEIGHT (200)

enum EControlTags
{
  kCtrlTagScope = 0,
  kCtrlTagBender,
  kCtrlTagKeyboard,
  kCtrlTagFreqOut,
  kCtrlAutomationGraphs,
  kCtrlStatus,
  kNumCtrlTags
};


enum EParam
{
	iParamShape = 0,
	iParamGain,
	iParamDuty,
	iParamNoiseMode,
	iParamUseAutomationGraphs,

	iParamVolumeSteps,
	iParamVolumeLoopPoint,
	iParamVolumeTime,
	iParamVolumeTempoSync,

	iParamDutySteps,
	iParamDutyLoopPoint,
	iParamDutyTime,
	iParamDutyTempoSync,

	iParamPitchSteps,
	iParamPitchLoopPoint,
	iParamPitchTime,
	iParamPitchTempoSync,

	kNumParams,
};

using namespace iplug;
using namespace igraphics;

class NESting final : public Plugin
{
public:
  NESting(const InstanceInfo& info);

#if IPLUG_EDITOR
public:
	void ShowAutomationGraphs(bool visible);
#endif

#if IPLUG_DSP
public:
  void ProcessBlock(sample** inputs, sample** outputs, int nFrames) override;
  void ProcessMidiMsg(const IMidiMsg& msg) override;
  void OnReset() override;
  void OnParamChange(int paramIdx) override;
  void OnIdle() override;
  bool OnMessage(int msgTag, int ctrlTag, int dataSize, const void* pData) override;
  void SendControlMsgFromDelegate(int ctrlTag, int msgTag, int dataSize, const void* pData) override;

private:
	MidiSynth mSynth{ VoiceAllocator::kPolyModePoly, MidiSynth::kDefaultBlockSize };
	WDL_TypedBuf<sample> mInputBuffer;
	IBufferSender<2> mScopeSender;
	ISender<4> mSender;

	friend class NESVoice;
#endif
};
