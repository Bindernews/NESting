# IPLUG2_ROOT should point to the top level IPLUG2 folder from the project folder
# By default, that is three directories up from /Examples/NESting/config
IPLUG2_ROOT = ..\..\iPlug2

include ..\..\iPlug2/common-web.mk

SRC += $(PROJECT_ROOT)/NESting.cpp

# WAM_SRC +=

WAM_CFLAGS += -DFAUST_COMPILED -I$(ROOT)/Dependencies/Build/mac/include

WEB_CFLAGS += -DIGRAPHICS_NANOVG -DIGRAPHICS_GLES2 -DFAUST_COMPILED

WAM_LDFLAGS += -O3 -s EXPORT_NAME="'AudioWorkletGlobalScope.WAM.NESting'" -s ASSERTIONS=0

WEB_LDFLAGS += -O3 -s ASSERTIONS=0

WEB_LDFLAGS += $(NANOVG_LDFLAGS)
