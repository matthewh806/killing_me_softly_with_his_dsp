#include "SimpleDelayProcessor.h"

#define MAX_DELAY_SECONDS 2

using namespace OUS;

AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new SimpleDelayProcessor();
}

SimpleDelayProcessor::SimpleDelayProcessor()
: juce::AudioProcessor (BusesProperties().withInput  ("Input",     juce::AudioChannelSet::stereo())
                  .withOutput ("Output",    juce::AudioChannelSet::stereo())
                  .withInput  ("Sidechain", juce::AudioChannelSet::stereo()))
, state(*this, nullptr, "state",
        {
            std::make_unique<juce::AudioParameterBool>("sync", "Sync", false),
            std::make_unique<juce::AudioParameterFloat>("wetdry", "Wet/Dry Mix", 0.0f, 1.0f, 0.5f),
            std::make_unique<juce::AudioParameterFloat>("delaytime", "Delay Time", 0.0f, 2.0f, 0.1f),
            std::make_unique<juce::AudioParameterInt>("delaydivisor", "Delay Time", 0, 6, 3),
            std::make_unique<juce::AudioParameterFloat>("feedback", "Feedback", 0.0f, 1.0f, 0.5f)
        })
{
    state.state.addChild({ "uiState", {{"width", 400}, {"height", 250 } }, {} }, -1, nullptr);
}

//==============================================================================
bool SimpleDelayProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
    return layouts.getMainInputChannelSet() == layouts.getMainOutputChannelSet()
             && ! layouts.getMainInputChannelSet().isDisabled();
}

//==============================================================================
void SimpleDelayProcessor::prepareToPlay (double sampleRate,
                                          int maximumExpectedSamplesPerBlock)
{
    mBlockSize = maximumExpectedSamplesPerBlock;
    mSampleRate = static_cast<int>(sampleRate);
    
    mDelayBuffers.clear();
    
    auto const bufferSize = static_cast<unsigned int>(MAX_DELAY_SECONDS * sampleRate) + 1; // avoid round down?
    auto leftDelayBuffer = std::make_unique<CircularBuffer<float>>();
    leftDelayBuffer->createCircularBuffer(bufferSize);
    mDelayBuffers.push_back(std::move(leftDelayBuffer));
    
    auto rightDelayBuffer = std::make_unique<CircularBuffer<float>>();
    rightDelayBuffer->createCircularBuffer(bufferSize);
    mDelayBuffers.push_back(std::move(rightDelayBuffer));
}

void SimpleDelayProcessor::releaseResources()
{
    
}

void SimpleDelayProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer&)
{
    auto delayTime = static_cast<float>(*state.getRawParameterValue("delaytime"));
    auto const syncMode = static_cast<float>(*state.getRawParameterValue("sync")) >= 0.5f;
    if(syncMode)
    {
        // calculate delay mode based on current BPM
        // NOTE/ TODO: This assumes a 4/4 time signature at the moment
        if(auto* playhead = getPlayHead())
        {
            auto const pos = playhead->getPosition();
            if(pos.hasValue())
            {
                auto const syncDelayDivisor = syncedDelayDivisions[static_cast<int>(*state.getRawParameterValue("delaydivisor"))];
                auto const bpm = pos->getBpm().orFallback(120);
                
                auto time_quarter = 60.0 / bpm;
                delayTime = static_cast<float>(4.0 / syncDelayDivisor * time_quarter);
            }
        }
    }
    
    auto const wetDryRatio = static_cast<float>(*state.getRawParameterValue("wetdry"));
    auto const feedbackAmt = static_cast<float>(*state.getRawParameterValue("feedback"));
    
    auto const channels = buffer.getNumChannels();
    if(channels > 2)
    {
        return;
    }
    
    auto const numSamples = buffer.getNumSamples();
    auto const fractionalSampleDelay = delayTime * mSampleRate;
    
    for(auto ch = 0; ch < channels; ++ch)
    {
        for(int sample = 0; sample < numSamples; ++sample)
        {
            auto const inputSignal = buffer.getSample(ch, sample);
            auto const delayedSample = static_cast<float>(mDelayBuffers[static_cast<size_t>(ch)]->readBuffer(fractionalSampleDelay)); // not thread safe...

            double inputToDelayBuffer = inputSignal + feedbackAmt * delayedSample;
            mDelayBuffers[static_cast<size_t>(ch)]->writeBuffer(static_cast<float>(inputToDelayBuffer));
            buffer.setSample(ch, sample, (1.0f - wetDryRatio) * inputSignal + wetDryRatio * delayedSample);
        }
    }
}

void SimpleDelayProcessor::getStateInformation (MemoryBlock& destData)
{
    MemoryOutputStream stream (destData, true);
}

void SimpleDelayProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    MemoryInputStream stream (data, static_cast<size_t> (sizeInBytes), false);
}
