juce_add_gui_app(UIPlayground
    PRODUCT_NAME "UI Playground"
    MICROPHONE_PERMISSION_ENABLED   TRUE)

juce_generate_juce_header(UIPlayground)

set(UIPlaygroundApplicationSources
    ${UISources}
    ${CMAKE_SOURCE_DIR}/applications/ui_playground/Main.cpp
    ${CMAKE_SOURCE_DIR}/applications/ui_playground/MainComponent.h
    ${CMAKE_SOURCE_DIR}/applications/ui_playground/MainComponent.cpp
)
source_group("Source/ApplicationSources" FILES ${UIPlaygroundApplicationSources})

target_sources(UIPlayground PRIVATE
    ${UIPlaygroundApplicationSources}
)

target_compile_definitions(UIPlayground PRIVATE
    # JUCE_WEB_BROWSER and JUCE_USE_CURL would be on by default, but you might not need them.
    JUCE_WEB_BROWSER=0  # If you remove this, add `NEEDS_WEB_BROWSER TRUE` to the `juce_add_gui_app` call
    JUCE_USE_CURL=0     # If you remove this, add `NEEDS_CURL TRUE` to the `juce_add_gui_app` call
    JUCE_APPLICATION_NAME_STRING="$<TARGET_PROPERTY:UIPlayground,JUCE_PRODUCT_NAME>"
    JUCE_APPLICATION_VERSION_STRING="$<TARGET_PROPERTY:UIPlayground,JUCE_VERSION>")

target_link_libraries(UIPlayground
PRIVATE
    juce::juce_gui_extra
PUBLIC
    juce::juce_recommended_config_flags
    juce::juce_recommended_lto_flags
    juce::juce_recommended_warning_flags)
