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
    
    mFreeVibrationModes.prepare({sampleRate, static_cast<uint32>(maximumExpectedSamplesPerBlock), 2});
    mFreeVibrationModes.setFundamentalFrequency(*mState.getRawParameterValue("vibrationfrequency"));
    
    mClampedBarBuffer.setSize(2, maximumExpectedSamplesPerBlock);
    mFreeBarBuffer.setSize(2, maximumExpectedSamplesPerBlock);
}

void RulerTwangPlugin::releaseResources()
{
}

void RulerTwangPlugin::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiBuffer)
{
    juce::ignoreUnused(midiBuffer);
    
    buffer.clear();
    mClampedBarBuffer.clear();
    mFreeBarBuffer.clear();
    
    juce::dsp::AudioBlock<float> clampedBlock(mClampedBarBuffer, 0);
    mFullClampedModes.process(juce::dsp::ProcessContextReplacing<float>(clampedBlock));
    
    auto const numSamples = buffer.getNumSamples();
    for(auto i = 0; i < numSamples; ++i)
    {
        // generate white noise
        auto const val = random.nextFloat() * 2.0f - 1.0f;
        
        // modulate with clamped vibration values
        auto const modulatedVal = val * clampedBlock.getSample(0, i);
        mFreeBarBuffer.setSample(0, i, modulatedVal);
        mFreeBarBuffer.setSample(1, i, modulatedVal);
    }
    
    // lpf mfreebarbuffer
    
    // pass to free bar vibration processor
    juce::dsp::AudioBlock<float> freeBlock(mFreeBarBuffer, 0);
    mFreeVibrationModes.process(juce::dsp::ProcessContextReplacing<float>(freeBlock));
    
    // sum the two modes and output!
    for(auto i = 0; i < numSamples; ++i)
    {
        auto const value = 0.5f * mFreeBarBuffer.getSample(0, i) + 0.5f * mClampedBarBuffer.getSample(0, i);
        buffer.setSample(0, i, value);
        buffer.setSample(1, i, value);
    }
    
    // hpf buffer
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
        mFreeVibrationModes.setFundamentalFrequency(newValue);
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
