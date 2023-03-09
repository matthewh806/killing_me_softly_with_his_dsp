juce_add_plugin(MultitapDelay
    PRODUCT_NAME "MultitapDelay"
    MICROPHONE_PERMISSION_ENABLED   TRUE
    FORMATS Standalone VST3)

juce_generate_juce_header(MultitapDelay)

set(MultitapDelayApplicationSources
    ${UISources}
)
source_group("Source/ApplicationSources" FILES ${MultDelayApplicationSources})

target_sources(MultitapDelay PRIVATE
    ${CMAKE_SOURCE_DIR}/playground/multitap_delay/MultitapDelayPlugin.h
    ${CMAKE_SOURCE_DIR}/playground/multitap_delay/MultitapDelayPlugin.cpp
    ${MultDelayApplicationSources}
)

target_compile_definitions(MultitapDelay PRIVATE
    JUCE_APPLICATION_NAME_STRING="$<TARGET_PROPERTY:MultitapDelay,JUCE_PRODUCT_NAME>"
    JUCE_APPLICATION_VERSION_STRING="$<TARGET_PROPERTY:MultitapDelay,JUCE_VERSION>")

target_link_libraries(MultitapDelay PRIVATE Shared_VST_Target)
