juce_add_plugin(ARAJuceTest
    PRODUCT_NAME "ARAJuceTest"
    MICROPHONE_PERMISSION_ENABLED   TRUE
    IS_ARA_EFFECT TRUE
    FORMATS Standalone VST3)

juce_generate_juce_header(ARAJuceTest)

set(ARAJuceTestApplicationSources
    ${UISources}
)
source_group("Source/ApplicationSources" FILES ${ARAJuceTestApplicationSources})

target_sources(ARAJuceTest PRIVATE
    ${CMAKE_SOURCE_DIR}/playground/ara_juce_test_plugin/ARAPluginDemo.h
    ${CMAKE_SOURCE_DIR}/playground/ara_juce_test_plugin/main.cpp
    ${ARAJuceTestApplicationSources}
)

target_compile_definitions(ARAJuceTest PUBLIC
        # JUCE_WEB_BROWSER and JUCE_USE_CURL would be on by default, but you might not need them.
        ARA_ENABLE_INTERNAL_ASSERTS=1
        JUCE_WEB_BROWSER=0  # If you remove this, add `NEEDS_WEB_BROWSER TRUE` to the `juce_add_gui_app` call
        JUCE_USE_CURL=0     # If you remove this, add `NEEDS_CURL TRUE` to the `juce_add_gui_app` call
        JUCE_VST3_CAN_REPLACE_VST2=0)

target_compile_definitions(ARAJuceTest PRIVATE
    JUCE_APPLICATION_NAME_STRING="$<TARGET_PROPERTY:ARAJuceTest,JUCE_PRODUCT_NAME>"
    JUCE_APPLICATION_VERSION_STRING="$<TARGET_PROPERTY:ARAJuceTest,JUCE_VERSION>")

target_link_libraries(ARAJuceTest 
                      PRIVATE
                        juce::juce_audio_basics
                        juce::juce_audio_devices
                        juce::juce_audio_formats
                        juce::juce_audio_plugin_client 
                        juce::juce_audio_processors
                        juce::juce_audio_utils
                        juce::juce_core
                        juce::juce_data_structures
                        juce::juce_events 
                        juce::juce_graphics 
                        juce::juce_gui_basics 
                        juce::juce_gui_extra
                        juce::juce_recommended_config_flags
                        juce::juce_recommended_lto_flags
                        juce::juce_recommended_warning_flags)
