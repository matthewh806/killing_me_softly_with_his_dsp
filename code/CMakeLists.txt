cmake_minimum_required(VERSION 3.12)

set(CMAKE_XCODE_GENERATE_SCHEME true)
set(CMAKE_OSX_DEPLOYMENT_TARGET "10.14" CACHE STRING "Minimum OS X deployment version")
set(JUCE_ENABLE_MODULE_SOURCE_GROUPS ON CACHE BOOL "" FORCE)
set(CMAKE_OSX_ARCHITECTURES "x86_64" CACHE STRING "" FORCE)
set_property(GLOBAL PROPERTY USE_FOLDERS YES)

project(KILLING_ME_SOFTLY_WITH_HIS_DSP VERSION 0.0.1)

add_subdirectory(dependencies/JUCE)
add_subdirectory(dependencies/rubberband)

find_library(RUBBERBAND NAMES rubberband)

set(CoreSources
    core/Allocators.h
    core/Allocators.cpp
    core/CircularBuffer.h
    core/ReferenceCountedForwardAndReverseBuffer.h
    core/ReferenceCountedForwardAndReverseBuffer.cpp
    core/RingBuffer.h
    core/VectorOps.h
    core/sysutils.h
)
source_group("Source/CoreSources" FILES ${CoreSources})

set(DspSources
    dsp/processors/SimpleDelayProcessor.h
    dsp/processors/SimpleDelayProcessor.cpp
    dsp/processors/RealTimeStretchProcessor.h
    dsp/processors/RealTimeStretchProcessor.cpp
)
source_group("Source/DspSources" FILES ${DspSources})

set(UtilsSources
    utils/FileRecorder.h
    utils/FileRecorder.cpp
)
source_group("Source/UtilsSources" FILES ${UtilsSources})

juce_add_gui_app(RealTimeStretch
    PRODUCT_NAME "Juce Real Time Stretch"
    MICROPHONE_PERMISSION_ENABLED   TRUE)

juce_generate_juce_header(RealTimeStretch)

target_sources(RealTimeStretch
    PRIVATE
        ${CoreSources}
        ${DspSources}
        applications/real_time/Main.cpp
        applications/real_time/MainComponent.h
        applications/real_time/MainComponent.cpp)

target_compile_definitions(RealTimeStretch
    PRIVATE
        # JUCE_WEB_BROWSER and JUCE_USE_CURL would be on by default, but you might not need them.
        JUCE_WEB_BROWSER=0  # If you remove this, add `NEEDS_WEB_BROWSER TRUE` to the `juce_add_gui_app` call
        JUCE_USE_CURL=0     # If you remove this, add `NEEDS_CURL TRUE` to the `juce_add_gui_app` call
        JUCE_APPLICATION_NAME_STRING="$<TARGET_PROPERTY:RealTimeStretch,JUCE_PRODUCT_NAME>"
        JUCE_APPLICATION_VERSION_STRING="$<TARGET_PROPERTY:RealTimeStretch,JUCE_VERSION>")

target_include_directories(RealTimeStretch PRIVATE dependencies/rubberband/rubberband)

target_link_libraries(RealTimeStretch
    PRIVATE
        juce::juce_audio_utils
        juce::juce_gui_extra
    PUBLIC
        juce::juce_recommended_config_flags
        juce::juce_recommended_lto_flags
        juce::juce_recommended_warning_flags
        ${RUBBERBAND})

juce_add_gui_app(OfflineTimeStretch
    PRODUCT_NAME "Juce Offline Time Stretch")

juce_generate_juce_header(OfflineTimeStretch)

target_sources(OfflineTimeStretch
    PRIVATE
        ${CoreSources}
        applications/offline/Main.cpp
        applications/offline/MainComponent.h
        applications/offline/MainComponent.cpp)

target_compile_definitions(OfflineTimeStretch
    PRIVATE
        # JUCE_WEB_BROWSER and JUCE_USE_CURL would be on by default, but you might not need them.
        JUCE_WEB_BROWSER=0  # If you remove this, add `NEEDS_WEB_BROWSER TRUE` to the `juce_add_gui_app` call
        JUCE_USE_CURL=0     # If you remove this, add `NEEDS_CURL TRUE` to the `juce_add_gui_app` call
        JUCE_APPLICATION_NAME_STRING="$<TARGET_PROPERTY:OfflineTimeStretch,JUCE_PRODUCT_NAME>"
        JUCE_APPLICATION_VERSION_STRING="$<TARGET_PROPERTY:OfflineTimeStretch,JUCE_VERSION>")

