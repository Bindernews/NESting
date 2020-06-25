#include "NESting.h"
#include "IPlug_include_in_plug_src.h"
#include "math_utils.h"
#include "gui/LFOGraphControl.h"
#include "constants.h"
#include <cmath>
#include <vector>
#include <array>
#include <algorithm>
#include "presets/presets.h"

///////////////
// Constants //
///////////////

const char* TEMPO_DIVISION_NAMES[TEMPO_LIST_SIZE] = { "1/64", "1/32", "1/16T", "1/16", "1/16D", "1/8T", "1/8", "1/8D", "1/4", "1/4D", "1/2", "1/1", "2/1", "4/1", "8/1" };
const double TEMPO_DIVISION_VALUES[TEMPO_LIST_SIZE] = {
    1. / 64.,
    1. / 32.,
    1. / 24.,
    1. / 16.,
    1. / 9.,
    1. / 8.,
    1. / 6.,
    1. / 1.,
    1. / 0.5,
    1. / 0.25,
    1. / 0.125,
};

//////////////////////
// Helper Functions //
//////////////////////

NESting::NESting(const InstanceInfo& info)
: Plugin(info, MakeConfig(kNumParams, PRESET_LIST_SIZE + 1)),
  mUIQueue(20)
{
    initParams();

#if IPLUG_DSP
    mSynth.AddVoice(new NESVoice(*this), 0);
#endif
  
#if IPLUG_EDITOR
    mMakeGraphicsFunc = [&]() {
        return MakeGraphics(*this, PLUG_WIDTH, PLUG_HEIGHT, PLUG_FPS, GetScaleForScreen(PLUG_HEIGHT));
    };
    mLayoutFunc = [this](IGraphics* ui) { layoutUI(ui); };
#endif

    initPresets();
}


void NESting::initParams()
{
    GetParam(iParamShape)->InitEnum("Wave Shape", 0, { WAVE_SHAPE_LIST });
    GetParam(iParamNoiseMode)->InitEnum("Noise Mode", 0, { NOISE_MODE_LIST });
    GetParam(iParamAttack)->InitMilliseconds("Attack", 1, 0, ADSR_MAX_TIME);
    GetParam(iParamDecay)->InitMilliseconds("Decay", 1, 0, ADSR_MAX_TIME);
    GetParam(iParamSustain)->InitDouble("Sustain", 1, 0, 1, 0.01);
    GetParam(iParamRelease)->InitMilliseconds("Release", 1, 0, ADSR_MAX_TIME);
    GetParam(iParamUseAutomationGraphs)->InitBool("Graphs", true, "Automation Graphs", IParam::kFlagCannotAutomate | IParam::kFlagMeta);

    auto initGraphParams = [&](const char* prefix, int firstIdx) {
        WDL_String name;
        name.SetFormatted(50, "%s - Steps", prefix);
        GetParam(firstIdx + 0)->InitDouble(name.Get(), 8, 4, 64, 4, "", IParam::kFlagStepped);

        name.SetFormatted(50, "%s - Loop Point", prefix);
        GetParam(firstIdx + 1)->InitDouble(name.Get(), 1, 0, 1, 0.1);
        // For the loop point, we display the loop number relative to the current number of steps
        GetParam(firstIdx + 1)->SetDisplayFunc([&, firstIdx](double value, WDL_String& disp) {
            int numSteps = GetParam(firstIdx + 0)->Int();
            disp.SetFormatted(10, "%d", calcLoopPoint(value, numSteps));
            });

        name.SetFormatted(50, "%s - Time", prefix);
        GetParam(firstIdx + 2)->InitDouble(name.Get(), 0.5, 0, 1, 0.1);
        // Display either the value in milliseconds, or the tempo sync value when synchronizing with the tempo 
        GetParam(firstIdx + 2)->SetDisplayFunc([&, firstIdx](double value, WDL_String& disp) {
            bool tSync = GetParam(firstIdx + 3)->Bool();
            if (tSync) {
                int tempoIndex = NormToInt(0, TEMPO_LIST_SIZE, value);
                disp.SetFormatted(20, "%s", TEMPO_DIVISION_NAMES[tempoIndex]);
            }
            else {
                int timeMs = int(value * MAX_LFO_STEP_TIME_MS);
                disp.SetFormatted(20, "%d ms", timeMs);
            }
            });

        name.SetFormatted(50, "%s - Tempo Sync", prefix);
        GetParam(firstIdx + 3)->InitBool(name.Get(), true);


        name.SetFormatted(50, "%s Envelope", prefix);
        GetParam(firstIdx + 4)->InitDouble(name.Get(), 0.5, 0., 1., 0.01);
    };

    initGraphParams("Volume", iParamVolumeSteps);
    initGraphParams("Duty Cycle", iParamDutySteps);
    initGraphParams("Pitch", iParamPitchSteps);
    initGraphParams("Fine Pitch", iParamFineSteps);

    // Finally, initialize the relevant envelope parameter. The param doesn't do anything
    // if LFO graphs are enabled. It only works when they're disabled.
    GetParam(iParamVolumeEnvelope)->InitGain("Gain");
    GetParam(iParamDutyEnvelope)->InitEnum("Duty Cycle", 3, { DUTY_CYCLE_LIST });
    GetParam(iParamPitchEnvelope)->InitDouble("Pitch", 0, -12, 12, 1);
    GetParam(iParamFineEnvelope)->InitDouble("Fine Pitch", 0, -12, 12, 1);
}

