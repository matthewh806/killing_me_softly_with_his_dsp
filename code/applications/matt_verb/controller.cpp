//------------------------------------------------------------------------
// Copyright(c) 2022 the office of unspecified services.
//------------------------------------------------------------------------

#include "controller.h"
#include "base/source/fstreamer.h"
#include "cids.h"
#include "pluginterfaces/base/ustring.h"
#include "vstgui/plugin-bindings/vst3editor.h"

using namespace Steinberg;

namespace OUS
{

    class ModeParameter : public Vst::Parameter
    {
    public:
        ModeParameter(int32 flags, int32 id);

        void toString(Vst::ParamValue normValue, Vst::String128 string) const SMTG_OVERRIDE;
        bool fromString(const Vst::TChar* string, Vst::ParamValue& normValue) const SMTG_OVERRIDE;
    };

    ModeParameter::ModeParameter(int32 flags, int32 id)
    {
        Steinberg::UString(info.title, USTRINGSIZE(info.title)).assign(USTRING("Mode"));

        info.flags = flags;
        info.id = id;
        info.stepCount = 0;
        info.defaultNormalizedValue = 0.0f;
        info.unitId = Steinberg::Vst::kRootUnitId;
    }

    void ModeParameter::toString(Vst::ParamValue normValue, Vst::String128 string) const
    {
        if(normValue >= 0.5f)
        {
            Steinberg::UString(string, 128).fromAscii("Freeze");
        }
        else
        {
            Steinberg::UString(string, 128).fromAscii("Normal");
        }
    }

    bool ModeParameter::fromString(const Vst::TChar* string, Vst::ParamValue& normValue) const
    {
        UString wrapper(const_cast<char16*>(string), tstrlen(string));
        return wrapper.scanFloat(normValue);
    }

    class PredelayParameter : public Vst::Parameter
    {
    public:
        // TODO: Units in the toString method
        PredelayParameter(int32 flags, int32 id, float minPlain = 0.0f, float maxPlain = 10000.0f);

        Vst::ParamValue toPlain(Vst::ParamValue _valueNormalized) const SMTG_OVERRIDE;
        Vst::ParamValue toNormalized(Vst::ParamValue plainValue) const SMTG_OVERRIDE;

        void toString(Vst::ParamValue normValue, Vst::String128 string) const SMTG_OVERRIDE;
        bool fromString(const Vst::TChar* string, Vst::ParamValue& normValue) const SMTG_OVERRIDE;

    private:
        float mMinPlain;
        float mMaxPlain;
    };

    PredelayParameter::PredelayParameter(int32 flags, int32 id, float minPlain, float maxPlain)
    {
        Steinberg::UString(info.title, USTRINGSIZE(info.title)).assign(USTRING("Pre Delay"));
        Steinberg::UString(info.units, USTRINGSIZE(info.units)).assign(USTRING("ms"));

        info.flags = flags;
        info.id = id;
        info.stepCount = 0;
        info.defaultNormalizedValue = 0.0f;
        info.unitId = Steinberg::Vst::kRootUnitId;

        mMinPlain = minPlain;
        mMaxPlain = maxPlain;
    }

    //------------------------------------------------------------------------
    Vst::ParamValue PredelayParameter::toPlain(Vst::ParamValue _valueNormalized) const
    {
        return _valueNormalized * (mMaxPlain - mMinPlain) + mMinPlain;
    }

    //------------------------------------------------------------------------
    Vst::ParamValue PredelayParameter::toNormalized(Vst::ParamValue plainValue) const
    {
        return (plainValue - mMinPlain) / (mMaxPlain - mMinPlain);
    }

    void PredelayParameter::toString(Vst::ParamValue normValue, Vst::String128 string) const
    {
        char text[32];
        sprintf(text, "%d ms", int32(toPlain(normValue)));
        Steinberg::UString(string, 128).fromAscii(text);
    }

    bool PredelayParameter::fromString(const Vst::TChar* string, Vst::ParamValue& normValue) const
    {
        UString wrapper(const_cast<char16*>(string), tstrlen(string));

        Vst::ParamValue plainValue = 0.0;
        bool found = wrapper.scanFloat(plainValue);
        normValue = toNormalized(plainValue);

        return found;
    }

