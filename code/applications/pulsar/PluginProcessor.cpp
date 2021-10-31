#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
PulsarAudioProcessor::PulsarAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
     : juce::AudioProcessor (BusesProperties())
#endif
{
}

PulsarAudioProcessor::~PulsarAudioProcessor()
{
}

//==============================================================================
const String PulsarAudioProcessor::getName() const
{
    return "Pulsar";
}

bool PulsarAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool PulsarAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool PulsarAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double PulsarAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int PulsarAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int PulsarAudioProcessor::getCurrentProgram()
{
    return 0;
}

void PulsarAudioProcessor::setCurrentProgram (int index)
{
    juce::ignoreUnused(index);
}

const String PulsarAudioProcessor::getProgramName (int index)
{
    juce::ignoreUnused(index);
    return {};
}

void PulsarAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
    juce::ignoreUnused(index, newName);
}

//==============================================================================
void PulsarAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    juce::ignoreUnused(sampleRate, samplesPerBlock);
}

void PulsarAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool PulsarAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
  #if JucePlugin_IsMidiEffect
    ignoreUnused (layouts);
    return true;
  #else
    // This is the place where you check if the layout is supported.
    // In this template code we only support mono or stereo.
    if (layouts.getMainOutputChannelSet() != AudioChannelSet::mono()
     && layouts.getMainOutputChannelSet() != AudioChannelSet::stereo())
        return false;

    // This checks if the input layout matches the output layout
   #if ! JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
   #endif

    return true;
  #endif
}
#endif

void PulsarAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ignoreUnused(midiMessages);
    
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());
}

//==============================================================================
bool PulsarAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

AudioProcessorEditor* PulsarAudioProcessor::createEditor()
{
    return new PulsarAudioProcessorEditor (*this);
}

//==============================================================================
void PulsarAudioProcessor::getStateInformation (MemoryBlock& destData)
{
    juce::ignoreUnused(destData);
}

void PulsarAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    juce::ignoreUnused(data, sizeInBytes);
}

//==============================================================================
// This creates new instances of the plugin..
AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new PulsarAudioProcessor();
}
