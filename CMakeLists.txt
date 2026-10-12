cmake_minimum_required(VERSION 3.20)
project(FlowAutoPresetFollower VERSION 1.0.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Point CMake to the cloned JUCE directory
if(EXISTS "C:/JUCE")
    add_subdirectory("C:/JUCE" ${CMAKE_BINARY_DIR}/JUCE)
elseif(EXISTS "/workspaces/JUCE")
    add_subdirectory("/workspaces/JUCE" ${CMAKE_BINARY_DIR}/JUCE)
endif()

juce_add_plugin(FlowAutoPresetFollower
    COMPANY_NAME "FlowMusic"
    PLUGIN_MANUFACTURER_CODE Flow
    PLUGIN_CODE Fapf
    FORMATS VST3 Standalone
    PRODUCT_NAME "Flow Auto-Preset Follower"
    IS_SYNTH FALSE
    NEEDS_MIDI_INPUT TRUE
    NEEDS_MIDI_OUTPUT TRUE
    IS_MIDI_EFFECT FALSE
    EDITOR_WANTS_KEYBOARD_FOCUS TRUE
)

target_sources(FlowAutoPresetFollower PRIVATE
    Source/PluginProcessor.cpp
    Source/PluginProcessor.h
    Source/PluginEditor.cpp
    Source/PluginEditor.h
)

target_compile_definitions(FlowAutoPresetFollower PRIVATE
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0
    JUCE_VST3_CAN_REPLACE_VST2=0
)

target_link_libraries(FlowAutoPresetFollower PRIVATE
    juce::juce_audio_utils
    juce::juce_dsp
    juce::juce_gui_extra
)