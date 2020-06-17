#pragma once

#include "IPlug_include_in_plug_hdr.h"
#include <heapbuf.h>

#if IPLUG_DSP
#include "IPlugFaustGen.h"
#endif

#include "IControls.h"

#ifndef DSP_FILE
#define DSP_FILE ""
#endif

enum EControlTags
{
  kCtrlTagScope = 0,
  kNumCtrlTags
};

const int kNumParams = 5;

using namespace iplug;
using namespace igraphics;

class NESting final : public Plugin
{
public:
  NESting(const InstanceInfo& info);

#if IPLUG_DSP
  void ProcessBlock(sample** inputs, sample** outputs, int nFrames) override;
  void OnReset() override;
  void OnParamChange(int paramIdx) override;
  void OnIdle() override;
private:
  FAUST_BLOCK(Faust1, mFaustProcessor, DSP_FILE, 1, 1);
  WDL_TypedBuf<sample> mInputBuffer;
  IBufferSender<2> mScopeSender;
#endif
};
