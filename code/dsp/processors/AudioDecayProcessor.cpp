#include "AudioDecayProcessor.h"

AudioDecayProcessor::AudioDecayProcessor()
: juce::AudioProcessor (BusesProperties().withInput  ("Input",     juce::AudioChannelSet::stereo())
                  .withOutput ("Output",    juce::AudioChannelSet::stereo())
                  .withInput  ("Sidechain", juce::AudioChannelSet::stereo()))
{
}

//==============================================================================
bool AudioDecayProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
    return layouts.getMainInputChannelSet() == layouts.getMainOutputChannelSet()
             && ! layouts.getMainInputChannelSet().isDisabled();
}

//==============================================================================
void AudioDecayProcessor::prepareToPlay (double sampleRate,
                                          int maximumExpectedSamplesPerBlock)
{
    mBlockSize = maximumExpectedSamplesPerBlock;
    mSampleRate = static_cast<int>(sampleRate);
}

void AudioDecayProcessor::releaseResources()
{
    
}

void AudioDecayProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer&)
{
    auto const channels = buffer.getNumChannels();
    if(channels > 2)
    {
        return;
    }
    
    // Drop every other sample
    auto const numSamples = buffer.getNumSamples();
    auto const numChannels = buffer.getNumChannels();
    for(auto i = 0; i < numSamples; ++i)
    {
        for(int ch = 0; ch < numChannels; ++ch)
        {
            auto const value = (i % 2) ? buffer.getSample(ch, i) : 0.0f;
            buffer.setSample(ch, i, value);
        }
    }
}

void AudioDecayProcessor::getStateInformation (MemoryBlock& destData)
{
    MemoryOutputStream stream (destData, true);
}

void AudioDecayProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    MemoryInputStream stream (data, static_cast<size_t> (sizeInBytes), false);
}
