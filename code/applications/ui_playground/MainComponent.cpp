#include "MainComponent.h"
#include "PixelArtBinaryData.h"

using namespace OUS;

//==============================================================================
MainComponent::MainComponent(juce::AudioDeviceManager& deviceManager)
: juce::AudioAppComponent(deviceManager)
{
    setSize(600, 400);
    setAudioChannels(2, 2);

    addAndMakeVisible(mTitleLabel);
    mTitleLabel.setText("UI Playground", juce::NotificationType::dontSendNotification);
    mTitleLabel.setJustificationType(juce::Justification::centred);
    
    addAndMakeVisible(mSyncButton);
    addAndMakeVisible(mPlayButton);
    addAndMakeVisible(mPauseButton);
    addAndMakeVisible(mRecordButton);
    
    addAndMakeVisible(mSelectorComponent);
    mSelectorComponent.setText("CHOP", juce::NotificationType::dontSendNotification);
    mSelectorComponent.addItem("Manual", 1);
    mSelectorComponent.addItem("Threshold", 2);
    mSelectorComponent.addItem("Division", 3);
}

MainComponent::~MainComponent()
{
    shutdownAudio();
}

void MainComponent::prepareToPlay(int samplesPerBlockExpected, double sampleRate)
{
    mBlockSize = samplesPerBlockExpected;
    mSampleRate = static_cast<int>(sampleRate);
}

void MainComponent::getNextAudioBlock(const AudioSourceChannelInfo& bufferToFill)
{
    bufferToFill.clearActiveBufferRegion();
}

void MainComponent::releaseResources()
{
}

//==============================================================================
void MainComponent::paint(juce::Graphics& g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll(getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId));
}

void MainComponent::resized()
{
    auto bounds = getLocalBounds().reduced(20, 20);
    mTitleLabel.setBounds(bounds.removeFromTop(50));
    bounds.removeFromTop(10);
    mSyncButton.setBounds(bounds.removeFromTop(60));
    bounds.removeFromTop(10);
    
    auto transportButtonBounds = bounds.removeFromTop(60);
    auto const width = transportButtonBounds.getWidth();
    auto const transportButtonWidth = static_cast<int>(width * 0.25f);
    auto const transportButtonSpacing = static_cast<int>(width * 0.125f);
    mPlayButton.setBounds(transportButtonBounds.removeFromLeft(transportButtonWidth));
    transportButtonBounds.removeFromLeft(transportButtonSpacing);
    mPauseButton.setBounds(transportButtonBounds.removeFromLeft(transportButtonWidth));
    transportButtonBounds.removeFromLeft(transportButtonSpacing);
    mRecordButton.setBounds(transportButtonBounds.removeFromLeft(transportButtonWidth));
    bounds.removeFromTop(10);
    
    mSelectorComponent.setBounds(bounds.removeFromTop(30).removeFromLeft(200));
}
