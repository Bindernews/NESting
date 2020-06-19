cmake_minimum_required(VERSION 3.11 FATAL_ERROR)
project(NESting VERSION 1.0.0 LANGUAGES CXX)

set(iPlug2_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../iPlug2)
find_package(iPlug2 REQUIRED)
find_package(Faust REQUIRED)

#message("${CMAKE_GENERATOR}  ${CMAKE_GENERATOR_PLATFORM}  ${CMAKE_VS_PLATFORM_NAME}")

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

set(SRC_FILES
    config.h
    APUNoise.h
    APUNoise.cpp
    NESVoice.h
    NESVoice.cpp
    NESting.h
    NESting.cpp
    resources/main.rc
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
iplug2_add_interface(_base
  INCLUDE 
    ${CMAKE_CURRENT_SOURCE_DIR} ${FaustDSP_INCLUDE_DIR} 
    ${CMAKE_SOURCE_DIR}/resources ${CMAKE_SOURCE_DIR}/resources/fonts
  DEFINE "SAMPLE_TYPE_FLOAT" "FAUST_COMPILED"
  LINK iPlug2_Synth iPlug2_Faust iPlug2_GL2 iPlug2_NANOVG
  DEPEND FaustDSP
)
target_compile_features(_base INTERFACE cxx_std_14)

add_executable(NESting-app WIN32 ${SRC_FILES})
target_link_libraries(NESting-app PUBLIC iPlug2_APP _base)
iplug2_configure_target(NESting-app app)

add_library(NESting-vst2 SHARED ${SRC_FILES})
target_link_libraries(NESting-vst2 PUBLIC iPlug2_VST2 _base)
iplug2_configure_target(NESting-vst2 vst2)

add_library(NESting-vst3 SHARED ${SRC_FILES})
target_link_libraries(NESting-vst3 PUBLIC iPlug2_VST3 _base)
iplug2_configure_target(NESting-vst3 vst3)

source_group("memes" ${SRC_FILES})

