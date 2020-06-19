#include "NESting.h"
#include "IPlug_include_in_plug_src.h"
#include "math_utils.h"
#include "gui/ControlBarGraph.h"

#define NOISE_MODE_LIST "32767 Steps", "93 Steps"
#define WAVE_SHAPE_LIST "Square", "Triangle", "Noise", "DPCM"


#define DUTY_CYCLE_STOPS LabelPoint(0.00f, " 6%"), LabelPoint(0.25f, "12%"), LabelPoint(0.50f, "25%"),\
    LabelPoint(0.75f, "50%"), LabelPoint(1.00f, "75%"),
#define VOLUME_STOPS LabelPoint(0.0f, " 0%"), LabelPoint(0.5f, "50%"), LabelPoint(1.0f, "100%"),


NESting::NESting(const InstanceInfo& info)
: Plugin(info, MakeConfig(kNumParams, 1))
{
    GetParam(iParamGain)->InitGain("Gain");
    GetParam(iParamDuty)->InitInt("Duty Cycle", 2, 0, 4);
    GetParam(iParamShape)->InitEnum("Wave Shape", 0, { WAVE_SHAPE_LIST });
    GetParam(iParamNoiseMode)->InitEnum("Noise Mode", 0, { NOISE_MODE_LIST });
    GetParam(iParamVolumeSteps)->InitDouble("Volume - Steps", 8, 4, 64, 4);
    GetParam(iParamVolumeLoopPoint)->InitDouble("Volume - Loop Point", 1, 0, 1, 0.1);

#if IPLUG_DSP
    mSynth.AddVoice(new NESVoice(*this), 0);
#endif
  
#if IPLUG_EDITOR
    mMakeGraphicsFunc = [&]() {
        return MakeGraphics(*this, PLUG_WIDTH, PLUG_HEIGHT, PLUG_FPS, GetScaleForScreen(PLUG_HEIGHT));
    };

    mLayoutFunc = [&](IGraphics* pGraphics) {
        pGraphics->EnableMouseOver(true);
        pGraphics->EnableMultiTouch(true);
        pGraphics->SetLayoutOnResize(true);

        IRECT b = pGraphics->GetBounds().GetPadded(-20);

        IRECT knobs = b.GetFromTop(100.);
        IRECT viz = b.GetAltered(0., 100., 0., -210.);
        pGraphics->AttachCornerResizer(EUIResizerMode::Scale);
        pGraphics->LoadFont("Roboto-Regular", ROBOTO_FN);

        for (int i = 0; i < 4; i++) {
            pGraphics->AttachControl(new IVKnobControl(knobs.GetGridCell(i, 1, 5), i, "", DEFAULT_STYLE, false, false));
        }

        pGraphics->AttachControl(new IVLabelControl(b.GetPadded(-10).GetFromTLHC(40, 40), "MIDI:"), kCtrlStatus);

        pGraphics->AttachControl(new IVSwitchControl(b.GetFromTRHC(100.f, 80.f), iParamShape));
    
        pGraphics->AttachPanelBackground(COLOR_GRAY);
        pGraphics->AttachControl(new IVScopeControl<2>(viz, "", DEFAULT_STYLE.WithColor(kBG, COLOR_BLACK).WithColor(kFG, COLOR_GREEN)), kCtrlTagScope);

        IRECT graphBounds = b.GetFromBottom(200);
        auto barGraph = new ControlBarGraph(graphBounds);
        barGraph->SetStops({ DUTY_CYCLE_STOPS });
        barGraph->SetSteps(16);
        pGraphics->AttachControl(barGraph);

#ifdef APP_API
        IRECT keyboardBounds = b.GetFromBottom(200);
        pGraphics->AttachControl(new IVKeyboardControl(keyboardBounds), kCtrlTagKeyboard);
        pGraphics->SetQwertyMidiKeyHandlerFunc([pGraphics](const IMidiMsg& msg) {
            dynamic_cast<IVKeyboardControl*>(pGraphics->GetControlWithTag(kCtrlTagKeyboard))->SetNoteFromMidi(msg.NoteNumber(), msg.StatusMsg() == IMidiMsg::kNoteOn);
        });
#endif
       
    };
#endif
}

#if IPLUG_EDITOR
void NESting::ShowAutomationGraphs(bool visible)
{
    auto ui = GetUI();

    auto graphs = ui->GetControlWithTag(kCtrlAutomationGraphs);
    // Don't change state if we don't need to
    if ((graphs != nullptr) == visible) {
        return;
    }
    if (visible) {
        // Build automation graph panel
        IRECT b = ui->GetBounds().GetPadded(-20).GetFromBottom(AUTOMATION_PANEL_HEIGHT);
        auto panel = new IVPanelControl(b);
        
        auto buildGraphPanel = [](const IRECT& b, IColor color, int paramSteps, int paramLoop, std::initializer_list<LabelPoint> stops) {
            auto panel = new IVPanelControl(b);
            auto graph = new ControlBarGraph(b.GetFromTop(100));
            graph->iStyle.barColor = color;
            graph->SetStops(stops);
            panel->AttachIControl(graph, "");

            IRECT knobArea = b.GetPadded(0, -110, 0, 0);
            panel->AttachIControl(new IVKnobControl(knobArea.GetGridCell(0, 1, 4), paramSteps, "Steps"), "");
            panel->AttachIControl(new IVKnobControl(knobArea.GetGridCell(1, 1, 4), paramLoop, "Loop Point"), "");

            return panel;
        };

        // Volume
        IRECT b2 = b.GetGridCell(0, 2, 2).GetPadded(-10);
        panel->AttachIControl(buildGraphPanel(b2, COLOR_RED, iParamVolumeSteps, iParamVolumeLoopPoint,
            { VOLUME_STOPS }), "");
    }
    else {
        ui->RemoveControlWithTag(kCtrlAutomationGraphs);
    }
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

void NESting::OnIdle()
{
  mScopeSender.TransmitData(*this);
}
#endif
