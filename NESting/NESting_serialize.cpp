#include "NESting.h"
#include "constants.h"
#include "bson/bsonobjbuilder.h"
#include "bson/bsonobjiterator.h"

using namespace _bson;

enum StateVersion {
  kSaveV0 = 0x00000000,
  kSaveV1 = 0x00000001,
};

#define CURRENT_VERSION kSaveV1

/** Returns true if the field name of a bson element is equal to the given string. */
bool fieldNameEq(const bsonelement& elem, const char* rhs)
{
  return strncmp(elem.fieldName(), rhs, elem.fieldNameSize()) == 0;
}

bool NESting::SerializeState(IByteChunk& chunk) const
{
  bool saveOk = true;

  // First write the version info
  uint32_t versionData = CURRENT_VERSION;
  saveOk &= (chunk.Put(&versionData) > 0);

  // Now serialize the plugin state
  saveOk &= Plugin::SerializeState(chunk);

  bsonobjbuilder bb;

  auto putSlice = [&](const char *name, bn::slice<float> data) {
    bb.appendBinData(name, data.len() * sizeof(float), BinDataGeneral, data.data());
  };

  auto voice = GetVoice();
  putSlice("gainGraph", voice->mGainGraph.GetValues());
  putSlice("dutyGraph", voice->mDutyGraph.GetValues());
  putSlice("pitchGraph", voice->mPitchGraph.GetValues());
  putSlice("fineGraph", voice->mFinePitchGraph.GetValues());

  // Save the BSON data we have
  auto bob = bb.done();
  uint32_t bsonSize = bob.objsize();
  saveOk &= (chunk.Put(&bsonSize) > 0);
  saveOk &= (chunk.PutBytes(bob.objdata(), bob.objsize()) > 0);

  return saveOk;
}

int NESting::UnserializeState(const IByteChunk& chunk, int startPos)
{
  int pos = startPos;

  uint32_t saveVersion = 0;
  pos = chunk.Get(&saveVersion, pos);

  if (saveVersion == kSaveV0) {
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
  else if (saveVersion == kSaveV1) {
    pos = Plugin::UnserializeState(chunk, pos);

    auto voice = GetVoice();

    auto getSlice = [&](const bsonelement& elem, LFOGraph& graph) {
      int dlen;
      const char* ptr = elem.binDataClean(dlen);
      graph.SetValues(bn::slice(reinterpret_cast<const float*>(ptr), dlen / sizeof(float)));
    };

    uint32_t bsonSize;
    pos = chunk.Get(&bsonSize, pos);
    const char* bsonStart = reinterpret_cast<const char*>(chunk.GetData() + pos);
    bsonobjiterator bIter = bsonobjiterator(bsonStart, bsonStart + bsonSize);
    while (bIter.more()) {
      auto elem = bIter.next();

      if (fieldNameEq(elem, "gainGraph")) {
        getSlice(elem, voice->mGainGraph);
      }
      else if (fieldNameEq(elem, "dutyGraph")) {
        getSlice(elem, voice->mDutyGraph);
      }
      else if (fieldNameEq(elem, "pitchGraph")) {
        getSlice(elem, voice->mPitchGraph);
      }
      else if (fieldNameEq(elem, "fineGraph")) {
        getSlice(elem, voice->mFinePitchGraph);
      }
    }

    pos += bsonSize;

    return pos;
  }
  else {
    // Return early
    mUIQueue.Push(QMsg(1, "Error: Invalid preset"));
    return pos;
  }


  

  

  
}


