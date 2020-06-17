cmake_minimum_required(VERSION 3.11 FATAL_ERROR)
project(NESting VERSION 1.0.0 LANGUAGES CXX)

set(iPlug2_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../iPlug2)
find_package(iPlug2 REQUIRED)
find_package(Faust REQUIRED)
iplug2_configure("NESting"
    FAUST NANOVG GL2
)

#message("${CMAKE_GENERATOR}  ${CMAKE_GENERATOR_PLATFORM}  ${CMAKE_VS_PLATFORM_NAME}")

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

set(SRC_FILES
    config.h
    APUNoise.hpp
    FaustCode.hpp
    NESting.h
    NESting.cpp
    resources/main.rc
)

add_executable(NESting-app WIN32 ${SRC_FILES})
iplug2_configure_target(NESting-app app)

add_library(NESting-vst2 SHARED ${SRC_FILES})
iplug2_configure_target(NESting-vst2 vst2)

add_library(NESting-vst3 SHARED ${SRC_FILES})
iplug2_configure_target(NESting-vst3 vst3)

begin_faust_target("FaustDSP")
    add_faust_file("${CMAKE_CURRENT_SOURCE_DIR}/NESting.dsp" "Faust1")
end_faust_target()

foreach(target_name NESting-app NESting-vst2 NESting-vst3)
    target_include_directories(${target_name} PUBLIC 
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}> 
        ${CMAKE_CURRENT_SOURCE_DIR}/resources
        ${FaustDSP_INCLUDE_DIR}
        ${CMAKE_SOURCE_DIR}/resources
        ${CMAKE_SOURCE_DIR}/resources/fonts
    )
    source_group("src" ${SRC_FILES})
    target_compile_features(${target_name} PUBLIC cxx_std_14)
    add_dependencies(${target_name} "FaustDSP")
endforeach()
