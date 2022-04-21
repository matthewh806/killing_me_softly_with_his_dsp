juce_add_gui_app(FFT
    PRODUCT_NAME "FFT Project"
    MICROPHONE_PERMISSION_ENABLED   TRUE)

juce_generate_juce_header(FFT)

set(FFTApplicationSources
    ${UISources}
    ${CMAKE_SOURCE_DIR}/playground/fft/Main.cpp
    ${CMAKE_SOURCE_DIR}/playground/fft/MainComponent.h
    ${CMAKE_SOURCE_DIR}/playground/fft/MainComponent.cpp
)
source_group("Source" FILES ${FFTApplicationSources})

target_sources(FFT PRIVATE
    ${FFTApplicationSources}
)

target_compile_definitions(FFT PRIVATE
    # JUCE_WEB_BROWSER and JUCE_USE_CURL would be on by default, but you might not need them.
    JUCE_WEB_BROWSER=0  # If you remove this, add `NEEDS_WEB_BROWSER TRUE` to the `juce_add_gui_app` call
    JUCE_USE_CURL=0     # If you remove this, add `NEEDS_CURL TRUE` to the `juce_add_gui_app` call
    JUCE_APPLICATION_NAME_STRING="$<TARGET_PROPERTY:FFT,JUCE_PRODUCT_NAME>"
    JUCE_APPLICATION_VERSION_STRING="$<TARGET_PROPERTY:FFT,JUCE_VERSION>")

target_link_libraries(FFT
PRIVATE
    juce::juce_audio_utils
    juce::juce_gui_extra
    juce::juce_dsp
PUBLIC
    juce::juce_recommended_config_flags
    juce::juce_recommended_lto_flags
    juce::juce_recommended_warning_flags)
