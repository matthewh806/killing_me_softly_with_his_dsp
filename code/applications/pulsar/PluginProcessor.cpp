#include "PluginProcessor.h"
#include "PluginEditor.h"

using namespace OUS;

//==============================================================================
PulsarAudioProcessor::PulsarAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
: juce::AudioProcessor(getBusesLayout())
#endif
{
    mStartTime = juce::Time::getMillisecondCounterHiRes() * 0.001;
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
    return 1; // NB: some hosts don't cope very well if you tell them there are 0 programs,
              // so this should be at least 1, even if you're not really implementing programs.
}

int PulsarAudioProcessor::getCurrentProgram()
{
    return 0;
}

void PulsarAudioProcessor::setCurrentProgram(int index)
{
    juce::ignoreUnused(index);
}

const String PulsarAudioProcessor::getProgramName(int index)
{
    juce::ignoreUnused(index);
    return "None";
}

void PulsarAudioProcessor::changeProgramName(int index, const juce::String& newName)
{
    juce::ignoreUnused(index, newName);
}

//==============================================================================
void PulsarAudioProcessor::prepareToPlay(double sampleRate, int samplesPerBlock)
{
    juce::ignoreUnused(samplesPerBlock);

    mSampleRate = sampleRate;

    // Set to be consistent with the size of the midi buffer used in the juce VST3 wrapper.
    mOutgoingMessages.ensureSize(2048);
    mOutgoingMessages.clear();
}

void PulsarAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool PulsarAudioProcessor::isBusesLayoutSupported(const BusesLayout& layouts) const
{
#if JucePlugin_IsMidiEffect
    ignoreUnused(layouts);
    return true;
#else
    // This is the place where you check if the layout is supported.
    // In this template code we only support mono or stereo.
    if(layouts.getMainOutputChannelSet() != AudioChannelSet::mono() && layouts.getMainOutputChannelSet() != AudioChannelSet::stereo())
        return false;

        // This checks if the input layout matches the output layout
#if !JucePlugin_IsSynth
    if(layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
#endif

    return true;
#endif
}
#endif

void PulsarAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    for(auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear(i, 0, buffer.getNumSamples());

    if(!juce::JUCEApplication::isStandaloneApp())
    {
        //! @todo: This is a temp fix for empty midi messages being passed to the host
        //!       see https://github.com/matthewh806/killing_me_softly_with_his_dsp/issues/45
        //!
        //!       Obviously acquiring a lock on the audio thread is not a good idea
        //!       so i need to come up with a more robust solution
        //!
        //!       Its only a midi buffer so not quite as crucial as audio data, but still...getIncomingMidiBuffer
        //!
        const ScopedLock s1(mMidiMonitorLock);

        auto needsAsyncUpdate = false;
        for(auto const& metaMidi : midiMessages)
        {
            auto const message = metaMidi.getMessage();
            if(message.isNoteOn())
            {
                mIncomingMessages.add(metaMidi.getMessage());
                needsAsyncUpdate = true;
            }
        }

        if(needsAsyncUpdate)
        {
            triggerAsyncUpdate();
        }

        midiMessages.clear();

        // Add all midi messages in the buffer to the output
        // How to do this in a thread safe + lock free way...?
        midiMessages.swapWith(mOutgoingMessages);
    }
}

//==============================================================================
bool PulsarAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

AudioProcessorEditor* PulsarAudioProcessor::createEditor()
{
    return new PulsarAudioProcessorEditor(*this, mDeviceManager);
}

//==============================================================================
void PulsarAudioProcessor::getStateInformation(MemoryBlock& destData)
{
    juce::ignoreUnused(destData);
}

void PulsarAudioProcessor::setStateInformation(const void* data, int sizeInBytes)
{
    juce::ignoreUnused(data, sizeInBytes);
}

//==============================================================================

void PulsarAudioProcessor::sendNoteOnMessage(int noteNumber, float velocity)
{
    //! @todo: Use a midi buffer and do the timestamping properly...?

    auto const relativeStartTimeSeconds = Time::getMillisecondCounterHiRes() * 0.001 - mStartTime;
    auto messageOn = MidiMessage::noteOn(getMidiOutputChannel(), noteNumber, static_cast<uint8>(velocity));
    messageOn.setTimeStamp(relativeStartTimeSeconds);

    auto messageOff = MidiMessage::noteOff(messageOn.getChannel(), messageOn.getNoteNumber());
    messageOff.setTimeStamp(relativeStartTimeSeconds + NOTE_OFF_TIME_MS * 0.001); // lasts 100ms

    const ScopedLock s1(mMidiMonitorLock);
    mOutgoingMessages.addEvent(messageOn, 0);
    mOutgoingMessages.addEvent(messageOff, NOTE_OFF_TIME_MS * static_cast<int>(mSampleRate));

    if(juce::JUCEApplication::isStandaloneApp() && mMidiOutput)
    {
        mMidiOutput->sendBlockOfMessages(mOutgoingMessages, Time::getMillisecondCounterHiRes(), mSampleRate);
        mOutgoingMessages.clear();
    }
}

Physics::PulsarWorld& PulsarAudioProcessor::getWorld()
{
    return mWorld;
}

void PulsarAudioProcessor::updateEditorUI()
{
    auto* editor = getActiveEditor();
    if(editor == nullptr)
    {
        return;
    }

    editor->repaint();
}

int PulsarAudioProcessor::getMidiInputChannel() const
{
    return mMidiInputChannel;
}

void PulsarAudioProcessor::setMidiInputChannel(int channel)
{
    mMidiInputChannel = channel;
}

int PulsarAudioProcessor::getMidiOutputChannel() const
{
    return mMidiOutputChannel;
}

void PulsarAudioProcessor::setMidiOutputChannel(int channel)
{
    mMidiOutputChannel = channel;
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

void PulsarAudioProcessor::handleIncomingMidiMessage(juce::MidiInput* source, const juce::MidiMessage& message)
{
    juce::ignoreUnused(source);

    const ScopedLock s1(mMidiMonitorLock);
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

    for(auto& m : messages)
    {
        if(m.getChannel() != getMidiInputChannel())
        {
            continue;
        }

        if(m.isNoteOn())
        {
            mWorld.spawnBall(m.getNoteNumber(), m.getVelocity());
        }
        else if(m.isController())
        {
            auto const pitchVal = m.getControllerValue();

            // map 0 - 127 to 0 - 360
            auto anglularVelocity = 360.0 / static_cast<double>(127) * static_cast<double>(pitchVal);
            mWorld.setPolygonRotationSpeed(anglularVelocity);
        }
    }
}