///////////////////
// DSP-only Code //
///////////////////

#if IPLUG_DSP

void NESting::ProcessBlock(sample** inputs, sample** outputs, int nFrames)
{
    //sample* faustInputs[] = { mInputBuffer.Get() };

    // Pre-zero output buffers
    set_buffer(outputs[0], nFrames, 0.f);
    set_buffer(outputs[1], nFrames, 0.f);
    // Generate our sounds
    mSynth.ProcessBlock(nullptr, outputs, 3, 2, nFrames);
    memcpy(outputs[1], outputs[0], sizeof(sample) * nFrames);
    mScopeSender.ProcessBlock(outputs, nFrames, kCtrlTagScope);
}

void NESting::ProcessMidiMsg(const IMidiMsg& msg)
{
    TRACE;

    int status = msg.StatusMsg();

    switch (status)
    {
        case IMidiMsg::kNoteOn:
        case IMidiMsg::kNoteOff:
        case IMidiMsg::kPolyAftertouch:
        case IMidiMsg::kControlChange:
        case IMidiMsg::kProgramChange:
        case IMidiMsg::kChannelAftertouch:
        case IMidiMsg::kPitchWheel:
        {
            mSynth.AddMidiMsgToQueue(msg);
            SendMidiMsg(msg);
            break;
        }
        default:
            return;
    }
}

void NESting::OnReset()
{
    mSynth.Reset();
    mSynth.SetSampleRateAndBlockSize(GetSampleRate(), GetBlockSize());

    mInputBuffer.Resize(int(GetSampleRate()), true);
    for (int i = 0; i < mInputBuffer.GetSize(); i += 1) {
        mInputBuffer.Get()[i] = 0.f;
    }
}

