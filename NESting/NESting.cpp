#include "NESting.h"
#include "IPlug_include_in_plug_src.h"
#include "math_utils.h"
#include "gui/ControlBarGraph.h"

#define NOISE_MODE_LIST "32767 Steps", "93 Steps"
#define WAVE_SHAPE_LIST "Square", "Triangle", "Noise", "DPCM"
#define GRAPHS_GROUP "automationGraph"

#define DUTY_CYCLE_STOPS LabelPoint(0.00f, " 6%"), LabelPoint(0.25f, "12%"), LabelPoint(0.50f, "25%"),\
    LabelPoint(0.75f, "50%"), LabelPoint(1.00f, "75%"),
#define VOLUME_STOPS LabelPoint(0.0f, " 0%"), LabelPoint(0.5f, "50%"), LabelPoint(1.0f, "100%"),
#define PITCH_STOPS LabelPoint(0.00f, "-12"), LabelPoint(1.00f, "12"),

// Helper function for making graphics
IRECT SubRect(const IRECT& b, float x, float y, float w, float h)
{
    return b.GetAltered(x, y, 0., 0.).GetFromTLHC(w, h);
}


NESting::NESting(const InstanceInfo& info)
: Plugin(info, MakeConfig(kNumParams, 1))
{
    GetParam(iParamGain)->InitGain("Gain");
    GetParam(iParamDuty)->InitInt("Duty Cycle", 2, 0, 4);
    GetParam(iParamShape)->InitEnum("Wave Shape", 0, { WAVE_SHAPE_LIST });
    GetParam(iParamNoiseMode)->InitEnum("Noise Mode", 0, { NOISE_MODE_LIST });
    GetParam(iParamUseAutomationGraphs)->InitBool("Graphs", false, "Automation Graphs", IParam::kFlagCannotAutomate | IParam::kFlagMeta);

    auto initGraphParams = [&](const char* prefix, int firstIdx) {
        WDL_String name;
        name.SetFormatted(50, "%s - Steps", prefix);
        GetParam(firstIdx + 0)->InitDouble(name.Get(), 8, 4, 64, 4);
        name.SetFormatted(50, "%s - Loop Point", prefix);
        GetParam(firstIdx + 1)->InitDouble(name.Get(), 1, 0, 1, 0.1);
        name.SetFormatted(50, "%s - Time", prefix);
        GetParam(firstIdx + 2)->InitDouble(name.Get(), 0.5, 0, 1, 0.1);
        name.SetFormatted(50, "%s - Tempo Sync", prefix);
        GetParam(firstIdx + 3)->InitBool(name.Get(), true);
    };

    initGraphParams("Volume", iParamVolumeSteps);
    initGraphParams("Duty Cycle", iParamDutySteps);
    initGraphParams("Pitch", iParamPitchSteps);

#if IPLUG_DSP
    mSynth.AddVoice(new NESVoice(*this), 0);
#endif
  
#if IPLUG_EDITOR
    mMakeGraphicsFunc = [&]() {
        return MakeGraphics(*this, PLUG_WIDTH, PLUG_HEIGHT, PLUG_FPS, GetScaleForScreen(PLUG_HEIGHT));
    };

    mLayoutFunc = [&](IGraphics* ui) {
        ui->EnableMouseOver(true);
        ui->EnableMultiTouch(true);
        ui->SetLayoutOnResize(true);

        IRECT b = ui->GetBounds().GetPadded(-20);

        IRECT knobs = b.GetFromTop(60.);
        IRECT viz = SubRect(b, 0., 80., b.W(), 80.);
        ui->AttachCornerResizer(EUIResizerMode::Scale);
        ui->LoadFont("Roboto-Regular", ROBOTO_FN);

        IVStyle style = DEFAULT_STYLE.WithLabelText(IText(14.f));

        for (int i = 0; i < 4; i++) {
            ui->AttachControl(new IVKnobControl(knobs.GetGridCell(i, 1, 5), i, "", style, false, false));
        }
        ui->AttachControl(new IVSwitchControl(knobs.GetGridCell(4, 1, 5), iParamUseAutomationGraphs));

        ui->AttachControl(new IVLabelControl(b.GetPadded(-10).GetFromTLHC(40, 40), "MIDI:"), kCtrlStatus);
    
        ui->AttachPanelBackground(COLOR_GRAY);
        ui->AttachControl(new IVScopeControl<2>(viz, "", DEFAULT_STYLE.WithColor(kBG, COLOR_BLACK).WithColor(kFG, COLOR_GREEN)), kCtrlTagScope);

        // Build automation graph panels
        {
            IRECT bGraph = SubRect(b, 0., 170., b.W(), 400.);
            auto panel = new IVPanelControl(bGraph);
            ui->AttachControl(panel, -1, GRAPHS_GROUP);

            // paramIds: { idSteps, idLoop, idTime, idTimeSync }
            auto buildGraphPanel = [ui, style](const IRECT& b, IColor color, float defaultValue, int* paramIds, const bn::slice<LabelPoint> stops) {
                auto graph = new ControlBarGraph(b.GetFromTop(80), 64, defaultValue, "", style);
                graph->iStyle.barColor = color;
                graph->SetStops(stops);
                graph->SetSteps(8);
                ui->AttachControl(graph, -1, GRAPHS_GROUP);

                IRECT knobArea = b.GetAltered(0., 90., 0., 0.).GetFromTop(60.);
                ui->AttachControl(new IVKnobControl(knobArea.GetGridCell(0, 1, 4), paramIds[0], "Steps", style), -1, GRAPHS_GROUP);
                ui->AttachControl(new IVKnobControl(knobArea.GetGridCell(1, 1, 4), paramIds[1], "Loop Point", style), -1, GRAPHS_GROUP);
                ui->AttachControl(new IVKnobControl(knobArea.GetGridCell(2, 1, 4), paramIds[2], "Time", style), -1, GRAPHS_GROUP);
                ui->AttachControl(new IVToggleControl(knobArea.GetGridCell(3, 1, 4), paramIds[3], "Tempo Sync", style), -1, GRAPHS_GROUP);
            };

            // Volume
            IRECT b_1 = bGraph.GetGridCell(0, 2, 2).GetPadded(-10);
            int paramIds_1[] = { iParamVolumeSteps, iParamVolumeLoopPoint, iParamVolumeTime, iParamVolumeTempoSync };
            LabelPoint stops_1[] = { VOLUME_STOPS };
            buildGraphPanel(b_1, COLOR_RED, 1.0f, paramIds_1, SLICE_ARRAY(LabelPoint, stops_1));
            // Duty cycle
            IRECT b_2 = bGraph.GetGridCell(1, 2, 2).GetPadded(-10);
            int paramIds_2[] = { iParamDutySteps, iParamDutyLoopPoint, iParamDutyTime, iParamDutyTempoSync };
            LabelPoint stops_2[] = { DUTY_CYCLE_STOPS };
            buildGraphPanel(b_2, COLOR_ORANGE, 1.0f, paramIds_2, SLICE_ARRAY(LabelPoint, stops_2));
            // Pitch
            IRECT b_3 = bGraph.GetGridCell(2, 2, 2).GetPadded(-10);
            int paramIds_3[] = { iParamPitchSteps, iParamPitchLoopPoint, iParamPitchTime, iParamPitchTempoSync };
            LabelPoint stops_3[] = { PITCH_STOPS };
            buildGraphPanel(b_3, COLOR_BLUE, 1.0f, paramIds_3, SLICE_ARRAY(LabelPoint, stops_3));
        }
        if (!GetParam(iParamUseAutomationGraphs)->Bool()) {
            // By default, hide all graph controls. We only show them if the param is true
            ui->ForControlInGroup(GRAPHS_GROUP, [&](IControl& control) { control.Hide(true); });
        }
        

#ifdef APP_API
        IRECT keyboardBounds = b.GetFromBottom(200);
        ui->AttachControl(new IVKeyboardControl(keyboardBounds), kCtrlTagKeyboard);
        ui->SetQwertyMidiKeyHandlerFunc([ui](const IMidiMsg& msg) {
            dynamic_cast<IVKeyboardControl*>(ui->GetControlWithTag(kCtrlTagKeyboard))->SetNoteFromMidi(msg.NoteNumber(), msg.StatusMsg() == IMidiMsg::kNoteOn);
        });
#endif
       
    };
#endif
}

