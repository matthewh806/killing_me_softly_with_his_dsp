cmake_minimum_required(VERSION 3.19)

set(vst3sdk_SOURCE_DIR ../../dependencies/vst3sdk)
if(NOT vst3sdk_SOURCE_DIR)
    message(FATAL_ERROR "Path to VST3 SDK is empty!")
endif()

project(MattVerb VERSION 1.0.0)
  
set(SMTG_VSTGUI_ROOT "${vst3sdk_SOURCE_DIR}")

smtg_enable_vst3_sdk()

smtg_add_vst3plugin(MattVerb     
    version.h
    cids.h
    processor.h
    processor.cpp
    controller.h
    controller.cpp
    entry.cpp
)

#- VSTGUI Wanted ----
if(SMTG_ADD_VSTGUI)
	target_sources(MattVerb
		PRIVATE
			resource/editor.uidesc
	)
    target_link_libraries(MattVerb
        PRIVATE
            vstgui_support
    )

    smtg_target_add_plugin_resources(MattVerb
        RESOURCES
            resource/editor.uidesc
            resource/vst_background.png
            resource/slider_handle.png
            resource/vst_button_disabled.png
            resource/vst_button_enabled.png
    )
endif()
# -------------------

target_link_libraries(MattVerb
    PRIVATE
        sdk
        freeverb
)

if(SMTG_MAC)
    smtg_target_set_bundle(MattVerb INFOPLIST 
        "${CMAKE_CURRENT_LIST_DIR}/resource/Info.plist" 
        PREPROCESS
    )
elseif(SMTG_WIN)
    target_sources(MattVerb PRIVATE 
        resource/win32resource.rc
    )
    set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT MattVerb)
	
	set_target_properties(MattVerb PROPERTIES
		VS_DEBUGGER_COMMAND
			"$(ProgramW6432)/Steinberg/VST3PluginTestHost/VST3PluginTestHost.exe"
		VS_DEBUGGER_COMMAND_ARGUMENTS
			" --pluginfolder \"$(OutDir)\" "
	)
endif()


