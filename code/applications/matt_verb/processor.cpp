//------------------------------------------------------------------------
// Copyright(c) 2022 the office of unspecified services.
//------------------------------------------------------------------------

#include "processor.h"
#include "cids.h"

#include "base/source/fstreamer.h"
#include "pluginterfaces/vst/ivstparameterchanges.h"

using namespace Steinberg;

namespace OUS
{
    //------------------------------------------------------------------------
    // MattVerbProcessor
    //------------------------------------------------------------------------
    MattVerbProcessor::MattVerbProcessor()
    {
        //--- set the wanted controller for our processor
        setControllerClass(kMattVerbControllerUID);
    }

    //------------------------------------------------------------------------
    MattVerbProcessor::~MattVerbProcessor()
    {
    }

    //------------------------------------------------------------------------
    tresult PLUGIN_API MattVerbProcessor::initialize(FUnknown* context)
    {
        // Here the Plug-in will be instanciated

        //---always initialize the parent-------
        tresult result = AudioEffect::initialize(context);
        // if everything Ok, continue
        if(result != kResultOk)
        {
            return result;
        }

        //--- create Audio IO ------
        addAudioInput(STR16("Stereo In"), Steinberg::Vst::SpeakerArr::kStereo);
        addAudioOutput(STR16("Stereo Out"), Steinberg::Vst::SpeakerArr::kStereo);

        rev.setdry(0.5);
        rev.setwet(0.5);
        rev.setroomsize(0.8);

        return kResultOk;
    }

    //------------------------------------------------------------------------
    tresult PLUGIN_API MattVerbProcessor::terminate()
    {
        // Here the Plug-in will be de-instanciated, last possibility to remove some memory!

        //---do not forget to call parent ------
        return AudioEffect::terminate();
    }

    //------------------------------------------------------------------------
    tresult PLUGIN_API MattVerbProcessor::setActive(TBool state)
    {
        //--- called when the Plug-in is enable/disable (On/Off) -----
        return AudioEffect::setActive(state);
    }

    //------------------------------------------------------------------------
    tresult PLUGIN_API MattVerbProcessor::process(Vst::ProcessData& data)
    {
        if(data.numInputs == 0 || data.numOutputs == 0)
        {
            return kResultOk;
        }
        
        // TODO: Hack to prevent mono FIX!!!
        if(data.numInputs == 1 || data.numOutputs == 1)
        {
            return kResultOk;
        }

        //--- First : Read inputs parameter changes-----------
        if(data.inputParameterChanges)
        {
            int32 numParamsChanged = data.inputParameterChanges->getParameterCount();
            for(int32 index = 0; index < numParamsChanged; index++)
            {
                if(auto* paramQueue = data.inputParameterChanges->getParameterData(index))
                {
                    Vst::ParamValue value;
                    int32 sampleOffset;
                    int32 numPoints = paramQueue->getPointCount();
                    switch(paramQueue->getParameterId())
                    {
                        case MattVerbParams::kBypassId:
                        {
                            if(paramQueue->getPoint(numPoints - 1, sampleOffset, value) == kResultTrue)
                            {
                                mBypass = (value > 0.5f);
                            }

                            break;
                        }
                        case MattVerbParams::kDryWetId:
                        {
                            if(paramQueue->getPoint(numPoints - 1, sampleOffset, value) == kResultTrue)
                            {
                                rev.setdry(1.0 - value);
                                rev.setwet(value);
                            }

                            break;
                        }
                        case MattVerbParams::kModeId:
                        {
                            if(paramQueue->getPoint(numPoints - 1, sampleOffset, value) == kResultTrue)
                            {
                                rev.setmode(value);
                            }

                            break;
                        }
                        case MattVerbParams::kDampingId:
                        {
                            if(paramQueue->getPoint(numPoints - 1, sampleOffset, value) == kResultTrue)
                            {
                                rev.setdamp(value);
                            }

                            break;
                        }
                        case MattVerbParams::kWidthId:
                        {
                            if(paramQueue->getPoint(numPoints - 1, sampleOffset, value) == kResultTrue)
                            {
                                rev.setwidth(value);
                            }

                            break;
                        }
                        case MattVerbParams::kRoomSizeId:
                        {
                            if(paramQueue->getPoint(numPoints - 1, sampleOffset, value) == kResultTrue)
                            {
                                rev.setroomsize(value);
                            }

                            break;
                        }
                        case MattVerbParams::kPreDelaySizeId:
                        {
                            if(paramQueue->getPoint(numPoints - 1, sampleOffset, value) == kResultTrue)
                            {
                                // scale to appropriate range
                                constexpr float min = 0.0;
                                constexpr float max = 10000; // 10 secs

                                auto const scaledValue = (max - min) * value + min;
                                rev.setpredelaytime(scaledValue);
                            }
                        }
                    }
                }
            }
        }

        if(data.numSamples > 0)
        {
            Steinberg::Vst::SpeakerArrangement arr;
            getBusArrangement(Steinberg::Vst::kOutput, 0, arr);
            int numChannels = Steinberg::Vst::SpeakerArr::getChannelCount(arr);

            float** inputChannels = data.inputs[0].channelBuffers32;
            float** outputChannels = data.outputs[0].channelBuffers32;

            if(mBypass)
            {
                for(int sample = 0; sample < data.numSamples; sample++)
                {
                    for(int channel = 0; channel < numChannels; channel++)
                    {
                        outputChannels[channel][sample] = inputChannels[channel][sample];
                    }
                }
            }
            else
            {
                rev.processreplace(inputChannels[0], inputChannels[1], outputChannels[0], outputChannels[1], data.numSamples, 1);
            }
        }

        return kResultOk;
    }

