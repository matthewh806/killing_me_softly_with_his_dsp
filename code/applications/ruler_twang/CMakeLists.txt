juce_add_plugin(RulerTwang
    PRODUCT_NAME "RulerTwang"
    MICROPHONE_PERMISSION_ENABLED   TRUE
    FORMATS VST3)

juce_generate_juce_header(RulerTwang)

set(RulerTwangApplicationSources
    ${UISources}
)
source_group("Source/ApplicationSources" FILES ${RulerTwangApplicationSources})

target_sources(RulerTwang PRIVATE
    ${CMAKE_SOURCE_DIR}/applications/
    ${CMAKE_SOURCE_DIR}/dsp/physical_modelling/RulerVibrationalModes.h
    ${CMAKE_SOURCE_DIR}/dsp/physical_modelling/RulerVibrationalModes.cpp
    ${CMAKE_SOURCE_DIR}/dsp/physical_modelling/FreeVibrationalModes.h
    ${CMAKE_SOURCE_DIR}/dsp/physical_modelling/FreeVibrationalModes.cpp
    ${CMAKE_SOURCE_DIR}/dsp/physical_modelling/ClampedVibrationalModes.h
    ${CMAKE_SOURCE_DIR}/dsp/physical_modelling/ClampedVibrationalModes.cpp
    ${CMAKE_SOURCE_DIR}/applications/ruler_twang/RulerTwangPlugin.h
    ${CMAKE_SOURCE_DIR}/applications/ruler_twang/RulerTwangPlugin.cpp
    ${CMAKE_SOURCE_DIR}/applications/ruler_twang/RulerTwangPluginEditor.h
    ${CMAKE_SOURCE_DIR}/applications/ruler_twang/RulerTwangPluginEditor.cpp
    ${RulerTwangApplicationSources}
)

target_compile_definitions(RulerTwang PRIVATE
    JUCE_APPLICATION_NAME_STRING="$<TARGET_PROPERTY:RulerTwang,JUCE_PRODUCT_NAME>"
    JUCE_APPLICATION_VERSION_STRING="$<TARGET_PROPERTY:RulerTwang,JUCE_VERSION>")

target_link_libraries(RulerTwang PUBLIC juce::juce_dsp PRIVATE Shared_VST_Target)
