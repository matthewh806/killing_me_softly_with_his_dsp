juce_add_gui_app(SimpleDelay
    PRODUCT_NAME "Simple Delay"
    MICROPHONE_PERMISSION_ENABLED   TRUE)

juce_generate_juce_header(SimpleDelay)

set(SimpleDelayApplicationSources
    ${UISources}
    ${CMAKE_SOURCE_DIR}/applications/delay/Main.cpp
    ${CMAKE_SOURCE_DIR}/applications/delay/MainComponent.h
    ${CMAKE_SOURCE_DIR}/applications/delay/MainComponent.cpp
)
source_group("Source/ApplicationSources" FILES ${SimpleDelayApplicationSources})

target_sources(SimpleDelay PRIVATE
    ${CMAKE_SOURCE_DIR}/dsp/processors/SimpleDelayProcessor.h
    ${CMAKE_SOURCE_DIR}/dsp/processors/SimpleDelayProcessor.cpp
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
