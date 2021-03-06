#pragma once

#include "IPlug_include_in_plug_hdr.h"
#include <heapbuf.h>
#include <atomic>
#include <vector>
#include <memory>

#if IPLUG_DSP
#include "NESVoice.h"
#endif
#if IPLUG_EDITOR
#include <IControls.h>
#endif

#ifndef DSP_FILE
#define DSP_FILE ""
#endif

const int kNumPresets = 3;

enum EControlTags
{
  kCtrlTagScope = 0,
  kCtrlTagBender,
  kCtrlTagKeyboard,
  kCtrlTagFreqOut,
  kCtrlBackground,
  kCtrlAutomationGraphs,
  kCtrlStatus,
  kCtrlVolumeGraph,
  kCtrlDutyGraph = kCtrlVolumeGraph + 6,
  kCtrlPitchGraph = kCtrlDutyGraph + 6,
  kCtrlFineGraph = kCtrlPitchGraph + 6,
  kNumCtrlTags
};


enum EParam
{
  iParamShape = 0,
  iParamNoiseMode,
  iParamAttack,
  iParamDecay,
  iParamSustain,
  iParamRelease,
  iParamUseAutomationGraphs,

  iParamVolumeSteps,
  iParamVolumeLoopStart,
  iParamVolumeLoopEnd,
  iParamVolumeTime,
  iParamVolumeTempoSync,
  iParamVolumeEnvelope,

  iParamDutySteps,
  iParamDutyLoopStart,
  iParamDutyLoopEnd,
  iParamDutyTime,
  iParamDutyTempoSync,
  iParamDutyEnvelope,

  iParamPitchSteps,
  iParamPitchLoopStart,
  iParamPitchLoopEnd,
  iParamPitchTime,
  iParamPitchTempoSync,
  iParamPitchEnvelope,

  iParamFineSteps,
  iParamFineLoopStart,
  iParamFineLoopEnd,
  iParamFineTime,
  iParamFineTempoSync,
  iParamFineEnvelope,

  kNumParams,
};

using namespace iplug;
using namespace igraphics;

struct QMsg
{
  QMsg() : QMsg(0, nullptr, 0) {}
  QMsg(int kind) : QMsg(kind, nullptr, 0) {}
  QMsg(int kind, const char* msg) : QMsg(kind, (void*)msg, strlen(msg) + 1) {}

  QMsg(int kind, void* data, int dataSize)
  {
    this->kind = kind;
    mSize = dataSize;
    if (data != nullptr) {
      mData = new uint8_t[size_t(dataSize) + 1];
      memcpy(mData, data, dataSize);
    }
  }
  
  template<typename T>
  inline T* data_as() const { return reinterpret_cast<T*>(mData); }
  inline void* data() const { return mData; }
  inline int size() const { return mSize; }

  /** Release the resources associated with this QMsg. THIS MUST BE CALLED.
   * QMsg does NOT release resources by itself. */
  void done()
  {
    if (mData != nullptr)
    {
      delete[] mData;
      mData = nullptr;
      mSize = 0;
    }
  }

  int kind;

private:
  uint8_t* mData;
  int mSize;
};

class NESting final : public Plugin
{
public:
  NESting(const InstanceInfo& info);


private:
  void initParams();
  inline static int calcLoopPoint(double value, int numSteps)
  {
    return int(value * (double(numSteps) - 0.01));
  }

#if IPLUG_EDITOR
public:
  void UpdateUISize();
  void OnParamChangeUI(int paramIdx, const EParamSource source) override;
  void OnRestoreState() override;

private:
  /// Construct the UI elements with their correct, default positions.
  void buildUI(IGraphics *ui);
  /// Adjust the position of UI elements as required.
  void layoutUI(IGraphics* ui);
  /// Returns the desired height of the GUI. This changes based on some parameters.
  float getDesiredHeight();
  /// Update the graph UI from the DSP data
  void updateGraphUI();
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
  bool SerializeState(IByteChunk& chunk) const override;
  int UnserializeState(const IByteChunk& chunk, int startPos) override;

private:
  /** Call a function on each voice in our synth. */
  void forEachVoice(std::function<void(NESVoice*)> cb);
  /** Initalize the presets */
  void initPresets();
  /** Helper function that returns our first NESVoice object. Useful for saving state. */
  NESVoice* GetVoice() const;

  MidiSynth mSynth{ VoiceAllocator::kPolyModePoly, MidiSynth::kDefaultBlockSize };
  WDL_TypedBuf<sample> mInputBuffer;
  IBufferSender<2> mScopeSender;
  IPlugQueue<QMsg> mUIQueue;

  friend class NESVoice;
#endif // IPLUG_DSP
};
