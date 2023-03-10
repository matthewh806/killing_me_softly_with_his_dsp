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
    std::make_unique<juce::AudioParameterFloat>("tapadelay", "Tap A delay", 0.0f, 2000.0f, 500.0f),
    std::make_unique<juce::AudioParameterFloat>("tapbdelay", "Tap B delay", 0.0f, 2000.0f, 1250.0f),
    std::make_unique<juce::AudioParameterFloat>("tapcdelay", "Tap C delay", 0.0f, 2000.0f, 1750.0f),
    std::make_unique<juce::AudioParameterFloat>("tapddelay", "Tap D delay", 0.0f, 2000.0f, 2000.0f),
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
    mSampleRate = static_cast<float>(sampleRate);
    
    mDelayBuffers.clear();
    auto const bufferSize = static_cast<unsigned int>(MAX_DELAY_SECONDS * sampleRate + 1);
    std::array<float,4> delaySamples = { 0.5f * mSampleRate, 1.25f * mSampleRate, 1.75f * mSampleRate, 2.0f * mSampleRate };
    auto leftDelayBuffer = std::make_unique<MultitapCircularBuffer<float, 4>>(bufferSize, delaySamples);
    mDelayBuffers.push_back(std::move(leftDelayBuffer));
    
    auto rightDelayBuffer = std::make_unique<MultitapCircularBuffer<float, 4>>(bufferSize, delaySamples);
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
    auto const delayTimeC = static_cast<float>(*mState.getRawParameterValue("tapcdelay"));
    auto const delayTimeD = static_cast<float>(*mState.getRawParameterValue("tapddelay"));
    
    auto const delaySamplesA = delayTimeA / 1000.0f * mSampleRate;
    auto const delaySamplesB = delayTimeB / 1000.0f * mSampleRate;
    auto const delaySamplesC = delayTimeC / 1000.0f * mSampleRate;
    auto const delaySamplesD = delayTimeD / 1000.0f * mSampleRate;
    
    for(auto ch = 0; ch < channels; ++ch)
    {
        // TODO: Better way of updating this...
        mDelayBuffers[static_cast<size_t>(ch)]->setTapDelay(0, delaySamplesA);
        mDelayBuffers[static_cast<size_t>(ch)]->setTapDelay(1, delaySamplesB);
        mDelayBuffers[static_cast<size_t>(ch)]->setTapDelay(2, delaySamplesC);
        mDelayBuffers[static_cast<size_t>(ch)]->setTapDelay(3, delaySamplesD);
        
        auto const numTaps = mDelayBuffers[static_cast<size_t>(ch)]->getNumberOfTaps();
        
        for(int sample = 0; sample < numSamples; ++sample)
        {
            auto const inputSignal = buffer.getSample(ch, sample);
            
            auto delayedSample = 0.0f;
            for(size_t i = 0; i < numTaps; ++i)
            {
                delayedSample += mDelayBuffers[static_cast<size_t>(ch)]->readTap(i);
            }
            
            auto const wetDryRatio = static_cast<float>(*mState.getRawParameterValue("wetdry"));
            
            // TODO: Think about how to implement feedback individually
            // or just keep as global feedback and rename...
            auto const feedbackAmt = static_cast<float>(*mState.getRawParameterValue("feedbacka"));

            auto const inputToDelayBuffer = inputSignal + feedbackAmt * delayedSample;
            mDelayBuffers[static_cast<size_t>(ch)]->writeBuffer(inputToDelayBuffer);
            
            auto const outputValue = std::clamp((1.0f - wetDryRatio) * inputSignal + wetDryRatio * delayedSample, -1.0f, 1.0f);
            buffer.setSample(ch, sample, outputValue);
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
