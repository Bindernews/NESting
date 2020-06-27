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

  // Update UI when the time or tempo sync params change.
  auto updateTempoSync = [&](int ctrlTag, int baseParam) {
    // bool tSync = GetParam(baseParam + 4)->Bool();
    // TODO set stepped
    GetUI()->GetControlWithTag(ctrlTag + 3)->SetDirty(false);
  };

  switch (paramIdx)
  {
  case iParamUseAutomationGraphs:
    UpdateUISize();
    break;
  case iParamVolumeTime:
  case iParamVolumeTempoSync:
    updateTempoSync(kCtrlVolumeGraph, iParamVolumeSteps);
    break;
  case iParamDutyTime:
  case iParamDutyTempoSync:
    updateTempoSync(kCtrlDutyGraph, iParamDutySteps);
    break;
  case iParamPitchTime:
  case iParamPitchTempoSync:
    updateTempoSync(kCtrlPitchGraph, iParamPitchSteps);
    break;
  case iParamFineTime:
  case iParamFineTempoSync:
    updateTempoSync(kCtrlFineGraph, iParamFineSteps);
    break;
  }
}

void NESting::OnRestoreState()
{
  Plugin::OnRestoreState();
  updateGraphUI();
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
    auto buildGraphPanel = [this, ui, style](const IRECT& b, IColor color, float defaultValue, int ctrlTag, int paramId,
          int nSteps, DisplayMessageCallback cb) {
      auto graph = new LFOGraphControl(b.GetFromTop(80), MIN_LFO_GRAPH_STEPS, MAX_LFO_GRAPH_STEPS, defaultValue, "", style);
      graph->iStyle.barColor = color;
      graph->SetParamIdx(paramId + 0, 0);
      graph->SetParamIdx(paramId + 1, 1);
      graph->SetParamIdx(paramId + 2, 2);
      graph->SetNumSteps(nSteps);
      graph->SetActionFunction([&, ctrlTag](IControl* control) {
        auto barValues = control->As<LFOGraphControl>()->GetBarValues();
        SendArbitraryMsgFromUI(0, ctrlTag + 0, int(barValues.len()) * sizeof(float), barValues.data());
        });
      graph->SetDisplayFn(cb);
      ui->AttachControl(graph, ctrlTag + 0, GRAPHS_GROUP);

      IRECT knobArea = b.GetAltered(0., 90., 0., 0.).GetFromTop(60.);
      ui->AttachControl(new IVKnobControl(knobArea.GetGridCell(0, 1, 4), paramId + 0, "Steps", style), ctrlTag + 1, GRAPHS_GROUP);
      ui->AttachControl(new IVKnobControl(knobArea.GetGridCell(1, 1, 4), paramId + 3, "Time", style), ctrlTag + 3, GRAPHS_GROUP);
      ui->AttachControl(new IVToggleControl(knobArea.GetGridCell(2, 1, 4), paramId + 4, "Tempo Sync", style), ctrlTag + 4, GRAPHS_GROUP);
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
    auto display_1 = [](float value, WDL_String& msg) { msg.SetFormatted(20, "%3d%%", int(value * 100)); };
    addLabel(b_1, "Volume");
    buildGraphPanel(b_1, COLOR_RED, DEFAULT_ENV_VOLUME, kCtrlVolumeGraph, iParamVolumeSteps,
                    16, display_1);

    // Duty cycle
    IRECT b_2 = bGraph.GetGridCell(1, 2, 2).GetPadded(GRAPH_PAD);
    addLabel(b_2, "Duty Cycle (Square)");
    static const char* DUTY_CYCLE_STRINGS[] = { DUTY_CYCLE_LIST };
    auto display_2 = [&](float value, WDL_String& msg) {
      msg.SetFormatted(20, "%s", DUTY_CYCLE_STRINGS[int(value * 4)]);
    };
    buildGraphPanel(b_2, COLOR_ORANGE, DEFAULT_ENV_DUTY, kCtrlDutyGraph, iParamDutySteps,
                    5, display_2);

    // Pitch
    IRECT b_3 = bGraph.GetGridCell(2, 2, 2).GetPadded(GRAPH_PAD);
    addLabel(b_3, "Pitch");
    auto display_3 = [](float value, WDL_String& msg) {
      msg.SetFormatted(20, "%d", int(Lerp(PITCH_LOW, PITCH_HIGH, value)));
    };
    buildGraphPanel(b_3, COLOR_BLUE, DEFAULT_ENV_PITCH, kCtrlPitchGraph, iParamPitchSteps,
                    24, display_3);

    // Fine pitch
    IRECT b_4 = bGraph.GetGridCell(3, 2, 2).GetPadded(GRAPH_PAD);
    addLabel(b_4, "Fine Pitch");
    auto display_4 = [](float value, WDL_String& msg) {
      msg.SetFormatted(20, "%1.2f", Lerp(-1.f, 1.f, value));
    };
    buildGraphPanel(b_4, COLOR_GREEN, DEFAULT_ENV_FINE_PITCH, kCtrlFineGraph, iParamFineSteps,
                    12, display_4);
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

  updateGraphUI();

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

void NESting::updateGraphUI()
{ 
  auto ui = GetUI();
  if (!ui) { return; }

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
}

#endif
