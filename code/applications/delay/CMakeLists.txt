juce_add_plugin(SimpleDelay
    PRODUCT_NAME "Simple Delay"
    MICROPHONE_PERMISSION_ENABLED   TRUE
    FORMATS Standalone VST3)

juce_generate_juce_header(SimpleDelay)

set(SimpleDelayApplicationSources
    ${UISources}
)
source_group("Source/ApplicationSources" FILES ${SimpleDelayApplicationSources})

target_sources(SimpleDelay PRIVATE
    ${CMAKE_SOURCE_DIR}/dsp/processors/SimpleDelayProcessor.h
    ${CMAKE_SOURCE_DIR}/dsp/processors/SimpleDelayProcessor.cpp
    ${SimpleDelayApplicationSources}
)

target_compile_definitions(SimpleDelay PRIVATE
    JUCE_APPLICATION_NAME_STRING="$<TARGET_PROPERTY:SimpleDelay,JUCE_PRODUCT_NAME>"
    JUCE_APPLICATION_VERSION_STRING="$<TARGET_PROPERTY:SimpleDelay,JUCE_VERSION>")

target_link_libraries(SimpleDelay PRIVATE Shared_VST_Target)
