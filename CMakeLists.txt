cmake_minimum_required(VERSION 3.12)

project(JUCE_STRETCH VERSION 0.0.1)

find_package(JUCE CONFIG REQUIRED)

find_library(RUBBERBAND NAMES rubberband)

juce_add_gui_app(JuceStretch
    PRODUCT_NAME "Juce Stretch")

juce_generate_juce_header(JuceStretch)

target_sources(JuceStretch
    PRIVATE
        source/Main.cpp
        source/MainComponent.h
        source/MainComponent.cpp
        source/Allocators.h
        source/Allocators.cpp
        source/sysutils.h
        source/VectorOps.h)

target_compile_definitions(JuceStretch
    PRIVATE
        # JUCE_WEB_BROWSER and JUCE_USE_CURL would be on by default, but you might not need them.
        JUCE_WEB_BROWSER=0  # If you remove this, add `NEEDS_WEB_BROWSER TRUE` to the `juce_add_gui_app` call
        JUCE_USE_CURL=0     # If you remove this, add `NEEDS_CURL TRUE` to the `juce_add_gui_app` call
        JUCE_APPLICATION_NAME_STRING="$<TARGET_PROPERTY:JuceStretch,JUCE_PRODUCT_NAME>"
        JUCE_APPLICATION_VERSION_STRING="$<TARGET_PROPERTY:JuceStretch,JUCE_VERSION>")

target_include_directories(JuceStretch PRIVATE /usr/local/include/rubberband)

target_link_libraries(JuceStretch
    PRIVATE
        juce::juce_audio_utils
        juce::juce_gui_extra
    PUBLIC
        juce::juce_recommended_config_flags
        juce::juce_recommended_lto_flags
        juce::juce_recommended_warning_flags
        ${RUBBERBAND})
