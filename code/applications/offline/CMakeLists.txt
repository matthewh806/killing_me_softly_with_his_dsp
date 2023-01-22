find_library(RUBBERBAND NAMES rubberband)

juce_add_gui_app(StretchArmstrong
    PRODUCT_NAME "Stretch Armstrong")

juce_generate_juce_header(StretchArmstrong)

target_sources(StretchArmstrong
    PRIVATE
        ${CoreSources}
        ${DspSources}
        ${UISources}
        ${CMAKE_SOURCE_DIR}/applications/offline/Main.cpp
        ${CMAKE_SOURCE_DIR}/applications/offline/MainComponent.h
        ${CMAKE_SOURCE_DIR}/applications/offline/MainComponent.cpp)

target_compile_definitions(StretchArmstrong
    PRIVATE
        # JUCE_WEB_BROWSER and JUCE_USE_CURL would be on by default, but you might not need them.
        JUCE_WEB_BROWSER=0  # If you remove this, add `NEEDS_WEB_BROWSER TRUE` to the `juce_add_gui_app` call
        JUCE_USE_CURL=0     # If you remove this, add `NEEDS_CURL TRUE` to the `juce_add_gui_app` call
        JUCE_APPLICATION_NAME_STRING="$<TARGET_PROPERTY:StretchArmstrong,JUCE_PRODUCT_NAME>"
        JUCE_APPLICATION_VERSION_STRING="$<TARGET_PROPERTY:StretchArmstrong,JUCE_VERSION>")

target_include_directories(RealTimeStretch PRIVATE ${CMAKE_SOURCE_DIR}/dependencies/rubberband/rubberband)

target_link_libraries(StretchArmstrong
    PRIVATE
        juce::juce_audio_utils
        juce::juce_gui_extra
    PUBLIC
        juce::juce_recommended_config_flags
        juce::juce_recommended_lto_flags
        juce::juce_recommended_warning_flags
        rubberband)
