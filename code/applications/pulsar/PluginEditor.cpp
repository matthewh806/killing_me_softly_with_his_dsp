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

PulsarAudioProcessorEditor::PulsarAudioProcessorEditor (PulsarAudioProcessor& p, AudioDeviceManager& deviceManager)
: AudioProcessorEditor (&p)
, mDeviceManager(deviceManager)
{
    mInformationScreen.setSize(300, 300);
    setSize (500, 500);
    
    addKeyListener(this);
    
    auto& pulsarProcessor = static_cast<PulsarAudioProcessor&>(processor);
    if(juce::JUCEApplication::isStandaloneApp())
    {
        addAndMakeVisible(mMidiInputDeviceList);
        addAndMakeVisible(mMidiInputChannelList);
        addAndMakeVisible(mMidiOutputDeviceList);
        addAndMakeVisible(mMidiOutputChannelList);
        
        mMidiInputDeviceList.comboBox.setTextWhenNoChoicesAvailable("No Midi Inputs Enabled");
        auto midiInputs = MidiInput::getAvailableDevices();
        mMidiInputDeviceList.comboBox.onChange = [&]
        {
            auto const index = mMidiInputDeviceList.comboBox.getSelectedItemIndex();
            auto const inputs = MidiInput::getAvailableDevices();
            pulsarProcessor.setMidiInput(inputs[index].identifier);
        };
        
        for(int i = 0; i < midiInputs.size(); ++i)
        {
            mMidiInputDeviceList.comboBox.addItem(midiInputs[i].name, i+1);
            if(mDeviceManager.isMidiInputDeviceEnabled(midiInputs[i].identifier))
            {
                static_cast<PulsarAudioProcessor&>(processor).setMidiInput(midiInputs[i].identifier);
                mMidiInputDeviceList.comboBox.setSelectedId(i+1);
            }
        }
        
        for(int i = 1; i <= 16; ++i)
        {
            mMidiInputChannelList.comboBox.addItem(juce::String(i), i);
            mMidiOutputChannelList.comboBox.addItem(juce::String(i), i);
        }
        
        mMidiInputChannelList.comboBox.onChange = [&] {
            pulsarProcessor.setMidiInputChannel(mMidiInputChannelList.comboBox.getSelectedId());
        };
        
        mMidiOutputChannelList.comboBox.onChange = [&] {
            pulsarProcessor.setMidiOutputChannel(mMidiOutputChannelList.comboBox.getSelectedId());
        };
        
        mMidiOutputDeviceList.comboBox.setTextWhenNoChoicesAvailable("No Midi Outputs Enabled");
        auto midiOutputs = MidiOutput::getAvailableDevices();
        mMidiOutputDeviceList.comboBox.onChange = [&] {
            auto const index = mMidiOutputDeviceList.comboBox.getSelectedItemIndex();
            auto const outputs = MidiOutput::getAvailableDevices();
            pulsarProcessor.setMidiOutput(outputs[index].identifier);
        };
        
        for(int i = 0; i < midiOutputs.size(); ++i)
        {
            mMidiOutputDeviceList.comboBox.addItem(midiOutputs[i].name, i+1);
            
            if(mDeviceManager.getDefaultMidiOutputIdentifier() == midiOutputs[i].identifier)
            {
                pulsarProcessor.setMidiOutput(midiOutputs[i].identifier);
                mMidiOutputDeviceList.comboBox.setSelectedId(i+1);
            }
        }
    }
    else
    {
        pulsarProcessor.setMidiInputChannel(1);
        pulsarProcessor.setMidiOutputChannel(1);
    }
    
    pulsarProcessor.getWorld().setRect({0, 0, Physics::Utils::pixelsToMeters(static_cast<float>(getWidth())), Physics::Utils::pixelsToMeters(static_cast<float>(getHeight()))});
    
    addAndMakeVisible(mNoteStrategyList);
    mNoteStrategyList.comboBox.addItem("Random", 1);
    mNoteStrategyList.comboBox.addItem("Major", 2);
    mNoteStrategyList.comboBox.addItem("Minor", 3);
    mNoteStrategyList.comboBox.addItem("HarmonicMinor", 4);
    mNoteStrategyList.comboBox.addItem("MelodicMinor", 5);
    mNoteStrategyList.comboBox.addItem("MinorPentatonic", 6);
    mNoteStrategyList.comboBox.addItem("MajorPentatonic", 7);
    mNoteStrategyList.comboBox.addItem("Dorian", 8);
    mNoteStrategyList.comboBox.addItem("Phrygian", 9);
    mNoteStrategyList.comboBox.addItem("Lydian", 10);
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
    removeKeyListener(this);
}

