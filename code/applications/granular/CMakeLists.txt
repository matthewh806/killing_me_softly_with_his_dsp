juce_add_gui_app(Granular
    PRODUCT_NAME "Granular"
    MICROPHONE_PERMISSION_ENABLED   TRUE)

juce_generate_juce_header(Granular)

set(GranularApplicationSources
    ${UISources}
    ${CoreSources}
    ${CMAKE_SOURCE_DIR}/applications/granular/Main.cpp
    ${CMAKE_SOURCE_DIR}/applications/granular/MainComponent.h
    ${CMAKE_SOURCE_DIR}/applications/granular/MainComponent.cpp
    ${CMAKE_SOURCE_DIR}/applications/granular/Waveform.h
    ${CMAKE_SOURCE_DIR}/applications/granular/Waveform.cpp
    ${CMAKE_SOURCE_DIR}/applications/granular/Grain.h
    ${CMAKE_SOURCE_DIR}/applications/granular/Scheduler.h
    ${CMAKE_SOURCE_DIR}/applications/granular/Envelope.h
    ${CMAKE_SOURCE_DIR}/applications/granular/SequenceStrategy.h
)
source_group("Source/ApplicationSources" FILES ${GranularApplicationSources})

target_sources(Granular PRIVATE
    ${GranularApplicationSources}
)

target_compile_definitions(Granular PRIVATE
    # JUCE_WEB_BROWSER and JUCE_USE_CURL would be on by default, but you might not need them.
    JUCE_WEB_BROWSER=0  # If you remove this, add `NEEDS_WEB_BROWSER TRUE` to the `juce_add_gui_app` call
    JUCE_USE_CURL=0     # If you remove this, add `NEEDS_CURL TRUE` to the `juce_add_gui_app` call
    JUCE_APPLICATION_NAME_STRING="$<TARGET_PROPERTY:Granular,JUCE_PRODUCT_NAME>"
    JUCE_APPLICATION_VERSION_STRING="$<TARGET_PROPERTY:Granular,JUCE_VERSION>")

target_link_libraries(Granular
PRIVATE
    juce::juce_audio_utils
    juce::juce_gui_extra
PUBLIC
    juce::juce_recommended_config_flags
    juce::juce_recommended_lto_flags
    juce::juce_recommended_warning_flags)
