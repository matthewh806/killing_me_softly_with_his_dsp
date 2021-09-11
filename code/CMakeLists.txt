cmake_minimum_required(VERSION 3.12)

set(CMAKE_XCODE_GENERATE_SCHEME true)
set(CMAKE_OSX_DEPLOYMENT_TARGET "10.14" CACHE STRING "Minimum OS X deployment version")
set(JUCE_ENABLE_MODULE_SOURCE_GROUPS ON CACHE BOOL "" FORCE)
set(CMAKE_OSX_ARCHITECTURES "x86_64" CACHE STRING "" FORCE)
 set(CMAKE_CXX_STANDARD 17)
set_property(GLOBAL PROPERTY USE_FOLDERS YES)

project(KILLING_ME_SOFTLY_WITH_HIS_DSP VERSION 0.0.1)

add_subdirectory(dependencies/JUCE)
add_subdirectory(dependencies/rubberband)

set(CoreSources
    ${CMAKE_SOURCE_DIR}/core/Allocators.h
    ${CMAKE_SOURCE_DIR}/core/Allocators.cpp
    ${CMAKE_SOURCE_DIR}/core/CircularBuffer.h
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

set(UtilsSources
    ${CMAKE_SOURCE_DIR}/utils/FileRecorder.h
    ${CMAKE_SOURCE_DIR}/utils/FileRecorder.cpp
)
source_group("Source/UtilsSources" FILES ${UtilsSources})

add_subdirectory(applications)