#if IPLUG_EDITOR
void NESting::ShowAutomationGraphs(bool visible)
{
    GetUI()->ForControlInGroup(GRAPHS_GROUP, [&](IControl& control) {
        control.Hide(!visible);
        });
}

#endif

#if IPLUG_DSP

void NESting::ProcessBlock(sample** inputs, sample** outputs, int nFrames)
{
    sample* faustInputs[] = { mInputBuffer.Get() };
    // Pre-zero output buffers
    zero_buffers(outputs, 2, nFrames);
    mSynth.ProcessBlock(faustInputs, outputs, 1, 2, nFrames);
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
    double vNorm = GetParam(paramIdx)->GetNormalized();
    double vReal = GetParam(paramIdx)->Value();

    switch (paramIdx)
    {
    case iParamUseAutomationGraphs:
        mSender.PushData(ISenderData<4>(kCtrlAutomationGraphs, { float(GetParam(paramIdx)->Bool()), 0, 0, 0 }));
        break;
    }

    // Update the parameter state for our voices as well.
    for (size_t i = 0; i < mSynth.NVoices(); i += 1) {
        auto voice = dynamic_cast<NESVoice*>(mSynth.GetVoice(i));
        
        switch (paramIdx) {
        case iParamGain:
            voice->mOwnerGain = vNorm;
            break;
        case iParamDuty:
            voice->mFaustSquare.SetParameterValueNormalised(0, vNorm);
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
    return false;
}

void NESting::SendControlMsgFromDelegate(int ctrlTag, int msgTag, int dataSize, const void* pData)
{
    // Call the superclass method.
    Plugin::SendControlMsgFromDelegate(ctrlTag, msgTag, dataSize, pData);

    // Most of my custom messages use an array of 4 floats for their data.
    const ISenderData<4>& fData = *static_cast<const ISenderData<4>*>(pData);
    if (ctrlTag == kCtrlAutomationGraphs)
    {
        ShowAutomationGraphs(bool(fData.vals[0]));
    }
}

void NESting::OnIdle()
{
  mScopeSender.TransmitData(*this);
  mSender.TransmitData(*this);
}
#endif
