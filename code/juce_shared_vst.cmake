add_library(Shared_VST_Target STATIC)

_juce_initialise_target(Shared_VST_Target)

target_link_libraries(Shared_VST_Target
    PRIVATE
        juce::juce_audio_utils
        juce::juce_dsp
        juce::juce_gui_extra
    PUBLIC
        juce::juce_recommended_config_flags
        juce::juce_recommended_warning_flags)

target_compile_definitions(Shared_VST_Target
    PUBLIC
        JUCE_WEB_BROWSER=0  # If you remove this, add `NEEDS_WEB_BROWSER TRUE` to the `juce_add_gui_app` call
        JUCE_USE_CURL=0     # If you remove this, add `NEEDS_CURL TRUE` to the `juce_add_gui_app` callss
    INTERFACE
        $<TARGET_PROPERTY:Shared_VST_Target,COMPILE_DEFINITIONS>)

target_include_directories(Shared_VST_Target
    INTERFACE
        $<TARGET_PROPERTY:Shared_VST_Target,INCLUDE_DIRECTORIES>)


set_target_properties(Shared_VST_Target PROPERTIES
        POSITION_INDEPENDENT_CODE TRUE
        VISIBILITY_INLINES_HIDDEN TRUE
        C_VISIBILITY_PRESET hidden
        CXX_VISIBILITY_PRESET hidden)
