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
    JUCE_APPLICATION_NAME_STRING="$<TARGET_PROPERTY:Template,JUCE_PRODUCT_NAME>"
    JUCE_APPLICATION_VERSION_STRING="$<TARGET_PROPERTY:Template,JUCE_VERSION>")

target_link_libraries(Template PRIVATE Shared_VST_Target)
