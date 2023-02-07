//------------------------------------------------------------------------
// Copyright(c) 2022 the office of unspecified services.
//------------------------------------------------------------------------

#pragma once

#include "pluginterfaces/base/funknown.h"
#include "pluginterfaces/vst/vsttypes.h"

namespace OUS
{

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
    static const Steinberg::FUID kGPTVerbProcessorUID(0xA0F63E72, 0xCFF553D3, 0xA7D952EA, 0xE6974022);
    static const Steinberg::FUID kGPTVerbControllerUID(0x97D8D03A, 0x69845D92, 0x8F328E56, 0x717869D9);

#define GPTVerbVST3Category "Fx"

    //------------------------------------------------------------------------
} // namespace OUS
