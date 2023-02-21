juce_add_gui_app(WaveformComponentTest
    PRODUCT_NAME "WaveformComponent Project"
    MICROPHONE_PERMISSION_ENABLED   TRUE)

juce_generate_juce_header(WaveformComponentTest)

set_target_properties(WaveformComponentTest PROPERTIES EXCLUDE_FROM_ALL TRUE)

set(WaveformComponentTestSources
    ${UISources}
    ${CMAKE_SOURCE_DIR}/core/ReferenceCountedBuffer.h
    ${CMAKE_SOURCE_DIR}/test/components/waveform/Main.cpp
    ${CMAKE_SOURCE_DIR}/test/components/waveform/MainComponent.h
    ${CMAKE_SOURCE_DIR}/test/components/waveform/MainComponent.cpp
)
source_group("Source/ApplicationSources" FILES ${WaveformComponentTestSources})

target_sources(WaveformComponentTest PRIVATE
    ${WaveformComponentTestSources}
)

target_compile_definitions(WaveformComponentTest PRIVATE
    # JUCE_WEB_BROWSER and JUCE_USE_CURL would be on by default, but you might not need them.
    JUCE_WEB_BROWSER=0  # If you remove this, add `NEEDS_WEB_BROWSER TRUE` to the `juce_add_gui_app` call
    JUCE_USE_CURL=0     # If you remove this, add `NEEDS_CURL TRUE` to the `juce_add_gui_app` call
    JUCE_APPLICATION_NAME_STRING="$<TARGET_PROPERTY:WaveformComponentTest,JUCE_PRODUCT_NAME>"
    JUCE_APPLICATION_VERSION_STRING="$<TARGET_PROPERTY:WaveformComponentTest,JUCE_VERSION>")

target_link_libraries(WaveformComponentTest
PRIVATE
    juce::juce_audio_utils
    juce::juce_gui_extra
PUBLIC
    juce::juce_recommended_config_flags
    juce::juce_recommended_lto_flags
    juce::juce_recommended_warning_flags)