target_include_directories(OfflineTimeStretch PRIVATE dependencies/rubberband/rubberband)

target_link_libraries(OfflineTimeStretch
    PRIVATE
        juce::juce_audio_utils
        juce::juce_gui_extra
    PUBLIC
        juce::juce_recommended_config_flags
        juce::juce_recommended_lto_flags
        juce::juce_recommended_warning_flags
        ${RUBBERBAND})

juce_add_gui_app(SimpleDelay
    PRODUCT_NAME "Simple Delay"
    MICROPHONE_PERMISSION_ENABLED   TRUE)

juce_generate_juce_header(SimpleDelay)

set(SimpleDelayApplicationSources
    applications/delay/Main.cpp
    applications/delay/MainComponent.h
    applications/delay/MainComponent.cpp
)
source_group("Source/ApplicationSources" FILES ${SimpleDelayApplicationSources})

target_sources(SimpleDelay PRIVATE
    dsp/processors/SimpleDelayProcessor.h
    dsp/processors/SimpleDelayProcessor.cpp
    ${SimpleDelayApplicationSources}
)

target_compile_definitions(SimpleDelay PRIVATE
    # JUCE_WEB_BROWSER and JUCE_USE_CURL would be on by default, but you might not need them.
    JUCE_WEB_BROWSER=0  # If you remove this, add `NEEDS_WEB_BROWSER TRUE` to the `juce_add_gui_app` call
    JUCE_USE_CURL=0     # If you remove this, add `NEEDS_CURL TRUE` to the `juce_add_gui_app` call
    JUCE_APPLICATION_NAME_STRING="$<TARGET_PROPERTY:SimpleDelay,JUCE_PRODUCT_NAME>"
    JUCE_APPLICATION_VERSION_STRING="$<TARGET_PROPERTY:SimpleDelay,JUCE_VERSION>")

target_link_libraries(SimpleDelay
PRIVATE
    juce::juce_audio_utils
    juce::juce_gui_extra
PUBLIC
    juce::juce_recommended_config_flags
    juce::juce_recommended_lto_flags
    juce::juce_recommended_warning_flags)


juce_add_gui_app(BreakbeatMachine
  PRODUCT_NAME "Breakbeat Machine")

juce_generate_juce_header(BreakbeatMachine)

target_sources(BreakbeatMachine
    PRIVATE
        ${CoreSources}
        ${UtilsSources}
        applications/breakbeat_machine/BreakbeatAudioSource.h
        applications/breakbeat_machine/BreakbeatAudioSource.cpp
        applications/breakbeat_machine/BreakbeatMaker.h
        applications/breakbeat_machine/BreakbeatMaker.cpp
        applications/breakbeat_machine/SliceExporter.h
        applications/breakbeat_machine/SliceExporter.cpp
        applications/breakbeat_machine/Main.cpp)

target_compile_definitions(BreakbeatMachine
    PRIVATE
        # JUCE_WEB_BROWSER and JUCE_USE_CURL would be on by default, but you might not need them.
        JUCE_WEB_BROWSER=0  # If you remove this, add `NEEDS_WEB_BROWSER TRUE` to the `juce_add_gui_app` call
        JUCE_USE_CURL=0     # If you remove this, add `NEEDS_CURL TRUE` to the `juce_add_gui_app` call
        JUCE_APPLICATION_NAME_STRING="$<TARGET_PROPERTY:BreakbeatMachine,JUCE_PRODUCT_NAME>"
        JUCE_APPLICATION_VERSION_STRING="$<TARGET_PROPERTY:BreakbeatMachine,JUCE_VERSION>")

target_link_libraries(BreakbeatMachine
    PRIVATE
        juce::juce_audio_utils
        juce::juce_gui_extra
    PUBLIC
        juce::juce_recommended_config_flags
        juce::juce_recommended_lto_flags
        juce::juce_recommended_warning_flags)
