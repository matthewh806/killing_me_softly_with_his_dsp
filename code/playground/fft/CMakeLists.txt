juce_add_gui_app(FFTGUI
    PRODUCT_NAME "FFT Project"
    MICROPHONE_PERMISSION_ENABLED   TRUE)

juce_generate_juce_header(FFTGUI)

set(FFTSources
    ${CMAKE_SOURCE_DIR}/playground/fft/FFTWrapper.h
    ${CMAKE_SOURCE_DIR}/playground/fft/FFTWrapper.cpp
)

set(FFTApplicationSources
    ${UISources}
    ${FFTSources}
    ${CMAKE_SOURCE_DIR}/playground/fft/Main.cpp
    ${CMAKE_SOURCE_DIR}/playground/fft/MainComponent.h
    ${CMAKE_SOURCE_DIR}/playground/fft/MainComponent.cpp
)
source_group("Source" FILES ${FFTApplicationSources})

target_sources(FFTGUI PRIVATE
    ${FFTApplicationSources}
)

target_compile_definitions(FFTGUI PRIVATE
    # JUCE_WEB_BROWSER and JUCE_USE_CURL would be on by default, but you might not need them.
    JUCE_WEB_BROWSER=0  # If you remove this, add `NEEDS_WEB_BROWSER TRUE` to the `juce_add_gui_app` call
    JUCE_USE_CURL=0     # If you remove this, add `NEEDS_CURL TRUE` to the `juce_add_gui_app` call
    JUCE_APPLICATION_NAME_STRING="$<TARGET_PROPERTY:FFTGUI,JUCE_PRODUCT_NAME>"
    JUCE_APPLICATION_VERSION_STRING="$<TARGET_PROPERTY:FFTGUI,JUCE_VERSION>")

target_link_libraries(FFTGUI
PRIVATE
    juce::juce_audio_utils
    juce::juce_gui_extra
    juce::juce_dsp
PUBLIC
    juce::juce_recommended_config_flags
    juce::juce_recommended_lto_flags
    juce::juce_recommended_warning_flags)

juce_add_console_app(FFTCLI
    PRODUCT_NAME "FFT CLI Project"
)

juce_generate_juce_header(FFTCLI)

set(FFTCLIApplicationSources
    ${FFTSources}
    ${CMAKE_SOURCE_DIR}/playground/fft/CLIMain.cpp
)
source_group("Source" FILES ${FFTCLIApplicationSources})

target_sources(FFTCLI PRIVATE
    ${FFTCLIApplicationSources}
)

target_compile_definitions(FFTCLI PRIVATE
    # JUCE_WEB_BROWSER and JUCE_USE_CURL would be on by default, but you might not need them.
    JUCE_WEB_BROWSER=0  # If you remove this, add `NEEDS_WEB_BROWSER TRUE` to the `juce_add_gui_app` call
    JUCE_USE_CURL=0     # If you remove this, add `NEEDS_CURL TRUE` to the `juce_add_gui_app` call
    JUCE_APPLICATION_NAME_STRING="$<TARGET_PROPERTY:FFTCLI,JUCE_PRODUCT_NAME>"
    JUCE_APPLICATION_VERSION_STRING="$<TARGET_PROPERTY:FFTCLI,JUCE_VERSION>")

target_link_libraries(FFTCLI
PRIVATE
    juce::juce_audio_utils
    juce::juce_dsp
PUBLIC
    juce::juce_recommended_config_flags
    juce::juce_recommended_lto_flags
    juce::juce_recommended_warning_flags)