//==============================================================================

void PulsarAudioProcessorEditor::paint (Graphics& g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll (getLookAndFeel().findColour (ResizableWindow::backgroundColourId));
    g.setColour (Colours::white);
    
    g.drawSingleLineText("Number of balls: " + juce::String(static_cast<PulsarAudioProcessor&>(processor).getWorld().getNumberOfBalls()), 20, getHeight() - 80);
    
    auto& pulsarProcessor = static_cast<PulsarAudioProcessor&>(processor);
    Box2DRenderer box2DRenderer;
    box2DRenderer.render(g,
                         pulsarProcessor.getWorld().getWorld(),
                         pulsarProcessor.getWorld().getRect().getX(),
                         pulsarProcessor.getWorld().getRect().getY(),
                         pulsarProcessor.getWorld().getRect().getRight(),
                         pulsarProcessor.getWorld().getRect().getBottom(),
                         getLocalBounds().toFloat());
}

void PulsarAudioProcessorEditor::resized()
{
    auto bounds = getLocalBounds();
    bounds.reduced(20, 20);
    
    if(juce::JUCEApplication::isStandaloneApp())
    {
        auto ioBounds = bounds.removeFromTop(40);
        auto midiInputBounds = ioBounds.removeFromTop(static_cast<int>(ioBounds.getHeight() * 0.5));
        auto midiOutputBounds = ioBounds;
        
        mMidiInputDeviceList.setBounds(midiInputBounds.removeFromLeft(static_cast<int>(midiInputBounds.getWidth() * 0.5)));
        mMidiInputChannelList.setBounds(midiInputBounds);
        
        mMidiOutputDeviceList.setBounds(midiOutputBounds.removeFromLeft(static_cast<int>(midiOutputBounds.getWidth() * 0.5)));
        mMidiOutputChannelList.setBounds(midiOutputBounds);
    }
    
    auto scaleBounds = bounds.removeFromBottom(20);
    mNoteStrategyList.setBounds(scaleBounds.removeFromLeft(static_cast<int>(scaleBounds.getWidth() * 0.5)));
    mNoteKey.setBounds(scaleBounds.removeFromLeft(static_cast<int>(scaleBounds.getWidth() * 0.5)));
}

bool PulsarAudioProcessorEditor::keyPressed(juce::KeyPress const& key)
{
    auto& pulsarProcessor = static_cast<PulsarAudioProcessor&>(processor);
    if(key == 82) // r
    {
        pulsarProcessor.getWorld().incrementPolygonRotationSpeed();
    }
    else if (key == 73) // i
    {
        pulsarProcessor.getWorld().increaseEdgeSeparation();
    }
    else if (key == 68) // d
    {
        pulsarProcessor.getWorld().decreaseEdgeSeparation();
    }
    else if(key == 72) // h
    {
        // show control screen
        showInformationScreen();
    }
    else if(key == KeyPress::numberPad3 || key == 51)
    {
        pulsarProcessor.getWorld().createPolygon(3);
    }
    else if(key == KeyPress::numberPad4 || key == 52)
    {
        pulsarProcessor.getWorld().createPolygon(4);
    }
    else if(key == KeyPress::numberPad5 || key == 53)
    {
        pulsarProcessor.getWorld().createPolygon(5);
    }
    else if(key == KeyPress::numberPad6 || key == 54)
    {
        pulsarProcessor.getWorld().createPolygon(6);
    }
    else if(key == KeyPress::numberPad7 || key == 55)
    {
        pulsarProcessor.getWorld().createPolygon(7);
    }
    else if(key == KeyPress::numberPad8 || key == 56)
    {
        pulsarProcessor.getWorld().createPolygon(8);
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
    if(static_cast<PulsarAudioProcessor&>(processor).getWorld().testPointInPolygon(worldPos))
    {
        int const midiNote = mNoteStrategy.getMidiNote();
        int const velocity = mRandom.nextInt(127);
        
        static_cast<PulsarAudioProcessor&>(processor).getWorld().spawnBall(midiNote, velocity);
    }
}

void PulsarAudioProcessorEditor::showInformationScreen()
{
    juce::DialogWindow::LaunchOptions o;
    o.dialogTitle = juce::translate("Pulsar Information");
    
    o.content.setNonOwned(&mInformationScreen);
    o.componentToCentreAround = nullptr;
    o.dialogBackgroundColour = juce::Desktop::getInstance().getDefaultLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId);
    o.escapeKeyTriggersCloseButton = true;
    o.useNativeTitleBar = false;
    o.resizable = false;
    o.useBottomRightCornerResizer = false;
    
    o.launchAsync();
}
