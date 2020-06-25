cmake_minimum_required(VERSION 3.11 FATAL_ERROR)
project(NESting VERSION 1.0.0 LANGUAGES CXX)

set(iPlug2_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../iPlug2)
find_package(iPlug2 REQUIRED)
find_package(Faust REQUIRED)

#message("${CMAKE_GENERATOR}  ${CMAKE_GENERATOR_PLATFORM}  ${CMAKE_CXX_COMPILER_ID}")


# Add the bson-cxx library
set(sdk "${CMAKE_SOURCE_DIR}/../bson-cxx/src")
set(_src
    "${sdk}/bson/base64.h" 
    "${sdk}/bson/base64.cpp"
    "${sdk}/bson/bson.cpp"
    "${sdk}/bson/bsonelement.h"
    "${sdk}/bson/bsonobj.h"
    "${sdk}/bson/bsonobjbuilder.h"
    "${sdk}/bson/bsonobjiterator.h"
    "${sdk}/bson/bsontypes.h"
    "${sdk}/bson/builder.h"
    "${sdk}/bson/endian.h"
    "${sdk}/bson/errorcodes.h"
    "${sdk}/bson/hex.h"
    "${sdk}/bson/hex.cpp"
    "${sdk}/bson/oid.h"
    "${sdk}/bson/ordering.h"
    "${sdk}/bson/parse_number.h"
    "${sdk}/bson/parse_number.cpp"
    "${sdk}/bson/status.h"
    "${sdk}/bson/status_with.h"
    "${sdk}/bson/string_data.h"
)
add_library(BsonCXX STATIC)
iplug2_target_add(BsonCXX PUBLIC
  INCLUDE ${sdk}
  SOURCE ${_src}
)
source_group(TREE ${sdk} PREFIX "" FILES ${_src})
unset(_src)


set_property(GLOBAL PROPERTY USE_FOLDERS ON)

set(sdk "${CMAKE_SOURCE_DIR}")

set(RES_FILES
  "${sdk}/resources/fonts/Roboto-Regular.ttf"
)

set(SRC_FILES
    config.h
    constants.h
    APUNoise.h
    APUNoise.cpp
    NESVoice.h
    NESVoice.cpp
    NESting.h
    NESting.cpp
    NESting_gui.cpp
    NESting_serialize.cpp
    LFOGraph.h
    LFOGraph.cpp
    slice.hpp
    math_utils.h
    gui/IVDropdownMenuControl.h
    gui/IVDropdownMenuControl.cpp
    gui/LFOGraphControl.h
    gui/LFOGraphControl.cpp
    presets/presets.h
    presets/presets.cpp
    ${RES_FILES}
)

# Have to do this first, so we can include these files in all other targets.
add_faust_target(FaustDSP
  "${CMAKE_CURRENT_SOURCE_DIR}/BlockSquare.dsp" "Square"
  "${CMAKE_CURRENT_SOURCE_DIR}/BlockTriangle.dsp" "Triangle"
)

# This declares a new interface library named _base which includes the include
# dirs we need, defines some globals, and then links to the various iPlug2
# libraries we're using. 
# We could use individual calls for this, but I find this function easier to read.
add_library(_base INTERFACE)
iplug2_target_add(_base INTERFACE
  INCLUDE 
    ${CMAKE_CURRENT_SOURCE_DIR} 
    ${FaustDSP_INCLUDE_DIR}
    ${CMAKE_SOURCE_DIR}/resources ${CMAKE_SOURCE_DIR}/resources/fonts
  DEFINE "SAMPLE_TYPE_FLOAT" "FAUST_COMPILED"
  LINK iPlug2_Synth iPlug2_Faust iPlug2_GL3 iPlug2_NANOVG BsonCXX
  DEPEND FaustDSP
)
if (CMAKE_CXX_COMPILER_ID MATCHES "MSVC")
  target_compile_options(_base INTERFACE "/openmp" "/std:c++17" "/MT$<$<CONFIG:Debug>:d>")
else()
  iplug2_target_add(_base INTERFACE DEFINE "NO_OPENMP")
endif()

add_executable(NESting-app WIN32 MACOSX_BUNDLE ${SRC_FILES})
iplug2_target_add(NESting-app PUBLIC LINK iPlug2_APP _base RESOURCE ${RES_FILES})
iplug2_configure_target(NESting-app app)

add_library(NESting-vst2 MODULE ${SRC_FILES})
iplug2_target_add(NESting-vst2 PUBLIC LINK iPlug2_VST2 _base RESOURCE ${RES_FILES})
iplug2_configure_target(NESting-vst2 vst2)

add_library(NESting-vst3 MODULE ${SRC_FILES})
iplug2_target_add(NESting-vst3 PUBLIC 
  DEFINE "VST3_NUM_CC_CHANS=0" 
  LINK iPlug2_VST3 _base 
  RESOURCE ${RES_FILES}
)
iplug2_configure_target(NESting-vst3 vst3)

source_group("Resources" FILES ${RES_FILES})

if (CMAKE_CXX_COMPILER_ID MATCHES "MSVC")
  set(REAPER_PATH "C:\\Program Files\\REAPER (x64)\\reaper.exe")
  string(CONFIGURE [=[<?xml version="1.0" encoding="utf-8"?>
<Project ToolsVersion="Current" xmlns="http://schemas.microsoft.com/developer/msbuild/2003">
  <PropertyGroup Condition="'$(Configuration)|$(Platform)'=='Debug|x64'">
    <LocalDebuggerCommand>${REAPER_PATH}</LocalDebuggerCommand>
    <DebuggerFlavor>WindowsLocalDebugger</DebuggerFlavor>
    <LocalDebuggerCommandArguments>${CMAKE_SOURCE_DIR}\NESting.RPP</LocalDebuggerCommandArguments>
  </PropertyGroup>
  <PropertyGroup Condition="'$(Configuration)|$(Platform)'=='Release|x64'">
    <LocalDebuggerCommand>${REAPER_PATH}</LocalDebuggerCommand>
    <DebuggerFlavor>WindowsLocalDebugger</DebuggerFlavor>
    <LocalDebuggerCommandArguments>${CMAKE_SOURCE_DIR}\NESting.RPP</LocalDebuggerCommandArguments>
  </PropertyGroup>
</Project>]=] _text)
  file(WRITE "${CMAKE_BINARY_DIR}/NESting-vst3.vcxproj.user" "${_text}")
  file(WRITE "${CMAKE_BINARY_DIR}/NESting-vst2.vcxproj.user" "${_text}")
endif()

