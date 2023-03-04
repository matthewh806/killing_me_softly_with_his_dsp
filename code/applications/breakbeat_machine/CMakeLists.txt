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
        ${CMAKE_SOURCE_DIR}/applications/breakbeat_machine/Main.cpp
        ${CMAKE_SOURCE_DIR}/applications/breakbeat_machine/BreakbeatMainWindow.h
        )

target_compile_definitions(BreakbeatMachine
    PRIVATE
        JUCE_APPLICATION_NAME_STRING="$<TARGET_PROPERTY:BreakbeatMachine,JUCE_PRODUCT_NAME>"
        JUCE_APPLICATION_VERSION_STRING="$<TARGET_PROPERTY:BreakbeatMachine,JUCE_VERSION>")

target_include_directories(BreakbeatMachine PRIVATE ${CMAKE_SOURCE_DIR}/dependencies/rubberband/rubberband)
target_include_directories(BreakbeatMachine PRIVATE /usr/local/include/aubio)
target_include_directories(BreakbeatMachine PRIVATE /usr/include/aubio)

target_link_libraries(BreakbeatMachine
    PRIVATE
        Shared_VST_Target
    PUBLIC
        rubberband
        aubio)
