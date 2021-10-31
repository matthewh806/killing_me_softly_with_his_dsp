/*
  ==============================================================================

    This file was auto-generated!

    It contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "Utils.h"

//==============================================================================

PulsarAudioProcessorEditor::PulsarAudioProcessorEditor (PulsarAudioProcessor& p)
: AudioProcessorEditor (&p)
{
    setSize (500, 500);
    
    addKeyListener(this);
    
    addAndMakeVisible(mMidiInputDeviceList);
    addAndMakeVisible(mMidiInputChannelList);
    addAndMakeVisible(mMidiOutputDeviceList);
    addAndMakeVisible(mMidiOutputChannelList);
    
    mMidiInputDeviceList.comboBox.setTextWhenNoChoicesAvailable("No Midi Inputs Enabled");
    auto midiInputs = MidiInput::getAvailableDevices();
    mMidiInputDeviceList.comboBox.onChange = [this]
    {
        auto const index = mMidiInputDeviceList.comboBox.getSelectedItemIndex();
        auto const inputs = MidiInput::getAvailableDevices();
        setMidiInput(inputs[index].identifier);
    };
    
    for(int i = 0; i < midiInputs.size(); ++i)
    {
        mMidiInputDeviceList.comboBox.addItem(midiInputs[i].name, i+1);
        if(mDeviceManager.isMidiInputDeviceEnabled(midiInputs[i].identifier))
        {
            setMidiInput(midiInputs[i].identifier);
            mMidiInputDeviceList.comboBox.setSelectedId(i+1);
        }
    }
    
    for(int i = 1; i <= 16; ++i)
    {
        mMidiInputChannelList.comboBox.addItem(juce::String(i), i);
        mMidiOutputChannelList.comboBox.addItem(juce::String(i), i);
    }
    
    mMidiInputChannelList.comboBox.onChange = [this] {
        mMidiInputChannel = mMidiInputChannelList.comboBox.getSelectedId();
    };
    
    mMidiOutputChannelList.comboBox.onChange = [this] {
        mMidiOutputChannel = mMidiOutputChannelList.comboBox.getSelectedId();
    };
    
    mMidiOutputDeviceList.comboBox.setTextWhenNoChoicesAvailable("No Midi Outputs Enabled");
    auto midiOutputs = MidiOutput::getAvailableDevices();
    mMidiOutputDeviceList.comboBox.onChange = [this] {
        auto const index = mMidiOutputDeviceList.comboBox.getSelectedItemIndex();
        auto const outputs = MidiOutput::getAvailableDevices();
        setMidiOutput(outputs[index].identifier);
    };
    
    for(int i = 0; i < midiOutputs.size(); ++i)
    {
        mMidiOutputDeviceList.comboBox.addItem(midiOutputs[i].name, i+1);
        
        if(mDeviceManager.getDefaultMidiOutputIdentifier() == midiOutputs[i].identifier)
        {
            setMidiOutput(midiOutputs[i].identifier);
            mMidiOutputDeviceList.comboBox.setSelectedId(i+1);
        }
    }
    
    mWorld.setRect({0, 0, Physics::Utils::pixelsToMeters(static_cast<float>(getWidth())), Physics::Utils::pixelsToMeters(static_cast<float>(getHeight()))});
    
    addAndMakeVisible(mNoteStrategyList);
    mNoteStrategyList.comboBox.addItem("random", 1);
    mNoteStrategyList.comboBox.addItem("major", 2);
    mNoteStrategyList.comboBox.addItem("minor", 3);
    mNoteStrategyList.comboBox.setSelectedId(1);
    mNoteStrategyList.comboBox.onChange = [this]()
    {
        auto const strategy = static_cast<NoteStrategy::Strategy>(mNoteStrategyList.comboBox.getSelectedItemIndex());
        mNoteStrategy.setStrategy(strategy);
    };
    
    addAndMakeVisible(mNoteKey);
    mNoteKey.comboBox.addItemList({"C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B"}, 1);
    mNoteKey.comboBox.onChange = [this]()
    {
        auto const noteName = mNoteKey.comboBox.getItemText(mNoteKey.comboBox.getSelectedItemIndex());
        mNoteStrategy.setKey(noteName.toStdString());
    };
}

PulsarAudioProcessorEditor::~PulsarAudioProcessorEditor()
{
    stopTimer();
    removeKeyListener(this);
}

//==============================================================================
void PulsarAudioProcessorEditor::paint (Graphics& g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll (getLookAndFeel().findColour (ResizableWindow::backgroundColourId));
    g.setColour (Colours::white);
    
    g.drawSingleLineText("Number of balls: " + juce::String(mWorld.getNumberOfBalls()), 20, getHeight() - 80);
    
    Box2DRenderer box2DRenderer;
    box2DRenderer.render(g, mWorld.getWorld(), mWorld.getRect().getX(), mWorld.getRect().getY(), mWorld.getRect().getRight(), mWorld.getRect().getBottom(), getLocalBounds().toFloat());
}

void PulsarAudioProcessorEditor::resized()
{
    auto bounds = getLocalBounds();
    bounds.reduced(20, 20);
    
    auto ioBounds = bounds.removeFromTop(40);
    auto midiInputBounds = ioBounds.removeFromTop(static_cast<int>(ioBounds.getHeight() * 0.5));
    auto midiOutputBounds = ioBounds;
    
    mMidiInputDeviceList.setBounds(midiInputBounds.removeFromLeft(static_cast<int>(midiInputBounds.getWidth() * 0.5)));
    mMidiInputChannelList.setBounds(midiInputBounds);
    
    mMidiOutputDeviceList.setBounds(midiOutputBounds.removeFromLeft(static_cast<int>(midiOutputBounds.getWidth() * 0.5)));
    mMidiOutputChannelList.setBounds(midiOutputBounds);
    
    auto scaleBounds = bounds.removeFromBottom(20);
    mNoteStrategyList.setBounds(scaleBounds.removeFromLeft(static_cast<int>(scaleBounds.getWidth() * 0.5)));
    mNoteKey.setBounds(scaleBounds.removeFromLeft(static_cast<int>(scaleBounds.getWidth() * 0.5)));
}

bool PulsarAudioProcessorEditor::keyPressed(juce::KeyPress const& key)
{
    if(key == 82) // r
    {
        mWorld.incrementPolygonRotationSpeed();
    }
    else if (key == 73) // i
    {
        mWorld.increaseEdgeSeparation();
    }
    else if (key == 68) // d
    {
        mWorld.decreaseEdgeSeparation();
    }
    else if(key == KeyPress::numberPad3 || key == 51)
    {
        mWorld.createPolygon(3);
    }
    else if(key == KeyPress::numberPad4 || key == 52)
    {
        mWorld.createPolygon(4);
    }
    else if(key == KeyPress::numberPad5 || key == 53)
    {
        mWorld.createPolygon(5);
    }
    else if(key == KeyPress::numberPad6 || key == 54)
    {
        mWorld.createPolygon(6);
    }
    else if(key == KeyPress::numberPad7 || key == 55)
    {
        mWorld.createPolygon(7);
    }
    else if(key == KeyPress::numberPad8 || key == 56)
    {
        mWorld.createPolygon(8);
    }
    else if(key == KeyPress::numberPad9 || key == 57)
    {
        mWorld.createPolygon(9);
    }
    
    return true;
}

bool PulsarAudioProcessorEditor::keyPressed(juce::KeyPress const& key, juce::Component *originatingComponent)
{
    juce::ignoreUnused(originatingComponent);
    return keyPressed(key);
}

void PulsarAudioProcessorEditor::mouseUp (juce::MouseEvent const& event)
{
    b2Vec2 const worldPos {Physics::Utils::pixelsToMeters(event.position.x), Physics::Utils::pixelsToMeters(event.position.y)};
    if(mWorld.testPointInPolygon(worldPos))
    {
        int const midiNote = mNoteStrategy.getMidiNote();
        int const velocity = mRandom.nextInt(127);
        
        mWorld.spawnBall(midiNote, velocity);
    }
}

void PulsarAudioProcessorEditor::handleIncomingMidiMessage (juce::MidiInput *source, const juce::MidiMessage &message)
{
    juce::ignoreUnused(source);
    
    const ScopedLock s1 (mMidiMonitorLock);
    mIncomingMessages.add(message);
    triggerAsyncUpdate();
}

void PulsarAudioProcessorEditor::handleAsyncUpdate()
{
    // midi message loop
    Array<MidiMessage> messages;
    
    {
        const ScopedLock s1(mMidiMonitorLock);
        messages.swapWith(mIncomingMessages);
    }
    
    for(auto &m : messages)
    {
        if(m.getChannel() != mMidiInputChannel)
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

void PulsarAudioProcessorEditor::sendNoteOnMessage(int noteNumber, float velocity)
{
    if(!mMidiOutput)
    {
        return;
    }
    
    //! @todo: Use a midi buffer and do the timestamping properly...?
    auto messageOn = MidiMessage::noteOn(mMidiOutputChannel, noteNumber, static_cast<uint8>(velocity));
    messageOn.setTimeStamp(Time::getMillisecondCounterHiRes());
    auto messageOff = MidiMessage::noteOff (messageOn.getChannel(), messageOn.getNoteNumber());
    messageOff.setTimeStamp (messageOn.getTimeStamp() + NOTE_OFF_TIME_MS); // lasts 300ms
    
    MidiBuffer buffer;
    buffer.addEvent(messageOn, 0);
    buffer.addEvent(messageOff, static_cast<int>(NOTE_OFF_TIME_MS * 44.1));
    
    mMidiOutput->sendBlockOfMessages(buffer, Time::getMillisecondCounterHiRes(), 44100.0);
}

//==============================================================================
void PulsarAudioProcessorEditor::setMidiInput(String const& identifier)
{
    auto list = MidiInput::getAvailableDevices();
    mDeviceManager.removeMidiInputDeviceCallback(identifier, this);
    
    if(!mDeviceManager.isMidiInputDeviceEnabled(identifier))
    {
        mDeviceManager.setMidiInputDeviceEnabled(identifier, true);
    }
    
    mDeviceManager.addMidiInputDeviceCallback(identifier, this);
}

void PulsarAudioProcessorEditor::setMidiOutput(juce::String const& identifier)
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
