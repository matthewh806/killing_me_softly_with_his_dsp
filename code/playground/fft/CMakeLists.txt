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
    JUCE_APPLICATION_NAME_STRING="$<TARGET_PROPERTY:FFTGUI,JUCE_PRODUCT_NAME>"
    JUCE_APPLICATION_VERSION_STRING="$<TARGET_PROPERTY:FFTGUI,JUCE_VERSION>")

target_link_libraries(FFTGUI PRIVATE Shared_VST_Target)

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
    JUCE_APPLICATION_NAME_STRING="$<TARGET_PROPERTY:FFTCLI,JUCE_PRODUCT_NAME>"
    JUCE_APPLICATION_VERSION_STRING="$<TARGET_PROPERTY:FFTCLI,JUCE_VERSION>")

target_link_libraries(FFTCLI PRIVATE Shared_VST_Target)
