#include "NESting.h"
#include "IPlug_include_in_plug_src.h"
#include "math_utils.h"
#include "gui/LFOGraphControl.h"
#include "constants.h"
#include <cmath>
#include <vector>
#include <array>
#include <algorithm>
#include <execution>

///////////////
// Constants //
///////////////

#define NOISE_MODE_LIST "32767 Steps", "93 Steps"
#define WAVE_SHAPE_LIST "Square", "Triangle", "Noise", "DPCM"
#define GRAPHS_GROUP "automationGraph"
#define ENVELOPES_GROUP "envelopes"
#define DUTY_CYCLE_LIST "6%", "12.5%", "25%", "50%", "75%",
#define AUTOMATION_PANEL_HEIGHT (200)

#define TEMPO_LIST_SIZE (15)
const char* TEMPO_DIVISION_NAMES[15] = { "1/64", "1/32", "1/16T", "1/16", "1/16D", "1/8T", "1/8", "1/8D", "1/4", "1/4D", "1/2", "1/1", "2/1", "4/1", "8/1" };
const double TEMPO_DIVISION_VALUES[15] = {
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

// Helper function for making graphics
IRECT SubRect(const IRECT& b, float x, float y, float w, float h)
{
    return b.GetAltered(x, y, 0., 0.).GetFromTLHC(w, h);
}

static int calcLoopPoint(double value, int numSteps)
{
    return int(value * (double(numSteps) - 0.01));
}

NESting::NESting(const InstanceInfo& info)
: Plugin(info, MakeConfig(kNumParams, 1)),
  mUIQueue(20)
{
    initParams();

#if IPLUG_DSP
    mSynth.AddVoice(new NESVoice(*this), 0);
    initPresets();
#endif
  
#if IPLUG_EDITOR
    mMakeGraphicsFunc = [&]() {
        return MakeGraphics(*this, PLUG_WIDTH, PLUG_HEIGHT, PLUG_FPS, GetScaleForScreen(PLUG_HEIGHT));
    };
    mLayoutFunc = [this](IGraphics* ui) { buildUI(ui); };
#endif
}


void NESting::initParams()
{
    GetParam(iParamShape)->InitEnum("Wave Shape", 0, { WAVE_SHAPE_LIST });
    GetParam(iParamNoiseMode)->InitEnum("Noise Mode", 0, { NOISE_MODE_LIST });
    GetParam(iParamAttack)->InitMilliseconds("Attack", 1, 0, ADSR_MAX_TIME);
    GetParam(iParamDecay)->InitMilliseconds("Decay", 1, 0, ADSR_MAX_TIME);
    GetParam(iParamSustain)->InitDouble("Sustain", 1, 0, 1, 0.01);
    GetParam(iParamRelease)->InitMilliseconds("Release", 1, 0, ADSR_MAX_TIME);
    GetParam(iParamUseAutomationGraphs)->InitBool("Graphs", false, "Automation Graphs", IParam::kFlagCannotAutomate | IParam::kFlagMeta);

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

//////////////////////
// Editor-Only Code //
//////////////////////


#if IPLUG_EDITOR
void NESting::ShowAutomationGraphs(bool visible)
{
    if (!GetUI()) { return; }
    GetUI()->ForControlInGroup(ENVELOPES_GROUP, [&](IControl& control) { control.Hide(visible); });
    GetUI()->ForControlInGroup(GRAPHS_GROUP, [&](IControl& control) { control.Hide(!visible); });
}

void NESting::OnParamChangeUI(int paramIdx, const EParamSource source)
{
    // This can get called even when the UI isn't being displayed.
    if (!GetUI()) { return; }

    // Update UI when a steps or loop point knob changes.
    auto updateGraphAndKnobs = [&](int ctrlTag, int baseParam) {
        auto ui = GetUI();
        auto graph = ui->GetControlWithTag(ctrlTag)->As<LFOGraphControl>();
        // Make sure nothing has gone horribly wrong.
        assert(graph != nullptr);
        int numSteps = GetParam(baseParam + 0)->Int();
        double loopPoint = GetParam(baseParam + 1)->Value();

        graph->SetSteps(numSteps);
        graph->SetLoopPoint(calcLoopPoint(loopPoint, numSteps));
        graph->SetDirty(false);
        // Set the loop point knob as dirty when we change the step knob.
        ui->GetControlWithTag(ctrlTag + 2)->SetDirty(false);
        // The other two params don't matter. They only influence the DSP.
    };

    // Update UI when the time or tempo sync params change.
    auto updateTempoSync = [&](int ctrlTag, int baseParam) {
        // bool tSync = GetParam(baseParam + 3)->Bool();
        // TODO set stepped
        GetUI()->GetControlWithTag(ctrlTag + 3)->SetDirty(false);
    };

    switch (paramIdx)
    {
    case iParamUseAutomationGraphs:
        ShowAutomationGraphs(GetParam(paramIdx)->Bool());
        break;

    case iParamVolumeSteps:
    case iParamVolumeLoopPoint:
        updateGraphAndKnobs(kCtrlVolumeGraph, iParamVolumeSteps);
        break;
    case iParamVolumeTime:
    case iParamVolumeTempoSync:
        updateTempoSync(kCtrlVolumeGraph, iParamVolumeSteps);
        break;
    case iParamDutySteps:
    case iParamDutyLoopPoint:
        updateGraphAndKnobs(kCtrlDutyGraph, iParamDutySteps);
        break;
    case iParamDutyTime:
    case iParamDutyTempoSync:
        updateTempoSync(kCtrlDutyGraph, iParamDutySteps);
        break;
    case iParamPitchSteps:
    case iParamPitchLoopPoint:
        updateGraphAndKnobs(kCtrlPitchGraph, iParamPitchSteps);
        break;
    case iParamPitchTime:
    case iParamPitchTempoSync:
        updateTempoSync(kCtrlPitchGraph, iParamPitchSteps);
        break;
    case iParamFineSteps:
    case iParamFineLoopPoint:
        updateGraphAndKnobs(kCtrlFineGraph, iParamFineSteps);
        break;
    case iParamFineTime:
    case iParamFineTempoSync:
        updateTempoSync(kCtrlFineGraph, iParamFineSteps);
        break;
    }
}

void NESting::buildUI(IGraphics *ui)
{
  ui->EnableMouseOver(true);
  ui->EnableMultiTouch(true);
  ui->SetLayoutOnResize(true);

  IRECT b = ui->GetBounds().GetPadded(-20);

  IRECT knobs = b.GetFromTop(60.);
  IRECT viz = SubRect(b, 0., 80., b.W(), 80.);
  ui->AttachCornerResizer(EUIResizerMode::Scale);
  ui->LoadFont("Roboto-Regular", ROBOTO_FN);

  IVStyle style = DEFAULT_STYLE.WithLabelText(IText(14.f));

  for (int i = 0; i < 6; i++) {
    ui->AttachControl(new IVKnobControl(knobs.GetGridCell(i, 1, 7), i, "", style, false, false));
  }
  ui->AttachControl(new IVSwitchControl(knobs.GetGridCell(6, 1, 7), iParamUseAutomationGraphs));

  ui->AttachPanelBackground(COLOR_GRAY);
  ui->AttachControl(new IVScopeControl<2>(viz, "", DEFAULT_STYLE.WithColor(kBG, COLOR_BLACK).WithColor(kFG, COLOR_GREEN)), kCtrlTagScope);

  // Build automation graph panels
  {
    IRECT bGraph = SubRect(b, 0., 170., b.W(), 400.);
    auto panel = new IVPanelControl(bGraph);
    ui->AttachControl(panel, -1, GRAPHS_GROUP);

    // ctrl tags: { graph, steps, loop point, time, tempo sync }
    // paramIds: { idSteps, idLoop, idTime, idTimeSync }
    auto buildGraphPanel = [this, ui, style](const IRECT& b, IColor color, float defaultValue, int ctrlTag, int paramId, const LabelPoint* stops, int stopCount) {
      auto graph = new LFOGraphControl(b.GetFromTop(80), MAX_LFO_GRAPH_STEPS, defaultValue, "", style);
      graph->iStyle.barColor = color;
      graph->SetStops(bn::slice<LabelPoint>(stops, stopCount));
      graph->SetSteps(8);
      graph->SetActionFunction([&, ctrlTag](IControl* control) {
        auto barValues = control->As<LFOGraphControl>()->GetBarValues();
        SendArbitraryMsgFromUI(0, ctrlTag + 0, int(barValues.len()) * sizeof(float), barValues.data());
        });
      ui->AttachControl(graph, ctrlTag + 0, GRAPHS_GROUP);

      IRECT knobArea = b.GetAltered(0., 90., 0., 0.).GetFromTop(60.);
      ui->AttachControl(new IVKnobControl(knobArea.GetGridCell(0, 1, 4), paramId + 0, "Steps", style), ctrlTag + 1, GRAPHS_GROUP);
      ui->AttachControl(new IVKnobControl(knobArea.GetGridCell(1, 1, 4), paramId + 1, "Loop Point", style), ctrlTag + 2, GRAPHS_GROUP);
      ui->AttachControl(new IVKnobControl(knobArea.GetGridCell(2, 1, 4), paramId + 2, "Time", style), ctrlTag + 3, GRAPHS_GROUP);
      ui->AttachControl(new IVToggleControl(knobArea.GetGridCell(3, 1, 4), paramId + 3, "Tempo Sync", style), ctrlTag + 4, GRAPHS_GROUP);
    };

    // Volume
    IRECT b_1 = bGraph.GetGridCell(0, 2, 2).GetPadded(-10);
    std::vector<LabelPoint> stops_1;
    for (int i = 0; i < 21; i++) {
      stops_1.push_back(LabelPoint(float(i) / 20.f));
    }
    stops_1[0].label = "0%";
    stops_1[10].label = "50%";
    stops_1[20].label = "100%";
    buildGraphPanel(b_1, COLOR_RED, DEFAULT_ENV_VOLUME, kCtrlVolumeGraph, iParamVolumeSteps, stops_1.data(), stops_1.size());

    // Duty cycle
    IRECT b_2 = bGraph.GetGridCell(1, 2, 2).GetPadded(-10);
    std::array<LabelPoint, 5> stops_2 = { LabelPoint(0.00f, " 6%"), LabelPoint(0.25f, "12%"),
        LabelPoint(0.50f, "25%"), LabelPoint(0.75f, "50%"), LabelPoint(1.00f, "75%"), };
    buildGraphPanel(b_2, COLOR_ORANGE, DEFAULT_ENV_DUTY, kCtrlDutyGraph, iParamDutySteps, stops_2.data(), stops_2.size());

    // Pitch
    IRECT b_3 = bGraph.GetGridCell(2, 2, 2).GetPadded(-10);
    std::vector<LabelPoint> stops_3;
    for (int i = 0; i < 25; i++) {
      stops_3.push_back(LabelPoint(float(i) / 24.f));
    }
    stops_3[0].label = "-12";
    stops_3[12].label = "0";
    stops_3[24].label = "12";
    buildGraphPanel(b_3, COLOR_BLUE, DEFAULT_ENV_PITCH, kCtrlPitchGraph, iParamPitchSteps, stops_3.data(), stops_3.size());

    // Fine pitch
    IRECT b_4 = bGraph.GetGridCell(3, 2, 2).GetPadded(-10);
    std::vector<LabelPoint> stops_4;
    for (int i = 0; i < 17; i++) {
      stops_4.push_back(LabelPoint(float(i) / 16.f));
    }
    stops_4[0].label = "-1";
    stops_4[8].label = "0";
    stops_4[16].label = "1";
    buildGraphPanel(b_4, COLOR_GREEN, DEFAULT_ENV_FINE_PITCH, kCtrlFineGraph, iParamFineSteps, stops_4.data(), stops_4.size());
  }

  // Build the generic envelopes
  IRECT bEnvels = b.GetAltered(0., 180., 0., 0.).GetFromTop(80.);
  ui->AttachControl(new IVKnobControl(bEnvels.SubRectHorizontal(4, 0), iParamVolumeEnvelope),
    kCtrlVolumeGraph + 5, ENVELOPES_GROUP);
  ui->AttachControl(new IVKnobControl(bEnvels.SubRectHorizontal(4, 1), iParamDutyEnvelope),
    kCtrlDutyGraph + 5, ENVELOPES_GROUP);
  ui->AttachControl(new IVKnobControl(bEnvels.SubRectHorizontal(4, 2), iParamPitchEnvelope),
    kCtrlPitchGraph + 5, ENVELOPES_GROUP);
  ui->AttachControl(new IVKnobControl(bEnvels.SubRectHorizontal(4, 3), iParamFineEnvelope),
    kCtrlFineGraph + 5, ENVELOPES_GROUP);

  // Show/hide controls based on the current state of the Use LFO Graphs parameter.
  ShowAutomationGraphs(GetParam(iParamUseAutomationGraphs)->Bool());

  // Update the graph bars with values from the DSP
  // TODO this will fail if the EDITOR and DSP aren't defined together.
  auto setControlValues = [&](IControl* control, const LFOGraph & src) {
    auto graph = control->As<LFOGraphControl>();
    auto sValues = src.GetValues();
    for (int i = 0; i < sValues.len(); i++) {
      graph->SetBarValue(i, sValues[i], false);
    }
  };

  auto voice = dynamic_cast<NESVoice*>(mSynth.GetVoice(0));
  setControlValues(ui->GetControlWithTag(kCtrlVolumeGraph), voice->mGainGraph);
  setControlValues(ui->GetControlWithTag(kCtrlDutyGraph), voice->mDutyGraph);
  setControlValues(ui->GetControlWithTag(kCtrlPitchGraph), voice->mPitchGraph);
  setControlValues(ui->GetControlWithTag(kCtrlFineGraph), voice->mFinePitchGraph);

#ifdef APP_API
  IRECT keyboardBounds = b.GetFromBottom(200);
  ui->AttachControl(new IVKeyboardControl(keyboardBounds), kCtrlTagKeyboard);
  ui->SetQwertyMidiKeyHandlerFunc([ui](const IMidiMsg& msg) {
    dynamic_cast<IVKeyboardControl*>(ui->GetControlWithTag(kCtrlTagKeyboard))->SetNoteFromMidi(msg.NoteNumber(), msg.StatusMsg() == IMidiMsg::kNoteOn);
    });
#endif
}

#endif

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
}

NESVoice* NESting::GetVoice() const
{
  return dynamic_cast<NESVoice*>(const_cast<NESting*>(this)->mSynth.GetVoice(0));
}
#endif
