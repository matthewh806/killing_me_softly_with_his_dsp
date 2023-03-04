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
        JUCE_APPLICATION_NAME_STRING="$<TARGET_PROPERTY:StretchArmstrong,JUCE_PRODUCT_NAME>"
        JUCE_APPLICATION_VERSION_STRING="$<TARGET_PROPERTY:StretchArmstrong,JUCE_VERSION>")

target_include_directories(RealTimeStretch PRIVATE ${CMAKE_SOURCE_DIR}/dependencies/rubberband/rubberband)

target_link_libraries(StretchArmstrong
    PRIVATE
        Shared_VST_Target
    PUBLIC
        rubberband)
