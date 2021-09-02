juce_add_gui_app(AudioDecay
    PRODUCT_NAME "Audio Decay"
    MICROPHONE_PERMISSION_ENABLED   TRUE)

juce_generate_juce_header(AudioDecay)

set(ApplicationSources
    ${UISources}
    ${CMAKE_SOURCE_DIR}/applications/audio_decay/Main.cpp
    ${CMAKE_SOURCE_DIR}/applications/audio_decay/MainComponent.h
    ${CMAKE_SOURCE_DIR}/applications/audio_decay/MainComponent.cpp
)
source_group("Source/ApplicationSources" FILES ${ApplicationSources})

target_sources(AudioDecay PRIVATE
    ${CMAKE_SOURCE_DIR}/dsp/processors/AudioDecayProcessor.h
    ${CMAKE_SOURCE_DIR}/dsp/processors/AudioDecayProcessor.cpp
    ${ApplicationSources}
)

target_compile_definitions(AudioDecay PRIVATE
    # JUCE_WEB_BROWSER and JUCE_USE_CURL would be on by default, but you might not need them.
    JUCE_WEB_BROWSER=0  # If you remove this, add `NEEDS_WEB_BROWSER TRUE` to the `juce_add_gui_app` call
    JUCE_USE_CURL=0     # If you remove this, add `NEEDS_CURL TRUE` to the `juce_add_gui_app` call
    JUCE_APPLICATION_NAME_STRING="$<TARGET_PROPERTY:AudioDecay,JUCE_PRODUCT_NAME>"
    JUCE_APPLICATION_VERSION_STRING="$<TARGET_PROPERTY:AudioDecay,JUCE_VERSION>")

target_link_libraries(AudioDecay
PRIVATE
    juce::juce_audio_utils
    juce::juce_gui_extra
PUBLIC
    juce::juce_recommended_config_flags
    juce::juce_recommended_lto_flags
    juce::juce_recommended_warning_flags)
