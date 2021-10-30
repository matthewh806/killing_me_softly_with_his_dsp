juce_add_plugin(Pulsar
    PRODUCT_NAME "Pulsar Project"
    MICROPHONE_PERMISSION_ENABLED   TRUE
    FORMATS Standalone)

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
    # JUCE_WEB_BROWSER and JUCE_USE_CURL would be on by default, but you might not need them.
    JUCE_WEB_BROWSER=0  # If you remove this, add `NEEDS_WEB_BROWSER TRUE` to the `juce_add_gui_app` call
    JUCE_USE_CURL=0     # If you remove this, add `NEEDS_CURL TRUE` to the `juce_add_gui_app` call
    JUCE_APPLICATION_NAME_STRING="$<TARGET_PROPERTY:Pulsar,JUCE_PRODUCT_NAME>"
    JUCE_APPLICATION_VERSION_STRING="$<TARGET_PROPERTY:Pulsar,JUCE_VERSION>")

target_link_libraries(Pulsar
PRIVATE
    juce::juce_audio_utils
    juce::juce_gui_extra
    juce::juce_box2d
PUBLIC
    juce::juce_recommended_config_flags
    juce::juce_recommended_lto_flags
    juce::juce_recommended_warning_flags)
