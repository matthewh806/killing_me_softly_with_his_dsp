find_library(RUBBERBAND NAMES rubberband)


juce_add_gui_app(RealTimeStretch
    PRODUCT_NAME "Juce Real Time Stretch"
    MICROPHONE_PERMISSION_ENABLED   TRUE)

juce_generate_juce_header(RealTimeStretch)

set_target_properties(RealTimeStretch PROPERTIES EXCLUDE_FROM_ALL TRUE)

target_sources(RealTimeStretch
    PRIVATE
        ${CoreSources}
        ${UISources}
        ${DspSources}
        ${CMAKE_SOURCE_DIR}/applications/real_time/Main.cpp
        ${CMAKE_SOURCE_DIR}/applications/real_time/MainComponent.h
        ${CMAKE_SOURCE_DIR}/applications/real_time/MainComponent.cpp)

target_compile_definitions(RealTimeStretch
    PRIVATE
        JUCE_APPLICATION_NAME_STRING="$<TARGET_PROPERTY:RealTimeStretch,JUCE_PRODUCT_NAME>"
        JUCE_APPLICATION_VERSION_STRING="$<TARGET_PROPERTY:RealTimeStretch,JUCE_VERSION>")

target_include_directories(RealTimeStretch PRIVATE ${CMAKE_SOURCE_DIR}/dependencies/rubberband/rubberband)

target_link_libraries(RealTimeStretch
    PRIVATE
        Shared_VST_Target
    PUBLIC
        rubberband)
