#include "NESting.h"
#include "constants.h"

enum StateVersion {
  kStateV0 = 0x00000000,
};

#define CURRENT_VERSION kStateV0

bool NESting::SerializeState(IByteChunk& chunk) const
{
  bool saveOk = true;

  // First write the version info
  uint32_t versionData = CURRENT_VERSION;
  saveOk &= (chunk.Put(&versionData) > 0);

  // Now serialize the plugin state
  saveOk &= Plugin::SerializeState(chunk);

  auto putSlice = [&](bn::slice<float> data) -> bool {
    return (chunk.PutBytes(data.data(), data.len() * sizeof(float)) > 0);
  };

  auto voice = GetVoice();
  saveOk &= putSlice(voice->mGainGraph.GetValues());
  saveOk &= putSlice(voice->mDutyGraph.GetValues());
  saveOk &= putSlice(voice->mPitchGraph.GetValues());
  saveOk &= putSlice(voice->mFinePitchGraph.GetValues());
  return saveOk;
}

int NESting::UnserializeState(const IByteChunk& chunk, int startPos)
{
  int pos = startPos;

  uint32_t saveVersion = 0;
  pos = chunk.Get(&saveVersion, pos);

  if (saveVersion != CURRENT_VERSION) {
    // Return early
    mUIQueue.Push(QMsg(1, "Error: Invalid preset"));
    return pos;
  }

  pos = Plugin::UnserializeState(chunk, pos);

  WDL_TypedBuf<float> buf;
  buf.Resize(MAX_LFO_GRAPH_STEPS);
  auto getSlice = [&](LFOGraph& graph, int pos) -> int {
    int dataSize = sizeof(float) * MAX_LFO_GRAPH_STEPS;
    int npos = chunk.GetBytes(buf.Get(), dataSize, pos);
    graph.SetValues(bn::slice(buf.Get(), buf.GetSize()));
    return npos;
  };

  auto voice = GetVoice();
  pos = getSlice(voice->mGainGraph, pos);
  pos = getSlice(voice->mDutyGraph, pos);
  pos = getSlice(voice->mPitchGraph, pos);
  pos = getSlice(voice->mFinePitchGraph, pos);
  return pos;
}