void NESting::OnParamChange(int paramIdx)
{
    // Helper function that updates the fields of an LFOGraph based on the relevant parameters.
    auto updateLFOGraph = [&, paramIdx](LFOGraph& graph, int baseParam) {
        switch (paramIdx - baseParam)
        {
        case 0: // Steps
            graph.mNumSteps = GetParam(paramIdx)->Int();
            break;
        case 1: // Loop point
            graph.mLoopPoint = calcLoopPoint(GetParam(paramIdx)->Value(), graph.mNumSteps);
            break;
        case 2: // Time and Tempo Sync
        case 3:
            double timeNorm = GetParam(baseParam + 2)->Value();
            bool tSync = GetParam(baseParam + 3)->Bool();  // are we doing temo-sync or milliseconds
            if (tSync) {
                double tempoDiv = TEMPO_DIVISION_VALUES[NormToInt(0, TEMPO_LIST_SIZE, timeNorm)];
                graph.SetStepSamples(int(tempoDiv * GetSamplesPerBeat()));
            }
            else {
                // Convert timeNorm to time in milliseconds, then time in seconds.
                double timeSec = double(NormToInt(0, MAX_LFO_STEP_TIME_MS, timeNorm)) / 1000.;
                graph.SetStepSamples(int(TimeToSamples(timeSec, GetSampleRate())));
            }
        }
    };

    double vReal = GetParam(paramIdx)->Value();

#define LFO_PARAM_CASES(name) case iParam##name##Steps: case iParam##name##LoopPoint: \
    case iParam##name##Time: case iParam##name##TempoSync

    // Update the parameter state for our voices as well.
    for (size_t i = 0; i < mSynth.NVoices(); i++) {
      auto voice = dynamic_cast<NESVoice*>(mSynth.GetVoice(i));
        
      switch (paramIdx) {
      case iParamAttack:
        voice->mADSR.SetStageTime(0, sample(vReal));
        break;
      case iParamDecay:
        voice->mADSR.SetStageTime(1, sample(vReal));
      case iParamSustain:
        voice->mSustain = float(vReal);
      case iParamRelease:
        voice->mADSR.SetStageTime(3, sample(vReal));
        break;
      
      LFO_PARAM_CASES(Volume):
        updateLFOGraph(voice->mGainGraph, iParamVolumeSteps);
        break;
      LFO_PARAM_CASES(Duty):
        updateLFOGraph(voice->mDutyGraph, iParamDutySteps);
        break;
      LFO_PARAM_CASES(Pitch):
        updateLFOGraph(voice->mPitchGraph, iParamPitchSteps);
        break;
      LFO_PARAM_CASES(Fine) :
        //updateLFOGraph(voice->m)
        break;
      case iParamShape:
        voice->mShape = GetParam(paramIdx)->Int();
        break;
      }
    };
}

bool NESting::OnMessage(int msgTag, int ctrlTag, int dataSize, const void* pData)
{
  if (ctrlTag == kCtrlTagBender && msgTag == IWheelControl::kMessageTagSetPitchBendRange)
  {
      const int bendRange = *static_cast<const int*>(pData);
      mSynth.SetPitchBendRange(bendRange);
  }

  // This SHOULD be handled on the DSP thread, as the data is being passed from the UI thread.
  // TODO only do this if necessary.
  const float* fData = static_cast<const float*>(pData);
  size_t fDataSize = size_t(dataSize) / sizeof(float);
  for (size_t i = 0; i < mSynth.NVoices(); i += 1) {
    auto voice = dynamic_cast<NESVoice*>(mSynth.GetVoice(i));
    switch (ctrlTag)
    {
    case kCtrlVolumeGraph:
      voice->mGainGraph.SetValues(bn::slice(fData, fDataSize));
      break;
    case kCtrlDutyGraph:
      voice->mDutyGraph.SetValues(bn::slice(fData, fDataSize));
      break;
    case kCtrlPitchGraph:
      voice->mPitchGraph.SetValues(bn::slice(fData, fDataSize));
      break;
    case kCtrlFineGraph:
      voice->mFinePitchGraph.SetValues(bn::slice(fData, fDataSize));
      break;
    }
  };

  return false;
}

void NESting::SendControlMsgFromDelegate(int ctrlTag, int msgTag, int dataSize, const void* pData)
{
    // Call the superclass method.
    Plugin::SendControlMsgFromDelegate(ctrlTag, msgTag, dataSize, pData);
}

void NESting::OnIdle()
{
  mScopeSender.TransmitData(*this);


  if (GetUI())
  {
    QMsg msg;
    while (mUIQueue.Pop(msg))
    {
      switch (msg.kind)
      {
      case 1:
        GetUI()->ShowMessageBox(msg.data_as<char>(), PLUG_NAME " - Error", EMsgBoxType::kMB_OK);
        break;
      }
      msg.done();
    }
  }
}

void NESting::initPresets()
{
  for (int i = 0; i < PRESET_LIST_SIZE; i++) {
    const rp_preset_t& pre = PRESET_LIST[i];
    IByteChunk chunk;
    chunk.PutBytes(pre.data, pre.size);
    MakePresetFromChunk(pre.name, chunk);
  }
}

NESVoice* NESting::GetVoice() const
{
  return dynamic_cast<NESVoice*>(const_cast<NESting*>(this)->mSynth.GetVoice(0));
}
#endif
