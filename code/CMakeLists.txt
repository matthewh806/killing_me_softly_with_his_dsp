cmake_minimum_required(VERSION 3.19)

set(CMAKE_XCODE_GENERATE_SCHEME true)
set(CMAKE_OSX_DEPLOYMENT_TARGET "10.14" CACHE STRING "Minimum OS X deployment version")
set(JUCE_ENABLE_MODULE_SOURCE_GROUPS ON CACHE BOOL "" FORCE)
set(CMAKE_OSX_ARCHITECTURES "x86_64" CACHE STRING "" FORCE)
set(CMAKE_CXX_STANDARD 17)
set_property(GLOBAL PROPERTY USE_FOLDERS YES)
set(JUCE_COPY_PLUGIN_AFTER_BUILD ON)
set(SMTG_RUN_VST_VALIDATOR OFF)

project(KILLING_ME_SOFTLY_WITH_HIS_DSP VERSION 0.0.2)

add_subdirectory(dependencies/JUCE)
add_subdirectory(dependencies/rubberband)
add_subdirectory(dependencies/vst3sdk)

if(UNIX AND NOT APPLE)
    set_property(TARGET rubberband PROPERTY CMAKE_POSITION_INDEPENDENT_CODE TRUE)
endif()

find_library(AUBIO NAMES aubio)

set(CoreSources
    ${CMAKE_SOURCE_DIR}/core/Allocators.h
    ${CMAKE_SOURCE_DIR}/core/Allocators.cpp
    ${CMAKE_SOURCE_DIR}/core/CircularBuffer.h
    ${CMAKE_SOURCE_DIR}/core/ReferenceCountedBuffer.h
    ${CMAKE_SOURCE_DIR}/core/ReferenceCountedForwardAndReverseBuffer.h
    ${CMAKE_SOURCE_DIR}/core/ReferenceCountedForwardAndReverseBuffer.cpp
    ${CMAKE_SOURCE_DIR}/core/RingBuffer.h
    ${CMAKE_SOURCE_DIR}/core/VectorOps.h
    ${CMAKE_SOURCE_DIR}/core/sysutils.h
)
source_group("Source/CoreSources" FILES ${CoreSources})

set(UISources
    ${CMAKE_SOURCE_DIR}/ui/CustomLookAndFeel.h
    ${CMAKE_SOURCE_DIR}/ui/CustomLookAndFeel.cpp
    ${CMAKE_SOURCE_DIR}/ui/MainWindow.h
)
source_group("Source/UISources" FILES ${UISources})

set(SynthSources
    ${CMAKE_SOURCE_DIR}/dsp/synthesis/granular/Grain.h
    ${CMAKE_SOURCE_DIR}/dsp/synthesis/granular/Grain.cpp
    ${CMAKE_SOURCE_DIR}/dsp/synthesis/granular/Source.h
    ${CMAKE_SOURCE_DIR}/dsp/synthesis/granular/Source.cpp
    ${CMAKE_SOURCE_DIR}/dsp/synthesis/granular/Scheduler.h
    ${CMAKE_SOURCE_DIR}/dsp/synthesis/granular/Scheduler.cpp
    ${CMAKE_SOURCE_DIR}/dsp/synthesis/granular/SequenceStrategy.h
    ${CMAKE_SOURCE_DIR}/dsp/synthesis/granular/SequenceStrategy.cpp
)
source_group("Source/SynthSources" FILES ${UISources})

set(EnvelopSources
    ${CMAKE_SOURCE_DIR}/dsp/envelopes/Envelope.h
    ${CMAKE_SOURCE_DIR}/dsp/envelopes/Envelope.cpp
)
source_group("Source/EnvelopeSources" FILES ${UISources})

set(DspSources
    ${CMAKE_SOURCE_DIR}/dsp/OfflineStretcher.h
    ${CMAKE_SOURCE_DIR}/dsp/OfflineStretcher.cpp
    ${CMAKE_SOURCE_DIR}/dsp/processors/SimpleDelayProcessor.h
    ${CMAKE_SOURCE_DIR}/dsp/processors/SimpleDelayProcessor.cpp
    ${CMAKE_SOURCE_DIR}/dsp/processors/RealTimeStretchProcessor.h
    ${CMAKE_SOURCE_DIR}/dsp/processors/RealTimeStretchProcessor.cpp
    ${CMAKE_SOURCE_DIR}/dsp/processors/TemplateProcessor.h
    ${CMAKE_SOURCE_DIR}/dsp/processors/TemplateProcessor.cpp
)
source_group("Source/DspSources" FILES ${DspSources})

set(AnalysisSources
    ${CMAKE_SOURCE_DIR}/analysis/AudioAnalyser.h
    ${CMAKE_SOURCE_DIR}/analysis/AudioAnalyser.cpp
)
source_group("Source/AnalysisSources" FILES ${AnalysisSources})

set(UtilsSources
    ${CMAKE_SOURCE_DIR}/utils/FileRecorder.h
    ${CMAKE_SOURCE_DIR}/utils/FileRecorder.cpp
)
source_group("Source/UtilsSources" FILES ${UtilsSources})

add_subdirectory(applications)
add_subdirectory(playground)
