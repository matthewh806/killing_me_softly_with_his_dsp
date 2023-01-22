set(PRINT_RUBBERBAND_ERRORS false STRING "Enables the stderr of the rubberband processor to be printed")
set(PRINT_DOPPLER_DEBUG false STRING "Enables the logging to stdout of the Doppler related code")

juce_add_plugin(DopplerShift
    PRODUCT_NAME "Doppler Shift"
    MICROPHONE_PERMISSION_ENABLED   TRUE
    FORMATS VST3)

juce_generate_juce_header(DopplerShift)

set(DopplerShiftSources
    ${UISources}
    ${CoreSources}
    ${CMAKE_SOURCE_DIR}/applications/doppler_shift/DopplerShiftPlugin.h
    ${CMAKE_SOURCE_DIR}/applications/doppler_shift/DopplerShiftPlugin.cpp
    ${CMAKE_SOURCE_DIR}/applications/doppler_shift/DopplerShiftPluginEditor.h
    ${CMAKE_SOURCE_DIR}/applications/doppler_shift/RubberbandPitchShifter.h
    ${CMAKE_SOURCE_DIR}/applications/doppler_shift/RubberbandPitchShifter.cpp
)
source_group("Source/ApplicationSources" FILES ${DopplerShiftSources})

target_sources(DopplerShift PRIVATE
  ${DopplerShiftSources}
)

target_compile_definitions(DopplerShift PRIVATE
    JUCE_APPLICATION_NAME_STRING="$<TARGET_PROPERTY:DopplerShift,JUCE_PRODUCT_NAME>"
    JUCE_APPLICATION_VERSION_STRING="$<TARGET_PROPERTY:DopplerShift,JUCE_VERSION>")

if(PRINT_RUBBERBAND_ERRORS)
    target_compile_definitions(DopplerShift PRIVATE PRINT_RUBBERBAND_ERRORS)
endif()

if(PRINT_DOPPLER_DEBUG)
    target_compile_definitions(DopplerShift PRIVATE PRINT_DOPPLER_DEBUG)
endif()

find_program(REAPER NAMES Reaper Reaper64)
if(REAPER)
    message(${REAPER})
    set_target_properties(DopplerShift PROPERTIES CMAKE_XCODE_SCHEME_EXECUTABLE ${REAPER})
endif()

target_link_libraries(DopplerShift PRIVATE Shared_VST_Target rubberband)
