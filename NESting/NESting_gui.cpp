#include "NESting.h"
#include "constants.h"
#include "gui/LFOGraphControl.h"

#if IPLUG_EDITOR


const IColorStop COLOR_STOP_BG_TOP = IColorStop(IColor(255, 191, 191, 191), 0.f);
const IColorStop COLOR_STOP_BG_BOT = IColorStop(IColor(255, 129, 129, 129), 1.f);

void NESting::UpdateUISize()
{
  auto ui = GetUI();
  if (!ui) { return; }
  int newH = int(getDesiredHeight());
  ui->Resize(PLUG_WIDTH, newH, ui->GetDrawScale(), true);
  LayoutUI(ui);
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
    UpdateUISize();
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

void NESting::buildUI(IGraphics* ui)
{
  ui->EnableMouseOver(true);
  ui->EnableMultiTouch(true);
  ui->SetLayoutOnResize(true);

  IRECT b = ui->GetBounds().GetPadded(-20);

  IRECT knobs = b.GetFromTop(60.);
  IRECT viz = b.GetTranslated(0., 80.).GetFromTop(80.);
  ui->AttachCornerResizer(EUIResizerMode::Scale);
  ui->LoadFont("Roboto-Regular", ROBOTO_FN);

  // Add background panel
  ui->AttachPanelBackground(COLOR_WHITE);

  IVStyle style = DEFAULT_STYLE.WithLabelText(IText(14.f));

  for (int i = 0; i < 6; i++) {
    ui->AttachControl(new IVKnobControl(knobs.GetGridCell(i, 1, 7), i, "", style, false, false));
  }
  ui->AttachControl(new IVSwitchControl(knobs.GetGridCell(6, 1, 7), iParamUseAutomationGraphs));

  ui->AttachControl(new IVScopeControl<2>(viz, "", DEFAULT_STYLE.WithColor(kBG, COLOR_BLACK).WithColor(kFG, COLOR_GREEN)), kCtrlTagScope);

  // Build automation graph panels
  {
    IRECT bGraph = b.GetTranslated(0., MAIN_PANEL_HEIGHT).GetFromTop(GRAPH_PANEL_HEIGHT);

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

    const float GRAPH_PAD = -10.f;
    const float LABEL_HEIGHT = 24.f;

    auto addLabel = [&](IRECT& area, const char* msg) {
      auto style = DEFAULT_STYLE.WithDrawFrame(false).WithDrawShadows(false)
        .WithValueText(DEFAULT_TEXT.WithSize(16.f).WithAlign(EAlign::Near).WithVAlign(EVAlign::Top));
      auto lbl = new IVLabelControl(area.GetFromTop(LABEL_HEIGHT).GetHShifted(2.f), msg, style);
      ui->AttachControl(lbl, -1, GRAPHS_GROUP);
      area.Translate(0., LABEL_HEIGHT);
    };

    // Volume
    IRECT b_1 = bGraph.GetGridCell(0, 2, 2).GetPadded(GRAPH_PAD);
    addLabel(b_1, "Volume");
    std::vector<LabelPoint> stops_1;
    for (int i = 0; i < 21; i++) {
      stops_1.push_back(LabelPoint(float(i) / 20.f));
    }
    stops_1[0].label = "0%";
    stops_1[10].label = "50%";
    stops_1[20].label = "100%";
    buildGraphPanel(b_1, COLOR_RED, DEFAULT_ENV_VOLUME, kCtrlVolumeGraph, iParamVolumeSteps, stops_1.data(), stops_1.size());

    // Duty cycle
    IRECT b_2 = bGraph.GetGridCell(1, 2, 2).GetPadded(GRAPH_PAD);
    addLabel(b_2, "Duty Cycle (Square)");
    std::array<LabelPoint, 5> stops_2 = { LabelPoint(0.00f, " 6%"), LabelPoint(0.25f, "12%"),
        LabelPoint(0.50f, "25%"), LabelPoint(0.75f, "50%"), LabelPoint(1.00f, "75%"), };
    buildGraphPanel(b_2, COLOR_ORANGE, DEFAULT_ENV_DUTY, kCtrlDutyGraph, iParamDutySteps, stops_2.data(), stops_2.size());

    // Pitch
    IRECT b_3 = bGraph.GetGridCell(2, 2, 2).GetPadded(GRAPH_PAD);
    addLabel(b_3, "Pitch");
    std::vector<LabelPoint> stops_3;
    for (int i = 0; i < 25; i++) {
      stops_3.push_back(LabelPoint(float(i) / 24.f));
    }
    stops_3[0].label = "-12";
    stops_3[12].label = "0";
    stops_3[24].label = "12";
    buildGraphPanel(b_3, COLOR_BLUE, DEFAULT_ENV_PITCH, kCtrlPitchGraph, iParamPitchSteps, stops_3.data(), stops_3.size());

    // Fine pitch
    IRECT b_4 = bGraph.GetGridCell(3, 2, 2).GetPadded(GRAPH_PAD);
    addLabel(b_4, "Fine Pitch");
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
  IRECT bEnvels = b.GetAltered(0., MAIN_PANEL_HEIGHT, 0., 0.).GetFromTop(AUTOMATION_PANEL_HEIGHT);
  ui->AttachControl(new IVKnobControl(bEnvels.SubRectHorizontal(4, 0), iParamVolumeEnvelope),
    kCtrlVolumeGraph + 5, ENVELOPES_GROUP);
  ui->AttachControl(new IVKnobControl(bEnvels.SubRectHorizontal(4, 1), iParamDutyEnvelope),
    kCtrlDutyGraph + 5, ENVELOPES_GROUP);
  ui->AttachControl(new IVKnobControl(bEnvels.SubRectHorizontal(4, 2), iParamPitchEnvelope),
    kCtrlPitchGraph + 5, ENVELOPES_GROUP);
  ui->AttachControl(new IVKnobControl(bEnvels.SubRectHorizontal(4, 3), iParamFineEnvelope),
    kCtrlFineGraph + 5, ENVELOPES_GROUP);

  // Update the graph bars with values from the DSP
  // TODO this will fail if the EDITOR and DSP aren't defined together.
  auto setControlValues = [&](IControl* control, const LFOGraph& src) {
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
  IRECT keyboardBounds = b.GetFromBottom(KEYBOARD_HEIGHT);
  ui->AttachControl(new IVKeyboardControl(keyboardBounds), kCtrlTagKeyboard);
  ui->SetQwertyMidiKeyHandlerFunc([ui](const IMidiMsg& msg) {
    dynamic_cast<IVKeyboardControl*>(ui->GetControlWithTag(kCtrlTagKeyboard))->SetNoteFromMidi(msg.NoteNumber(), msg.StatusMsg() == IMidiMsg::kNoteOn);
    });
#endif
  
}

void NESting::layoutUI(IGraphics* ui)
{
  if (ui->NControls() == 0)
  {
    buildUI(ui);
  }

  IRECT b = ui->GetBounds().GetPadded(-20);

  // Adjust the background
  auto bgPanel = ui->GetBackgroundControl()->As<IPanelControl>();
  bgPanel->SetPattern(
    IPattern::CreateLinearGradient(b, EDirection::Vertical, { COLOR_STOP_BG_TOP, COLOR_STOP_BG_BOT }));
  bgPanel->SetRECT(ui->GetBounds());

#ifdef APP_API
  // Adjust the location of the keyboard.
  IRECT keyboardBounds = b.GetFromBottom(KEYBOARD_HEIGHT);
  ui->GetControlWithTag(kCtrlTagKeyboard)->SetRECT(keyboardBounds);
#endif

  // Show/hide controls based on the current state of the Use LFO Graphs parameter.
  {
    bool visible = GetParam(iParamUseAutomationGraphs)->Bool();
    ui->ForControlInGroup(ENVELOPES_GROUP, [&](IControl& control) { control.Hide(visible); });
    ui->ForControlInGroup(GRAPHS_GROUP, [&](IControl& control) { control.Hide(!visible); });
  }
}


float NESting::getDesiredHeight()
{
  float h = MAIN_PANEL_HEIGHT;
  if (GetParam(iParamUseAutomationGraphs)->Bool()) {
    h += GRAPH_PANEL_HEIGHT + PANEL_PADDING;
  }
  else {
    h += AUTOMATION_PANEL_HEIGHT;
  }
#ifdef APP_API
  h += KEYBOARD_PANEL_HEIGHT;
#endif
  return h;
}

#endif