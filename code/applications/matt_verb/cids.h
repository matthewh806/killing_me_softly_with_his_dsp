//------------------------------------------------------------------------
// Copyright(c) 2022 the office of unspecified services.
//------------------------------------------------------------------------

#pragma once

#include "pluginterfaces/base/funknown.h"
#include "pluginterfaces/vst/vsttypes.h"

namespace OUS {

enum MattVerbParams : Steinberg::Vst::ParamID
{
    kBypassId = 100,
    kDryWetId = 101,
    kModeId = 102,
    kDampingId = 103,
    kWidthId = 104,
    kRoomSizeId = 105,
    kPreDelaySizeId = 106
};

//------------------------------------------------------------------------
static const Steinberg::FUID kMattVerbProcessorUID (0x4A7AFD45, 0x51AA5BCB, 0x89D2A6CF, 0x66F014EE);
static const Steinberg::FUID kMattVerbControllerUID (0x58B6792E, 0x28B6585B, 0xA082F7F2, 0x96E598FB);

#define MattVerbVST3Category "Fx"

//------------------------------------------------------------------------
} // namespace OUS
