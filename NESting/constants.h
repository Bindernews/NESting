#pragma once

#define MAX_LFO_STEP_TIME_MS (512)
// Approx 0db in range [-70, 24]
#define DEFAULT_ENV_VOLUME (0.744f)
#define DEFAULT_ENV_DUTY (1.0f)
#define DEFAULT_ENV_PITCH (0.5f)
#define DEFAULT_ENV_FINE_PITCH (0.5f)
#define MAX_LFO_GRAPH_STEPS (64)
#define ADSR_MAX_TIME 1000

#define NOISE_MODE_LIST "32767 Steps", "93 Steps"
#define WAVE_SHAPE_LIST "Square", "Triangle", "Noise", "DPCM"
#define DUTY_CYCLE_LIST "6%", "12.5%", "25%", "50%", "75%",

#define TEMPO_LIST_SIZE (15)
extern const char* TEMPO_DIVISION_NAMES[15];
extern const double TEMPO_DIVISION_VALUES[15];

// GUI defines
#define MAIN_PANEL_HEIGHT (170.)
#define AUTOMATION_PANEL_HEIGHT (80.)
#define GRAPH_PANEL_HEIGHT (400.)
#define KEYBOARD_HEIGHT (200.)
#define PANEL_PADDING (20.f)
#define KEYBOARD_PANEL_HEIGHT (KEYBOARD_HEIGHT + 60.f)
#define GRAPHS_GROUP "automationGraph"
#define ENVELOPES_GROUP "envelopes"
