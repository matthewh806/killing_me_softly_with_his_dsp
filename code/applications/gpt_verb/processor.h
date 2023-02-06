#include "../../dependencies/freeverb/revmodel.hpp"
#include "public.sdk/source/vst/vstaudioeffect.h"
#include "pluginterfaces/vst/ivstparameterchanges.h"
#include "cids.h"

namespace OUS
{
    class ReverbVST : public Steinberg::Vst::AudioEffect
    {
    public:
        ReverbVST()
        {
            // Initialize the RevModel object
            m_reverb = new revmodel();
            setControllerClass(OUS::kGPTVerbControllerUID);
        }

        ~ReverbVST()
        {
            // Clean up the RevModel object
            delete m_reverb;
        }
        
        // Create function
        static Steinberg::FUnknown* createInstance (void* /*context*/)
        {
            return (Steinberg::Vst::IAudioProcessor*)new ReverbVST;
        }
        
        Steinberg::tresult initialize(Steinberg::FUnknown* context) override
        {
            // Here the Plug-in will be instanciated
            //---always initialize the parent-------
            Steinberg::tresult result = AudioEffect::initialize (context);
            // if everything Ok, continue
            if (result != Steinberg::kResultOk)
            {
                return result;
            }

            //--- create Audio IO ------
            addAudioInput (STR16 ("Stereo In"), Steinberg::Vst::SpeakerArr::kStereo);
            addAudioOutput (STR16 ("Stereo Out"), Steinberg::Vst::SpeakerArr::kStereo);
            
            m_reverb->setdry(0.5);
            m_reverb->setwet(0.5);
            m_reverb->setroomsize(0.8);
            m_reverb->setpredelaytime(0.0);

            return Steinberg::kResultOk;
        }

        Steinberg::tresult setActive(Steinberg::TBool state) override
        {
            // Reset the RevModel object when the plugin is activated or deactivated
            if (state)
            {
                // DOESNT EXIST
    //            m_reverb->reset();
            }

            return AudioEffect::setActive(state);
        }

        Steinberg::tresult setState(Steinberg::IBStream* state) override
        {
            // Load the RevModel state from the VST plugin state
    //        if (state)
    //        {
    //            m_reverb->setstate(state);
    //        }

            return Steinberg::kResultOk;
        }

        Steinberg::tresult getState(Steinberg::IBStream* state) override
        {
            // Save the RevModel state to the VST plugin state
    //        if (state)
    //        {
    //            m_reverb->getstate(state);
    //        }

            return Steinberg::kResultOk;
        }

        Steinberg::tresult PLUGIN_API process(Steinberg::Vst::ProcessData& data) override
        {
            // Check if there is input and output audio
            if (data.numInputs == 0 || data.numOutputs == 0)
                return Steinberg::kResultOk;

            // Get audio input and output buffers
            float* inL = data.inputs[0].channelBuffers32[0];
            float* inR = data.inputs[0].channelBuffers32[1];
            float* outL = data.outputs[0].channelBuffers32[0];
            float* outR = data.outputs[0].channelBuffers32[1];

            // Update reverb model parameters
            if (data.inputParameterChanges)
            {
                Steinberg::int32 numParamsChanged = data.inputParameterChanges->getParameterCount();
                for (Steinberg::int32 i = 0; i < numParamsChanged; i++)
                {
                    Steinberg::Vst::IParamValueQueue* paramQueue = data.inputParameterChanges->getParameterData(i);
                    if (paramQueue)
                    {
                        Steinberg::Vst::ParamValue value;
                        Steinberg::int32 sampleOffset;
                        Steinberg::int32 numPoints = paramQueue->getPointCount();
                        switch (paramQueue->getParameterId())
                        {
                            case OUS::GPTVerbParams::kParamDampId:
                                if (paramQueue->getPoint(numPoints - 1, sampleOffset, value) == Steinberg::kResultTrue)
                                    m_reverb->setdamp(value);
                                break;
                            case OUS::GPTVerbParams::kParamRoomSizeId:
                                if (paramQueue->getPoint(numPoints - 1, sampleOffset, value) == Steinberg::kResultTrue)
                                    m_reverb->setroomsize(value);
                                break;
                            case OUS::GPTVerbParams::kParamWidthId:
                                if (paramQueue->getPoint(numPoints - 1, sampleOffset, value) == Steinberg::kResultTrue)
                                    m_reverb->setwidth(value);
                                break;
                            case OUS::GPTVerbParams::kParamFreezeModeId:
                                if (paramQueue->getPoint(numPoints - 1, sampleOffset, value) == Steinberg::kResultTrue)
                                    m_reverb->setmode(value > 1.0);
                                break;
                            case OUS::GPTVerbParams::kParamDryWetId:
                                if (paramQueue->getPoint(numPoints - 1, sampleOffset, value) == Steinberg::kResultTrue)
                                    m_reverb->setdry(1.0 - value);
                                m_reverb->setwet(value);
                                break;
                            default:
                                break;
                        }
                    }
                }
            }

            // Process audio
            m_reverb->processreplace(inL, inR, outL, outR, data.numSamples, 1);

            return Steinberg::kResultOk;
        }

    private:
        revmodel* m_reverb;
    };

}

