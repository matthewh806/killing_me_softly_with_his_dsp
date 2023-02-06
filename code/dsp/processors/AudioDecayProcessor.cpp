#include "AudioDecayProcessor.h"

using namespace OUS;

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
            std::make_unique<juce::AudioParameterInt>("bitdepth", "Bit Depth", 3, 24, 16),
            std::make_unique<juce::AudioParameterInt>("downsampling", "Downsampling Factor", 1, 10, 1),
            std::make_unique<juce::AudioParameterFloat>("wetdry", "Wet/Dry mix", 0.0f, 1.0f, 0.0f)
        })
{
    updateQuantisationLevel(static_cast<float>(*state.getRawParameterValue("bitdepth")));
    state.addParameterListener("bitdepth", this);
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
    
    auto const downsampleFactor = static_cast<int>(*state.getRawParameterValue("downsampling"));
    auto const wetDryMix = state.getParameter("wetdry")->getValue(); // 0.0 = 100% dry, 1.0 = 100% wet
    
    for(auto i = 0; i < numSamples; ++i)
    {
        for(int ch = 0; ch < numChannels; ++ch)
        {
            auto const s = buffer.getSample(ch, i);
            
            // First do the bit reduction
            auto const crushedValue = mQuantisationLevel * static_cast<int>(s / mQuantisationLevel);
            
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

void AudioDecayProcessor::parameterChanged (const String& parameterID, float newValue)
{
    if(parameterID.equalsIgnoreCase("bitdepth"))
    {
        updateQuantisationLevel(newValue);
    }
}

void AudioDecayProcessor::updateQuantisationLevel(float bitDepth)
{
    mQuantisationLevel = 2.0f / (std::pow(2.0f, bitDepth) - 1.0f);
}
