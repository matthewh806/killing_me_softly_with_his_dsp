#include "RulerTwangPlugin.h"

#define MAX_DELAY_SECONDS 2

using namespace OUS;

AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new RulerTwangPlugin();
}

RulerTwangPlugin::RulerTwangPlugin()
: juce::AudioProcessor(BusesProperties().withInput("Input", juce::AudioChannelSet::stereo())
                                        .withOutput("Output", juce::AudioChannelSet::stereo()))
, mState(*this, nullptr, "pluginstate",
{
    std::make_unique<juce::AudioParameterBool>("triggertwang", "Trigger Twang", 0.0f),
    std::make_unique<juce::AudioParameterFloat>("vibrationfrequency", "Vibration Frequency", 0.1f, 1000.0f, 220.0f),
    std::make_unique<juce::AudioParameterFloat>("decaytime", "Decay Time (ms)", 10.0f, 2000.0f, 450.0f),
})
, mFullClampedModes({1.0f, 6.2669f, 17.5475f, 34.3861f, 56.8426f})
{
    mState.addParameterListener("triggertwang", this);
    mState.addParameterListener("vibrationfrequency", this);
    mState.addParameterListener("decaytime", this);
    mState.state.addChild({"uiState", {{"width", 400}, {"height", 250}}, {}}, -1, nullptr);
    
    mFullClampedModes.setLevel(0.2f);
}

RulerTwangPlugin::~RulerTwangPlugin()
{
    mState.removeParameterListener("decaytime", this);
    mState.removeParameterListener("vibrationfrequency", this);
    mState.removeParameterListener("triggertwang", this);
}

//==============================================================================
bool RulerTwangPlugin::isBusesLayoutSupported(const BusesLayout& layouts) const
{
    return layouts.getMainInputChannelSet() == layouts.getMainOutputChannelSet() && !layouts.getMainInputChannelSet().isDisabled();
}

//==============================================================================
void RulerTwangPlugin::prepareToPlay(double sampleRate,
                                         int maximumExpectedSamplesPerBlock)
{
    mBlockSize = maximumExpectedSamplesPerBlock;
    mSampleRate = static_cast<int>(sampleRate);
    
    mFullClampedModes.prepare({sampleRate, static_cast<uint32>(maximumExpectedSamplesPerBlock), 2});
    mFullClampedModes.setFundamentalFrequency(*mState.getRawParameterValue("vibrationfrequency"));
}

void RulerTwangPlugin::releaseResources()
{
}

void RulerTwangPlugin::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiBuffer)
{
    juce::ignoreUnused(midiBuffer);
    buffer.clear();
    
    juce::dsp::AudioBlock<float> block(buffer, 0);
    mFullClampedModes.process(juce::dsp::ProcessContextReplacing<float> (block));
}

void RulerTwangPlugin::getStateInformation(MemoryBlock& destData)
{
    MemoryOutputStream stream(destData, true);
}

void RulerTwangPlugin::setStateInformation(const void* data, int sizeInBytes)
{
    MemoryInputStream stream(data, static_cast<size_t>(sizeInBytes), false);
}

void RulerTwangPlugin::parameterChanged(const juce::String& parameterID, float newValue)
{
    if(parameterID == "vibrationfrequency")
    {
        mFullClampedModes.setFundamentalFrequency(newValue);
    }
    else if(parameterID == "triggertwang")
    {
        mFullClampedModes.trigger();
    }
    else if(parameterID == "decaytime")
    {
        mFullClampedModes.setDecayTime(newValue);
    }
}
