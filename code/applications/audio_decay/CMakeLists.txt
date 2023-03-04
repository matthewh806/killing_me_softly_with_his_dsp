juce_add_plugin(AudioDecay
    PRODUCT_NAME "Smash My Bits Up"
    MICROPHONE_PERMISSION_ENABLED   TRUE
    FORMATS VST3)

juce_generate_juce_header(AudioDecay)

set(ApplicationSources
    ${UISources}
)
source_group("Source/ApplicationSources" FILES ${ApplicationSources})

target_sources(AudioDecay PRIVATE
    ${CMAKE_SOURCE_DIR}/dsp/processors/AudioDecayProcessor.h
    ${CMAKE_SOURCE_DIR}/dsp/processors/AudioDecayProcessor.cpp
    ${ApplicationSources}
)

target_compile_definitions(AudioDecay PRIVATE
    JUCE_VST3_CAN_REPLACE_VST2=0
    JUCE_APPLICATION_NAME_STRING="$<TARGET_PROPERTY:AudioDecay,JUCE_PRODUCT_NAME>"
    JUCE_APPLICATION_VERSION_STRING="$<TARGET_PROPERTY:AudioDecay,JUCE_VERSION>")

target_link_libraries(AudioDecay PRIVATE Shared_VST_Target)
