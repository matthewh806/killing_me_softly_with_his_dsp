#include "MultitapDelayPlugin.h"

#define MAX_DELAY_SECONDS 2

using namespace OUS;

AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new MultitapDelayPlugin();
}

MultitapDelayPlugin::MultitapDelayPlugin()
: juce::AudioProcessor(BusesProperties().withInput("Input", juce::AudioChannelSet::stereo())
                                        .withOutput("Output", juce::AudioChannelSet::stereo()))
, mState(*this, nullptr, "pluginstate",
{
    std::make_unique<juce::AudioParameterFloat>("tapadelay", "Tap A delay", 0.0f, 2.0f, 0.5f),
    std::make_unique<juce::AudioParameterFloat>("tapbdelay", "Tap B delay", 0.0f, 2.0f, 1.25f),
    std::make_unique<juce::AudioParameterFloat>("feedbacka", "Feedback A", 0.0f, 1.0f, 0.0f),
    std::make_unique<juce::AudioParameterFloat>("feedbackb", "Feedback B", 0.0f, 1.0f, 0.0f),
    std::make_unique<juce::AudioParameterFloat>("wetdry", "Wet/Dry Mix", 0.0f, 1.0f, 0.5f)
})
{
    mState.state.addChild({"uiState", {{"width", 400}, {"height", 250}}, {}}, -1, nullptr);
}

//==============================================================================
bool MultitapDelayPlugin::isBusesLayoutSupported(const BusesLayout& layouts) const
{
    return layouts.getMainInputChannelSet() == layouts.getMainOutputChannelSet() && !layouts.getMainInputChannelSet().isDisabled();
}

//==============================================================================
void MultitapDelayPlugin::prepareToPlay(double sampleRate,
                                         int maximumExpectedSamplesPerBlock)
{
    mBlockSize = maximumExpectedSamplesPerBlock;
    mSampleRate = static_cast<int>(sampleRate);
    
    mDelayBuffers.clear();
    auto const bufferSize = static_cast<unsigned int>(MAX_DELAY_SECONDS * sampleRate + 1);
    std::vector<float> delaySamples = {22050.0f, 11050.0f};
    auto leftDelayBuffer = std::make_unique<MultitapCircularBuffer<float>>(bufferSize, delaySamples);
    mDelayBuffers.push_back(std::move(leftDelayBuffer));
    
    auto rightDelayBuffer = std::make_unique<MultitapCircularBuffer<float>>(bufferSize, delaySamples);
    mDelayBuffers.push_back(std::move(rightDelayBuffer));
}

void MultitapDelayPlugin::releaseResources()
{
}

inline float value_from_to_range(float value, float oldMin, float oldMax, float newMin, float newMax)
{
    auto const oldRange = oldMax - oldMin;
    auto const newRange = newMax - newMin;
    
    return ((value - oldMin) / oldRange * newRange) + newMin;
}

void MultitapDelayPlugin::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiBuffer)
{
    juce::ignoreUnused(midiBuffer);
    
    auto const numSamples = buffer.getNumSamples();
    auto const channels = buffer.getNumChannels();
    
    // update the delay times if the param has changed
    auto const delayTimeA = static_cast<float>(*mState.getRawParameterValue("tapadelay"));
    auto const delayTimeB = static_cast<float>(*mState.getRawParameterValue("tapbdelay"));
    
    auto const delaySamplesA = delayTimeA * mSampleRate;
    auto const delaySamplesB = delayTimeB * mSampleRate;
    
    for(auto ch = 0; ch < channels; ++ch)
    {
        // TODO: Better way of updating this...
        mDelayBuffers[static_cast<size_t>(ch)]->setTapDelay(0, delaySamplesA);
        mDelayBuffers[static_cast<size_t>(ch)]->setTapDelay(1, delaySamplesB);
        auto const numTaps = mDelayBuffers[static_cast<size_t>(ch)]->getNumberOfTaps();
        
        for(int sample = 0; sample < numSamples; ++sample)
        {
            auto const inputSignal = buffer.getSample(ch, sample);
            
            auto delayedSample = 0.0f;
            for(size_t i = 0; i < numTaps; ++i)
            {
                delayedSample += mDelayBuffers[static_cast<size_t>(ch)]->readTap(i);
            }
            
            // mix the signals
            delayedSample = value_from_to_range(delayedSample, -1 * static_cast<int>(numTaps), 1 * static_cast<int>(numTaps), -1, 1);
            
            auto const wetDryRatio = static_cast<float>(*mState.getRawParameterValue("wetdry"));
            
            // TODO: Think about how to implement feedback individually
            // or just keep as global feedback and rename...
            auto const feedbackAmt = static_cast<float>(*mState.getRawParameterValue("feedbacka"));

            double inputToDelayBuffer = inputSignal + feedbackAmt * delayedSample;
            mDelayBuffers[static_cast<size_t>(ch)]->writeBuffer(static_cast<float>(inputToDelayBuffer));
            buffer.setSample(ch, sample, (1.0f - wetDryRatio) * inputSignal + wetDryRatio * delayedSample);
        }
    }
    
}

void MultitapDelayPlugin::getStateInformation(MemoryBlock& destData)
{
    MemoryOutputStream stream(destData, true);
}

void MultitapDelayPlugin::setStateInformation(const void* data, int sizeInBytes)
{
    MemoryInputStream stream(data, static_cast<size_t>(sizeInBytes), false);
}

void MultitapDelayPlugin::timerCallback()
{
}
