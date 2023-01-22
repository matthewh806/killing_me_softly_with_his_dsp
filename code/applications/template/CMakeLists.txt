juce_add_gui_app(Template
    PRODUCT_NAME "Template Project"
    MICROPHONE_PERMISSION_ENABLED   TRUE)

juce_generate_juce_header(Template)

set_target_properties(Template PROPERTIES EXCLUDE_FROM_ALL TRUE)

set(TemplateApplicationSources
    ${UISources}
    ${CMAKE_SOURCE_DIR}/applications/template/Main.cpp
    ${CMAKE_SOURCE_DIR}/applications/template/MainComponent.h
    ${CMAKE_SOURCE_DIR}/applications/template/MainComponent.cpp
)
source_group("Source/ApplicationSources" FILES ${TemplateApplicationSources})

target_sources(Template PRIVATE
    ${CMAKE_SOURCE_DIR}/dsp/processors/TemplateProcessor.h
    ${CMAKE_SOURCE_DIR}/dsp/processors/TemplateProcessor.cpp
    ${TemplateApplicationSources}
)

target_compile_definitions(Template PRIVATE
    # JUCE_WEB_BROWSER and JUCE_USE_CURL would be on by default, but you might not need them.
    JUCE_WEB_BROWSER=0  # If you remove this, add `NEEDS_WEB_BROWSER TRUE` to the `juce_add_gui_app` call
    JUCE_USE_CURL=0     # If you remove this, add `NEEDS_CURL TRUE` to the `juce_add_gui_app` call
    JUCE_APPLICATION_NAME_STRING="$<TARGET_PROPERTY:Template,JUCE_PRODUCT_NAME>"
    JUCE_APPLICATION_VERSION_STRING="$<TARGET_PROPERTY:Template,JUCE_VERSION>")

target_link_libraries(Template
PRIVATE
    juce::juce_audio_utils
    juce::juce_gui_extra
PUBLIC
    juce::juce_recommended_config_flags
    juce::juce_recommended_lto_flags
    juce::juce_recommended_warning_flags)
