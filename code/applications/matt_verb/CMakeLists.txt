cmake_minimum_required(VERSION 3.19)

set(vst3sdk_SOURCE_DIR /Users/harris/Work/software/VST_SDK/VST3_SDK)
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

    delayline.hpp
    delayline.cpp
    allpass.hpp
    allpass.cpp
    comb.hpp
    comb.cpp
    denormals.h
    revmodel.hpp
    revmodel.cpp
    tuning.h
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
    smtg_add_vst3_resource(MattVerb "resource/editor.uidesc")
    smtg_add_vst3_resource(MattVerb "resource/vst_background.png")
endif()
# -------------------

target_link_libraries(MattVerb
    PRIVATE
        sdk
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


