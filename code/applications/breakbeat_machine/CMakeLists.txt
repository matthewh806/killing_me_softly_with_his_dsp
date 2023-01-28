juce_add_gui_app(BreakbeatMachine
  PRODUCT_NAME "Breakbeat Machine")

juce_generate_juce_header(BreakbeatMachine)

target_sources(BreakbeatMachine
    PRIVATE
        ${CoreSources}
        ${DspSources}
        ${AnalysisSources}
        ${UISources}
        ${UtilsSources}
        ${CMAKE_SOURCE_DIR}/applications/breakbeat_machine/SampleManager.h
        ${CMAKE_SOURCE_DIR}/applications/breakbeat_machine/SampleManager.cpp
        ${CMAKE_SOURCE_DIR}/applications/breakbeat_machine/SliceManager.h
        ${CMAKE_SOURCE_DIR}/applications/breakbeat_machine/SliceManager.cpp
        ${CMAKE_SOURCE_DIR}/applications/breakbeat_machine/BreakbeatAudioSource.h
        ${CMAKE_SOURCE_DIR}/applications/breakbeat_machine/BreakbeatAudioSource.cpp
        ${CMAKE_SOURCE_DIR}/applications/breakbeat_machine/BreakbeatMaker.h
        ${CMAKE_SOURCE_DIR}/applications/breakbeat_machine/BreakbeatMaker.cpp
        ${CMAKE_SOURCE_DIR}/applications/breakbeat_machine/SliceExporter.h
        ${CMAKE_SOURCE_DIR}/applications/breakbeat_machine/SliceExporter.cpp
        ${CMAKE_SOURCE_DIR}/applications/breakbeat_machine/BreakbeatWaveform.h
        ${CMAKE_SOURCE_DIR}/applications/breakbeat_machine/BreakbeatWaveform.cpp
        ${CMAKE_SOURCE_DIR}/applications/breakbeat_machine/Main.cpp)

target_compile_definitions(BreakbeatMachine
    PRIVATE
        # JUCE_WEB_BROWSER and JUCE_USE_CURL would be on by default, but you might not need them.
        JUCE_WEB_BROWSER=0  # If you remove this, add `NEEDS_WEB_BROWSER TRUE` to the `juce_add_gui_app` call
        JUCE_USE_CURL=0     # If you remove this, add `NEEDS_CURL TRUE` to the `juce_add_gui_app` call
        JUCE_APPLICATION_NAME_STRING="$<TARGET_PROPERTY:BreakbeatMachine,JUCE_PRODUCT_NAME>"
        JUCE_APPLICATION_VERSION_STRING="$<TARGET_PROPERTY:BreakbeatMachine,JUCE_VERSION>")

target_include_directories(BreakbeatMachine PRIVATE ${CMAKE_SOURCE_DIR}/dependencies/rubberband/rubberband)
target_include_directories(BreakbeatMachine PRIVATE /usr/local/include/aubio)
target_include_directories(BreakbeatMachine PRIVATE /usr/include/aubio)

target_link_libraries(BreakbeatMachine
    PRIVATE
        juce::juce_audio_utils
        juce::juce_gui_extra
    PUBLIC
        juce::juce_recommended_config_flags
        juce::juce_recommended_lto_flags
        juce::juce_recommended_warning_flags
        rubberband
        ${AUBIO})
