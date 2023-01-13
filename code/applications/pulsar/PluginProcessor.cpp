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
    return new PulsarAudioProcessorEditor (*this, mDeviceManager);
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

void PulsarAudioProcessor::sendNoteOnMessage(int noteNumber, float velocity)
{
    if(!mMidiOutput)
    {
        return;
    }
    
    auto* editor = getActiveEditor();
    if(editor == nullptr)
    {
        return;
    }
    
    auto* pulsarEditor = static_cast<PulsarAudioProcessorEditor*>(editor);
    if(pulsarEditor == nullptr)
    {
        return;
    }
    
    //! @todo: Use a midi buffer and do the timestamping properly...?
    auto messageOn = MidiMessage::noteOn(pulsarEditor->getMidiOutputChannel(), noteNumber, static_cast<uint8>(velocity));
    messageOn.setTimeStamp(Time::getMillisecondCounterHiRes());
    auto messageOff = MidiMessage::noteOff (messageOn.getChannel(), messageOn.getNoteNumber());
    messageOff.setTimeStamp (messageOn.getTimeStamp() + NOTE_OFF_TIME_MS); // lasts 300ms
    
    MidiBuffer buffer;
    buffer.addEvent(messageOn, 0);
    buffer.addEvent(messageOff, static_cast<int>(NOTE_OFF_TIME_MS * 44.1));
    
    mMidiOutput->sendBlockOfMessages(buffer, Time::getMillisecondCounterHiRes(), 44100.0);
}

void PulsarAudioProcessor::setMidiInput(String const& identifier)
{
    auto list = MidiInput::getAvailableDevices();
    mDeviceManager.removeMidiInputDeviceCallback(identifier, this);
    
    if(!mDeviceManager.isMidiInputDeviceEnabled(identifier))
    {
        mDeviceManager.setMidiInputDeviceEnabled(identifier, true);
    }
    
    mDeviceManager.addMidiInputDeviceCallback(identifier, this);
}

void PulsarAudioProcessor::setMidiOutput(juce::String const& identifier)
{
    auto list = MidiOutput::getAvailableDevices();
    mMidiOutput = MidiOutput::openDevice(identifier);
    
    if(mMidiOutput == nullptr)
    {
        std::cerr << "Error opening midioutput device " << identifier << "\n";
        return;
    }
    
    mMidiOutput->startBackgroundThread();
}

//==============================================================================
// This creates new instances of the plugin..
AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new PulsarAudioProcessor();
}

void PulsarAudioProcessor::handleIncomingMidiMessage (juce::MidiInput *source, const juce::MidiMessage &message)
{
    juce::ignoreUnused(source);
    
    const ScopedLock s1 (mMidiMonitorLock);
    mIncomingMessages.add(message);
    triggerAsyncUpdate();
}

void PulsarAudioProcessor::handleAsyncUpdate()
{
    // midi message loop
    Array<MidiMessage> messages;
    
    {
        const ScopedLock s1(mMidiMonitorLock);
        messages.swapWith(mIncomingMessages);
    }
    
    auto* editor = getActiveEditor();
    if(editor == nullptr)
    {
        return;
    }
    
    auto* pulsarEditor = static_cast<PulsarAudioProcessorEditor*>(editor);
    if(pulsarEditor == nullptr)
    {
        return;
    }
    
    for(auto &m : messages)
    {
        if(m.getChannel() != pulsarEditor->getMidiInputChannel())
        {
            continue;
        }
        
        if(m.isNoteOn())
        {
            pulsarEditor->getWorld().spawnBall(m.getNoteNumber(), m.getVelocity());
        }
        else if(m.isController())
        {
            auto const pitchVal = m.getControllerValue();
            
            // map 0 - 127 to 0 - 360
            auto anglularVelocity = 360.0 / static_cast<double>(127) * static_cast<double>(pitchVal);
            pulsarEditor->getWorld().setPolygonRotationSpeed(anglularVelocity);
        }
    }
}
