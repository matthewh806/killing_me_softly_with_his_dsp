cmake_minimum_required(VERSION 3.19)

set(vst3sdk_SOURCE_DIR ../../dependencies/vst3sdk)
if(NOT vst3sdk_SOURCE_DIR)
    message(FATAL_ERROR "Path to VST3 SDK is empty!")
endif()

project(GPTVerb VERSION 1.0.0)
  
set(SMTG_VSTGUI_ROOT "${vst3sdk_SOURCE_DIR}")

smtg_enable_vst3_sdk()

smtg_add_vst3plugin(GPTVerb     
    version.h
    cids.h
    processor.h
    controller.h
    entry.cpp
)

#- VSTGUI Wanted ----
if(SMTG_ADD_VSTGUI)
	target_sources(GPTVerb
		PRIVATE
			resource/editor.uidesc
	)
    target_link_libraries(GPTVerb
        PRIVATE
            vstgui_support
    )
    smtg_add_vst3_resource(GPTVerb "resource/editor.uidesc")
    smtg_add_vst3_resource(GPTVerb "resource/vst_background.png")
endif()
# -------------------

target_link_libraries(GPTVerb
    PRIVATE
        sdk
        freeverb
)

if(SMTG_MAC)
    smtg_target_set_bundle(GPTVerb INFOPLIST 
        "${CMAKE_CURRENT_LIST_DIR}/resource/Info.plist" 
        PREPROCESS
    )
elseif(SMTG_WIN)
    target_sources(GPTVerb PRIVATE 
        resource/win32resource.rc
    )
    set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT GPTVerb)
	
	set_target_properties(GPTVerb PROPERTIES
		VS_DEBUGGER_COMMAND
			"$(ProgramW6432)/Steinberg/VST3PluginTestHost/VST3PluginTestHost.exe"
		VS_DEBUGGER_COMMAND_ARGUMENTS
			" --pluginfolder \"$(OutDir)\" "
	)
endif()


