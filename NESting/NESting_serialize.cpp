#include "NESting.h"
#include "constants.h"
#include "bson/bsonobjbuilder.h"
#include "bson/bsonobjiterator.h"
#include "wdl_base64.h"

using namespace _bson;

#define CURRENT_SAVE_VERSION 2

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

bool NESting::SerializeState(IByteChunk& chunk) const
{
  bool saveOk = true;

  // First write the version info
  uint32_t versionData = CURRENT_SAVE_VERSION;
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

  // Make sure we're not loading an invalid save version
  if (saveVersion != CURRENT_SAVE_VERSION) {
    // Return early
    mUIQueue.Push(QMsg(1, "Error: Invalid preset"));
    return pos;
  }

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

    if (fieldNameEq(elem, "params")) {
      auto slice = bsonSlice<double>(elem);
      for (int i = 0; i < slice.len(); i++) {
        GetParam(i)->SetNormalized(slice[i]);
        OnParamChange(i);
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

