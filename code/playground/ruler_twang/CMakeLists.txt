juce_add_plugin(RulerTwang
    PRODUCT_NAME "RulerTwang"
    MICROPHONE_PERMISSION_ENABLED   TRUE
    FORMATS Standalone VST3)

juce_generate_juce_header(RulerTwang)

set(RulerTwangApplicationSources
    ${UISources}
)
source_group("Source/ApplicationSources" FILES ${MultDelayApplicationSources})

target_sources(RulerTwang PRIVATE
    ${CMAKE_SOURCE_DIR}/playground/ruler_twang/RulerTwangPlugin.h
    ${CMAKE_SOURCE_DIR}/playground/ruler_twang/RulerTwangPlugin.cpp
    ${CMAKE_SOURCE_DIR}/playground/ruler_twang/FreeVibrationalModes.h
    ${CMAKE_SOURCE_DIR}/playground/ruler_twang/FreeVibrationalModes.cpp
    ${CMAKE_SOURCE_DIR}/playground/ruler_twang/ClampedVibrationalModes.h
    ${CMAKE_SOURCE_DIR}/playground/ruler_twang/ClampedVibrationalModes.cpp
    ${MultDelayApplicationSources}
)

target_compile_definitions(RulerTwang PRIVATE
    JUCE_APPLICATION_NAME_STRING="$<TARGET_PROPERTY:RulerTwang,JUCE_PRODUCT_NAME>"
    JUCE_APPLICATION_VERSION_STRING="$<TARGET_PROPERTY:RulerTwang,JUCE_VERSION>")

target_link_libraries(RulerTwang PUBLIC juce::juce_dsp PRIVATE Shared_VST_Target)
