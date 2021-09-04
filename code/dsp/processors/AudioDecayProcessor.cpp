#include "AudioDecayProcessor.h"

AudioDecayProcessor::AudioDecayProcessor()
: juce::AudioProcessor (BusesProperties().withInput  ("Input",     juce::AudioChannelSet::stereo())
                  .withOutput ("Output",    juce::AudioChannelSet::stereo())
                  .withInput  ("Sidechain", juce::AudioChannelSet::stereo()))
{
}

void AudioDecayProcessor::setQuantisationLevel(int bitDepth)
{
    auto const qL = 2.0f / (std::pow(2.0f, static_cast<float>(bitDepth)) - 1.0f);
    mQuantisationLevel.store(qL);
}

void AudioDecayProcessor::setWetDryMix(float mix)
{
    mWetDryMix.store(std::min(std::max(mix, 0.0f), 1.0f));
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
    // y(n) = quantisation_level * (int ( x(n) / quantisation_level ))
    
    // TODO: U GOTTA HI PASS THAT SHIT BOI
    // TODO: Otherwise Claude & Nyquist won't be best pleased with you
    
    auto const channels = buffer.getNumChannels();
    if(channels > 2)
    {
        return;
    }
    
    // Drop every other sample
    auto const numSamples = buffer.getNumSamples();
    auto const numChannels = buffer.getNumChannels();
    
    // store the atomic variables locally
    auto const quantisationLevel = mQuantisationLevel.load();
    auto const wetDryMix = mWetDryMix.load(); // 0.0 = 100% dry, 1.0 = 100% wet
    
    for(auto i = 0; i < numSamples; ++i)
    {
        for(int ch = 0; ch < numChannels; ++ch)
        {
            auto const s = buffer.getSample(ch, i);
            auto const crushedValue = quantisationLevel * static_cast<int>(s / quantisationLevel);
            auto const mixedValue = (1.0f - wetDryMix) * s + wetDryMix * crushedValue; // check this logic...
            
            buffer.setSample(ch, i, mixedValue);
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
