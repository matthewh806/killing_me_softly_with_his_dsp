//------------------------------------------------------------------------
// Copyright(c) 2022 the office of unspecified services.
//------------------------------------------------------------------------

#pragma once

#include "pluginterfaces/base/funknown.h"
#include "pluginterfaces/vst/vsttypes.h"

namespace OUS {

enum GPTVerbParams : Steinberg::Vst::ParamID
{
    kParamDampId = 100,
    kParamRoomSizeId,
    kParamWidthId,
    kParamFreezeModeId,
    kParamDryWetId,
    kParamNumParameters
};

//------------------------------------------------------------------------
static const Steinberg::FUID kGPTVerbProcessorUID (0x4A7AFD45, 0x51AA5BCB, 0x89D2A6CF, 0x66F014EE);
static const Steinberg::FUID kGPTVerbControllerUID (0x58B6792E, 0x28B6585B, 0xA082F7F2, 0x96E598FB);

#define GPTVerbVST3Category "Fx"

//------------------------------------------------------------------------
} // namespace OUS
