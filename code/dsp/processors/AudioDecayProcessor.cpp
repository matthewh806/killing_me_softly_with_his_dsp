#include "AudioDecayProcessor.h"

AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new AudioDecayProcessor();
}

AudioDecayProcessor::AudioDecayProcessor()
: juce::AudioProcessor (BusesProperties().withInput  ("Input",     juce::AudioChannelSet::stereo())
                  .withOutput ("Output",    juce::AudioChannelSet::stereo())
                  .withInput  ("Sidechain", juce::AudioChannelSet::stereo()))
, state(*this, nullptr, "state",
        {
            std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{"quantisation", 1}, "Quantisation Level", 0.0f, 1.0f, 0.5f),
            std::make_unique<juce::AudioParameterInt>(juce::ParameterID{"downsampling", 2}, "Downsampling Factor", 1, 16, 1),
            std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{"wetdry", 3}, "Wet/Dry mix", 0.0f, 1.0f, 0.0f)
        })
{
    state.state.addChild({ "uiState", {{"width", 400}, {"height", 200 } }, {} }, -1, nullptr);
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
    
    auto const numSamples = buffer.getNumSamples();
    auto const numChannels = buffer.getNumChannels();
    
    // TODO: Is this thread safe...?
    auto const quantisationLevel = state.getParameter("quantisation")->getValue();
    
    // TODO: Why is this zero? if I use getParameter
    // what does it do anyway? :S :S
    auto const downsampleFactor =  1.0; // state.getParameter("downsampling")->getValue();
    auto const wetDryMix = state.getParameter("wetdry")->getValue(); // 0.0 = 100% dry, 1.0 = 100% wet
    
    for(auto i = 0; i < numSamples; ++i)
    {
        for(int ch = 0; ch < numChannels; ++ch)
        {
            auto const s = buffer.getSample(ch, i);
            
            // First do the bit reduction
            auto const crushedValue = quantisationLevel * static_cast<int>(s / quantisationLevel);
            
            /* Now apply the downsampling operations
             * the simplest approach is just to preserve every N samples (0, N, 2N) and
             * zero those in between - preserving the length of the output and with no interpolation
             */
            auto const resampledValue = (i % static_cast<int>(downsampleFactor) == 0) ? crushedValue : 0.0f;
            auto const mixedValue = (1.0f - wetDryMix) * s + wetDryMix * resampledValue; // check this logic...
            
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