    //------------------------------------------------------------------------
    // MattVerbController Implementation
    //------------------------------------------------------------------------
    tresult PLUGIN_API MattVerbController::initialize(FUnknown* context)
    {
        // Here the Plug-in will be instanciated

        //---do not forget to call parent ------
        tresult result = EditControllerEx1::initialize(context);
        if(result != kResultOk)
        {
            return result;
        }

        auto* modeParam = new ModeParameter(Steinberg::Vst::ParameterInfo::kCanAutomate, kModeId);
        parameters.addParameter(modeParam);
        parameters.addParameter(STR16("Bypass"), nullptr, 1, 0, Steinberg::Vst::ParameterInfo::kCanAutomate | Steinberg::Vst::ParameterInfo::kIsBypass, kBypassId);
        parameters.addParameter(STR16("Damping"), nullptr, 0, 1, Steinberg::Vst::ParameterInfo::kCanAutomate, kDampingId);
        parameters.addParameter(STR16("Width"), nullptr, 0, 1, Steinberg::Vst::ParameterInfo::kCanAutomate, kWidthId);
        parameters.addParameter(STR16("Room Size"), nullptr, 0, 1, Steinberg::Vst::ParameterInfo::kCanAutomate, kRoomSizeId);
        parameters.addParameter(STR16("Dry / Wet"), nullptr, 0, 1, Steinberg::Vst::ParameterInfo::kCanAutomate, kDryWetId);
        auto* predelayParam = new PredelayParameter(Steinberg::Vst::ParameterInfo::kCanAutomate, kPreDelaySizeId);
        parameters.addParameter(predelayParam);

        return result;
    }

    //------------------------------------------------------------------------
    tresult PLUGIN_API MattVerbController::terminate()
    {
        // Here the Plug-in will be de-instanciated, last possibility to remove some memory!

        //---do not forget to call parent ------
        return EditControllerEx1::terminate();
    }

    //------------------------------------------------------------------------
    tresult PLUGIN_API MattVerbController::setComponentState(IBStream* state)
    {
        // Here you get the state of the component (Processor part)
        if(!state)
            return kResultFalse;

        float dryWetSaveParam = 0.0f;
        bool bypassSaveParam = false;
        float modeSaveParam = 0.0f;
        float dampingSaveParam = 0.0f;
        float widthSaveParam = 0.0f;
        float roomSizeSaveParam = 0.0f;
        float preDelaySaveParam = 0.0f;

        IBStreamer streamer(state, kLittleEndian);

        if(streamer.readFloat(dryWetSaveParam) == false || streamer.readBool(bypassSaveParam) == false || streamer.readFloat(modeSaveParam) == false || streamer.readFloat(dampingSaveParam) == false || streamer.readFloat(widthSaveParam) == false || streamer.readFloat(roomSizeSaveParam) == false || streamer.readFloat(preDelaySaveParam) == false)
        {
            return kResultFalse;
        }

        if(auto dryWetParam = parameters.getParameter(kDryWetId))
        {
            dryWetParam->setNormalized(dryWetSaveParam);
        }
        if(auto bypassParam = parameters.getParameter(kBypassId))
        {
            bypassParam->setNormalized(bypassSaveParam);
        }
        if(auto modeParam = parameters.getParameter(kModeId))
        {
            modeParam->setNormalized(modeSaveParam);
        }
        if(auto dampingParam = parameters.getParameter(kDampingId))
        {
            dampingParam->setNormalized(dampingSaveParam);
        }
        if(auto widthParam = parameters.getParameter(kWidthId))
        {
            widthParam->setNormalized(widthSaveParam);
        }
        if(auto roomSizeParam = parameters.getParameter(kRoomSizeId))
        {
            roomSizeParam->setNormalized(roomSizeSaveParam);
        }
        if(auto preDelayParam = parameters.getParameter(kPreDelaySizeId))
        {
            preDelayParam->setNormalized(preDelaySaveParam);
        }

        return kResultOk;
    }

    //------------------------------------------------------------------------
    IPlugView* PLUGIN_API MattVerbController::createView(FIDString name)
    {
        // Here the Host wants to open your editor (if you have one)
        if(FIDStringsEqual(name, Vst::ViewType::kEditor))
        {
            // create your editor here and return a IPlugView ptr of it
            auto* view = new VSTGUI::VST3Editor(this, "view", "editor.uidesc");
            return view;
        }
        return nullptr;
    }

    //------------------------------------------------------------------------
    tresult PLUGIN_API MattVerbController::setParamNormalized(Vst::ParamID tag, Vst::ParamValue value)
    {
        // called by host to update your parameters
        tresult result = EditControllerEx1::setParamNormalized(tag, value);
        return result;
    }

    //------------------------------------------------------------------------
    tresult PLUGIN_API MattVerbController::getParamStringByValue(Vst::ParamID tag, Vst::ParamValue valueNormalized, Vst::String128 string)
    {
        // called by host to get a string for given normalized value of a specific parameter
        // (without having to set the value!)
        return EditControllerEx1::getParamStringByValue(tag, valueNormalized, string);
    }

    //------------------------------------------------------------------------
    tresult PLUGIN_API MattVerbController::getParamValueByString(Vst::ParamID tag, Vst::TChar* string, Vst::ParamValue& valueNormalized)
    {
        // called by host to get a normalized value from a string representation of a specific parameter
        // (without having to set the value!)
        return EditControllerEx1::getParamValueByString(tag, string, valueNormalized);
    }

    //------------------------------------------------------------------------
} // namespace OUS
