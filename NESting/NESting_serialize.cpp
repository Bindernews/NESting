#include "NESting.h"
#include "constants.h"
#include "bson/bsonobjbuilder.h"
#include "bson/bsonobjiterator.h"
#include "wdl_base64.h"

using namespace _bson;

enum StateVersion {
  kSaveV0 = 0x00000000,
  kSaveV1 = 0x00000001,
  kSaveV2 = 0x00000002,
};

#define CURRENT_VERSION kSaveV2

/** Returns true if the field name of a bson element is equal to the given string. */
bool fieldNameEq(const bsonelement& elem, const char* rhs)
{
  return strncmp(elem.fieldName(), rhs, elem.fieldNameSize()) == 0;
}

/** @brief Creates a slice of the given type from a bson element of binary data */
template<typename T>
bn::slice<T> bsonSlice(const bsonelement& elem)
{
  int dlen;
  const char* ptr = elem.binDataClean(dlen);
  return bn::slice<T>(reinterpret_cast<const T*>(ptr), dlen / sizeof(T));
}

template<typename T>
void putSlice(bsonobjbuilder& bb, const char *name, bn::slice<T> data)
{
  bb.appendBinData(name, data.bytelen(), BinDataGeneral, data.data());
}

/** Map parameter indexes for save version 2 */
int mapParamV2(int paramId);

bool NESting::SerializeState(IByteChunk& chunk) const
{
  bool saveOk = true;

  // First write the version info
  uint32_t versionData = CURRENT_VERSION;
  saveOk &= (chunk.Put(&versionData) > 0);

  bsonobjbuilder bb;
  
  WDL_TypedBuf<double> paramBuf;
  for (int i = 0; i < NParams(); i++) {
    paramBuf.Add(GetParam(i)->GetNormalized());
  }
  putSlice(bb, "params", bn::slice<double>::FromWDL(paramBuf));

  auto voice = GetVoice();
  putSlice(bb, "gainGraph", voice->mGainGraph.GetValues());
  putSlice(bb, "dutyGraph", voice->mDutyGraph.GetValues());
  putSlice(bb, "pitchGraph", voice->mPitchGraph.GetValues());
  putSlice(bb, "fineGraph", voice->mFinePitchGraph.GetValues());

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
  else if (saveVersion == kSaveV2) {
    auto voice = GetVoice();

    auto graphSet = [&](const bsonelement& elem, LFOGraph& graph) {
      graph.SetValues(bsonSlice<float>(elem));
    };

    uint32_t bsonSize;
    pos = chunk.Get(&bsonSize, pos);
    const char* bsonStart = reinterpret_cast<const char*>(chunk.GetData() + pos);
    bsonobjiterator bIter = bsonobjiterator(bsonStart, bsonStart + bsonSize);
    while (bIter.more()) {
      auto elem = bIter.next();
      
      // Parse out parameters
      if (fieldNameEq(elem, "params")) {
        auto params = bsonSlice<double>(elem);
        for (int i = 0; i < params.len(); i++) {
          int paramIdx = -1;
          // Here we can re-map parameters as they're added or removed.
          switch (saveVersion) {
            case kSaveV2:
              paramIdx = mapParamV2(i);
              break;
          }
          if (paramIdx != -1) {
            GetParam(paramIdx)->SetNormalized(params[i]);
          }
        }
      }
      if (fieldNameEq(elem, "gainGraph")) {
        graphSet(elem, voice->mGainGraph);
      }
      else if (fieldNameEq(elem, "dutyGraph")) {
        graphSet(elem, voice->mDutyGraph);
      }
      else if (fieldNameEq(elem, "pitchGraph")) {
        graphSet(elem, voice->mPitchGraph);
      }
      else if (fieldNameEq(elem, "fineGraph")) {
        graphSet(elem, voice->mFinePitchGraph);
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

int mapParamV2(int paramId)
{
  /*
    siParamShape = 0,
    siParamNoiseMode,
    siParamAttack,
    siParamDecay,
    siParamSustain,
    siParamRelease,
    siParamUseAutomationGraphs,

    siParamVolumeSteps,
    siParamVolumeLoopPoint,
    siParamVolumeTime,
    siParamVolumeTempoSync,
    siParamVolumeEnvelope,

    siParamDutySteps,
    siParamDutyLoopPoint,
    siParamDutyTime,
    siParamDutyTempoSync,
    siParamDutyEnvelope,

    siParamPitchSteps,
    siParamPitchLoopPoint,
    siParamPitchTime,
    siParamPitchTempoSync,
    siParamPitchEnvelope,

    siParamFineSteps,
    siParamFineLoopPoint,
    siParamFineTime,
    siParamFineTempoSync,
    siParamFineEnvelope,
  */
#define PGroup(name) iParam##name##Steps, iParam##name##LoopPoint, \
  iParam##name##Time, iParam##name##TempoSync, iParam##name##Envelope
  
  static const int TABLE[] = {
    iParamShape,
    iParamNoiseMode,
    iParamAttack,
    iParamDecay,
    iParamSustain,
    iParamRelease,
    iParamUseAutomationGraphs,
    
    PGroup(Volume),
    PGroup(Duty),
    PGroup(Pitch),
    PGroup(Fine),
  };
  return TABLE[paramId];
}
