juce_add_plugin(Pulsar
    PRODUCT_NAME "Pulsar"
    MICROPHONE_PERMISSION_ENABLED   TRUE
    NEEDS_MIDI_INPUT TRUE
    NEEDS_MIDI_OUTPUT TRUE
    IS_MIDI_EFFECT TRUE
    FORMATS VST3)

juce_generate_juce_header(Pulsar)

set(PulsarApplicationSources
    ${UISources}
    ${CMAKE_SOURCE_DIR}/applications/pulsar/PluginEditor.h
    ${CMAKE_SOURCE_DIR}/applications/pulsar/PluginEditor.cpp
    ${CMAKE_SOURCE_DIR}/applications/pulsar/PluginProcessor.h
    ${CMAKE_SOURCE_DIR}/applications/pulsar/PluginProcessor.cpp
    ${CMAKE_SOURCE_DIR}/applications/pulsar/Ball.h
    ${CMAKE_SOURCE_DIR}/applications/pulsar/Ball.cpp
    ${CMAKE_SOURCE_DIR}/applications/pulsar/Polygon.h
    ${CMAKE_SOURCE_DIR}/applications/pulsar/Polygon.cpp
    ${CMAKE_SOURCE_DIR}/applications/pulsar/PulsarWorld.h
    ${CMAKE_SOURCE_DIR}/applications/pulsar/PulsarWorld.cpp
    ${CMAKE_SOURCE_DIR}/applications/pulsar/NoteStrategy.h
    ${CMAKE_SOURCE_DIR}/applications/pulsar/NoteStrategy.cpp
    ${CMAKE_SOURCE_DIR}/applications/pulsar/Utils.h
    ${CMAKE_SOURCE_DIR}/applications/pulsar/Utils.cpp
)
source_group("Source/ApplicationSources" FILES ${PulsarApplicationSources})

target_sources(Pulsar PRIVATE
  ${PulsarApplicationSources}
)

target_compile_definitions(Pulsar PRIVATE
    JUCE_VST3_CAN_REPLACE_VST2=0
    JUCE_APPLICATION_NAME_STRING="$<TARGET_PROPERTY:Pulsar,JUCE_PRODUCT_NAME>"
    JUCE_APPLICATION_VERSION_STRING="$<TARGET_PROPERTY:Pulsar,JUCE_VERSION>")

target_link_libraries(Pulsar
PUBLIC
    Shared_VST_Target
    juce::juce_box2d)
