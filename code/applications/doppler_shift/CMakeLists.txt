juce_add_plugin(DopplerShift
    PRODUCT_NAME "Doppler Shift"
    MICROPHONE_PERMISSION_ENABLED   TRUE
    FORMATS VST3)

juce_generate_juce_header(DopplerShift)

set(DopplerShiftSources
    ${UISources}
    ${CMAKE_SOURCE_DIR}/applications/doppler_shift/DopplerShiftPlugin.h
    ${CMAKE_SOURCE_DIR}/applications/doppler_shift/DopplerShiftPlugin.cpp
)
source_group("Source/ApplicationSources" FILES ${DopplerShiftSources})

target_sources(DopplerShift PRIVATE
  ${DopplerShiftSources}
)

target_compile_definitions(DopplerShift PRIVATE
    # JUCE_WEB_BROWSER and JUCE_USE_CURL would be on by default, but you might not need them.
    JUCE_WEB_BROWSER=0  # If you remove this, add `NEEDS_WEB_BROWSER TRUE` to the `juce_add_gui_app` call
    JUCE_USE_CURL=0     # If you remove this, add `NEEDS_CURL TRUE` to the `juce_add_gui_app` call
    JUCE_VST3_CAN_REPLACE_VST2=0
    JUCE_APPLICATION_NAME_STRING="$<TARGET_PROPERTY:DopplerShift,JUCE_PRODUCT_NAME>"
    JUCE_APPLICATION_VERSION_STRING="$<TARGET_PROPERTY:DopplerShift,JUCE_VERSION>")

target_link_libraries(DopplerShift
PRIVATE
    juce::juce_audio_utils
    juce::juce_gui_extra
    juce::juce_dsp
PUBLIC
    juce::juce_recommended_config_flags
    juce::juce_recommended_lto_flags
    juce::juce_recommended_warning_flags)
