//------------------------------------------------------------------------
// Copyright(c) 2022 the office of unspecified services.
//------------------------------------------------------------------------

#pragma once

#include "pluginterfaces/base/ustring.h"
#include "public.sdk/source/vst/vsteditcontroller.h"
#include "vstgui/plugin-bindings/vst3editor.h"

namespace OUS
{

    //------------------------------------------------------------------------
    //  GPTVerbController
    //------------------------------------------------------------------------
    class GPTVerbController : public Steinberg::Vst::EditControllerEx1
    {
    public:
        //------------------------------------------------------------------------
        GPTVerbController() = default;
        ~GPTVerbController() SMTG_OVERRIDE = default;

        // Create function
        static Steinberg::FUnknown* createInstance(void* /*context*/)
        {
            return (Steinberg::Vst::IEditController*)new GPTVerbController;
        }

        // IPluginBase
        Steinberg::tresult PLUGIN_API initialize(Steinberg::FUnknown* context) SMTG_OVERRIDE
        {
            // Call parent method
            Steinberg::tresult result = EditControllerEx1::initialize(context);
            if(result != Steinberg::kResultOk)
                return result;

            // Initialize parameters
            parameters.addParameter(new Steinberg::Vst::RangeParameter(USTRING("Damp"), kParamDampId, USTRING("%"), 0.0, 1.0, 0.5));
            parameters.addParameter(new Steinberg::Vst::RangeParameter(USTRING("Room Size"), kParamRoomSizeId, USTRING("%"), 0.0, 1.0, 0.5));
            parameters.addParameter(new Steinberg::Vst::RangeParameter(USTRING("Width"), kParamWidthId, USTRING("%"), 0.0, 1.0, 0.5));
            parameters.addParameter(new Steinberg::Vst::RangeParameter(USTRING("Freeze Mode"), kParamFreezeModeId, USTRING(""), 0.0, 2.0, 0.0));
            parameters.addParameter(new Steinberg::Vst::RangeParameter(USTRING("Dry/Wet Mix"), kParamDryWetId, USTRING("%"), 0.0, 1.0, 1.0));

            return Steinberg::kResultOk;
        }

        // EditController
        Steinberg::IPlugView* PLUGIN_API createView(Steinberg::FIDString name) SMTG_OVERRIDE
        {
            // Here the Host wants to open your editor (if you have one)
            if(Steinberg::FIDStringsEqual(name, Steinberg::Vst::ViewType::kEditor))
            {
                // create your editor here and return a IPlugView ptr of it
                auto* view = new VSTGUI::VST3Editor(this, "view", "editor.uidesc");
                return view;
            }
            return nullptr;
        }

        //	Steinberg::tresult PLUGIN_API setState (Steinberg::IBStream* state) SMTG_OVERRIDE;
        //	Steinberg::tresult PLUGIN_API getState (Steinberg::IBStream* state) SMTG_OVERRIDE;
        //	Steinberg::tresult PLUGIN_API setParamNormalized (Steinberg::Vst::ParamID tag,
        //                                                      Steinberg::Vst::ParamValue value) SMTG_OVERRIDE;
        //	Steinberg::tresult PLUGIN_API getParamStringByValue (Steinberg::Vst::ParamID tag,
        //                                                         Steinberg::Vst::ParamValue valueNormalized,
        //                                                         Steinberg::Vst::String128 string) SMTG_OVERRIDE;
        //	Steinberg::tresult PLUGIN_API getParamValueByString (Steinberg::Vst::ParamID tag,
        //                                                         Steinberg::Vst::TChar* string,
        //                                                         Steinberg::Vst::ParamValue& valueNormalized) SMTG_OVERRIDE;

        //---Interface---------
        DEFINE_INTERFACES
        // Here you can add more supported VST3 interfaces
        // DEF_INTERFACE (Vst::IXXX)
        END_DEFINE_INTERFACES(EditController)
        DELEGATE_REFCOUNT(EditController)

        //------------------------------------------------------------------------

    protected:
    };

    //------------------------------------------------------------------------
} // namespace OUS
