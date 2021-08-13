cmake_minimum_required(VERSION 3.12)

set(CMAKE_XCODE_GENERATE_SCHEME true)
set(CMAKE_OSX_DEPLOYMENT_TARGET "10.14" CACHE STRING "Minimum OS X deployment version")
set(JUCE_ENABLE_MODULE_SOURCE_GROUPS ON CACHE BOOL "" FORCE)
set(CMAKE_OSX_ARCHITECTURES "x86_64" CACHE STRING "" FORCE)
set_property(GLOBAL PROPERTY USE_FOLDERS YES)

project(KILLING_ME_SOFTLY_WITH_HIS_DSP VERSION 0.0.1)

add_subdirectory(Dependencies/JUCE)
add_subdirectory(Dependencies/rubberband)

find_library(RUBBERBAND NAMES rubberband)

juce_add_gui_app(JuceRealTimeStretch
    PRODUCT_NAME "Juce Real Time Stretch")

juce_generate_juce_header(JuceRealTimeStretch)

target_sources(JuceRealTimeStretch
    PRIVATE
        source/real_time/Main.cpp
        source/real_time/MainComponent.h
        source/real_time/MainComponent.cpp
        source/Allocators.h
        source/Allocators.cpp
        source/sysutils.h
        source/VectorOps.h)

target_compile_definitions(JuceRealTimeStretch
    PRIVATE
        # JUCE_WEB_BROWSER and JUCE_USE_CURL would be on by default, but you might not need them.
        JUCE_WEB_BROWSER=0  # If you remove this, add `NEEDS_WEB_BROWSER TRUE` to the `juce_add_gui_app` call
        JUCE_USE_CURL=0     # If you remove this, add `NEEDS_CURL TRUE` to the `juce_add_gui_app` call
        JUCE_APPLICATION_NAME_STRING="$<TARGET_PROPERTY:JuceRealTimeStretch,JUCE_PRODUCT_NAME>"
        JUCE_APPLICATION_VERSION_STRING="$<TARGET_PROPERTY:JuceRealTimeStretch,JUCE_VERSION>")

target_include_directories(JuceRealTimeStretch PRIVATE /usr/local/include/rubberband)

target_link_libraries(JuceRealTimeStretch
    PRIVATE
        juce::juce_audio_utils
        juce::juce_gui_extra
    PUBLIC
        juce::juce_recommended_config_flags
        juce::juce_recommended_lto_flags
        juce::juce_recommended_warning_flags
        ${RUBBERBAND})

juce_add_gui_app(JuceOfflineTimeStretch
    PRODUCT_NAME "Juce Offline Time Stretch")

juce_generate_juce_header(JuceOfflineTimeStretch)

target_sources(JuceOfflineTimeStretch
    PRIVATE
        source/offline/Main.cpp
        source/offline/MainComponent.h
        source/offline/MainComponent.cpp
        source/Allocators.h
        source/Allocators.cpp
        source/sysutils.h
        source/VectorOps.h)
        
target_compile_definitions(JuceOfflineTimeStretch
    PRIVATE
        # JUCE_WEB_BROWSER and JUCE_USE_CURL would be on by default, but you might not need them.
        JUCE_WEB_BROWSER=0  # If you remove this, add `NEEDS_WEB_BROWSER TRUE` to the `juce_add_gui_app` call
        JUCE_USE_CURL=0     # If you remove this, add `NEEDS_CURL TRUE` to the `juce_add_gui_app` call
        JUCE_APPLICATION_NAME_STRING="$<TARGET_PROPERTY:JuceOfflineTimeStretch,JUCE_PRODUCT_NAME>"
        JUCE_APPLICATION_VERSION_STRING="$<TARGET_PROPERTY:JuceOfflineTimeStretch,JUCE_VERSION>")

target_include_directories(JuceOfflineTimeStretch PRIVATE /usr/local/include/rubberband)

target_link_libraries(JuceOfflineTimeStretch
    PRIVATE
        juce::juce_audio_utils
        juce::juce_gui_extra
    PUBLIC
        juce::juce_recommended_config_flags
        juce::juce_recommended_lto_flags
        juce::juce_recommended_warning_flags
        ${RUBBERBAND})