    //------------------------------------------------------------------------
    tresult PLUGIN_API MattVerbProcessor::setupProcessing(Vst::ProcessSetup& newSetup)
    {
        //--- called before any processing ----
        return AudioEffect::setupProcessing(newSetup);
    }

    //------------------------------------------------------------------------
    tresult PLUGIN_API MattVerbProcessor::canProcessSampleSize(int32 symbolicSampleSize)
    {
        // by default kSample32 is supported
        if(symbolicSampleSize == Vst::kSample32)
            return kResultTrue;

        // disable the following comment if your processing support kSample64
        /* if (symbolicSampleSize == Vst::kSample64)
                return kResultTrue; */

        return kResultFalse;
    }

    //------------------------------------------------------------------------
    tresult PLUGIN_API MattVerbProcessor::setState(IBStream* state)
    {
        if(!state)
        {
            return kResultFalse;
        }

        // called when we load a preset, the model has to be reloaded
        IBStreamer streamer(state, kLittleEndian);

        float dryWetSaveParam = 0.0f;
        float bypassSaveParam = 0.0f;
        float modeSaveParam = 0.0f;
        float dampingSaveParam = 0.0f;
        float widthSaveParam = 0.0f;
        float roomSizeSaveParam = 0.0f;
        float preDelaySaveParam = 0.0f;

        if(streamer.readFloat(dryWetSaveParam) == false || streamer.readFloat(bypassSaveParam) == false || streamer.readFloat(modeSaveParam) == false || streamer.readFloat(dampingSaveParam) == false || streamer.readFloat(widthSaveParam) == false || streamer.readFloat(roomSizeSaveParam) == false || streamer.readFloat(preDelaySaveParam) == false)
        {
            return kResultFalse;
        }

        rev.setwet(dryWetSaveParam);
        rev.setdry(1.0 - dryWetSaveParam);
        rev.setmode(modeSaveParam);
        rev.setdamp(dampingSaveParam);
        rev.setwidth(widthSaveParam);
        rev.setroomsize(roomSizeSaveParam);
        rev.setpredelaytime(preDelaySaveParam);

        return kResultOk;
    }

    //------------------------------------------------------------------------
    tresult PLUGIN_API MattVerbProcessor::getState(IBStream* state)
    {
        // save model
        float dryWetSaveParam = rev.getwet();
        float bypassSaveParam = mBypass;
        float modeSaveParam = rev.getmode();
        float dampingSaveParam = rev.getdamp();
        float widthSaveParam = rev.getwidth();
        float roomSizeSaveParam = rev.getroomsize();
        float preDelaySaveParam = rev.getpredelaytime();

        IBStreamer streamer(state, kLittleEndian);
        streamer.writeFloat(dryWetSaveParam);
        streamer.writeFloat(bypassSaveParam);
        streamer.writeFloat(modeSaveParam);
        streamer.writeFloat(dampingSaveParam);
        streamer.writeFloat(widthSaveParam);
        streamer.writeFloat(roomSizeSaveParam);
        streamer.writeFloat(preDelaySaveParam);

        return kResultOk;
    }

    //------------------------------------------------------------------------
} // namespace OUS
