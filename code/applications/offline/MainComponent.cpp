#include "MainComponent.h"

//==============================================================================
MainComponent::MainComponent()
: mStretchFactorSlider("Stretch Factor", "x")
, mPitchShiftSlider("Pitch Shift", "x")
{
    addAndMakeVisible (&mOpenButton);
    mOpenButton.setButtonText ("Open...");
    mOpenButton.onClick = [this] { openButtonClicked(); };

    addAndMakeVisible (&mPlayButton);
    mPlayButton.setButtonText ("Play");
    mPlayButton.onClick = [this] { playButtonClicked(); };
    mPlayButton.setColour (juce::TextButton::buttonColourId, juce::Colours::green);
    mPlayButton.setEnabled (false);

    addAndMakeVisible (&mStopButton);
    mStopButton.setButtonText ("Stop");
    mStopButton.onClick = [this] { stopButtonClicked(); };
    mStopButton.setColour (juce::TextButton::buttonColourId, juce::Colours::red);
    mStopButton.setEnabled (false);
    
    addAndMakeVisible(&mStretchFactorSlider);
    mStretchFactorSlider.mLabels.add({0.0, "0.1x"});
    mStretchFactorSlider.mLabels.add({1, "10x"});
    mStretchFactorSlider.setRange(0.1, 4, 0.1);
    mStretchFactorSlider.setValue(1.0);
    
    addAndMakeVisible(&mPitchShiftSlider);
    mPitchShiftSlider.mLabels.add({0.0, "0.1x"});
    mPitchShiftSlider.mLabels.add({1, "10x"});
    mPitchShiftSlider.setRange(0.1, 10, 0.1);
    mPitchShiftSlider.setValue(1.0);
    
    addAndMakeVisible(&mStretchButton);
    mStretchButton.setButtonText("Stretch!");
    mStretchButton.onClick = [this] { stretchButtonClicked(); };
    mStretchButton.setEnabled(false);
    
    setSize (600, 350);
    
    mFormatManager.registerBasicFormats();
    mTransportSource.addChangeListener(this);
}

MainComponent::~MainComponent()
{
    shutdownAudio();
}

void MainComponent::prepareToPlay (int samplesPerBlockExpected, double sampleRate)
{
    mBlockSize = samplesPerBlockExpected;
    mSampleRate = static_cast<int>(sampleRate);
    
    mTransportSource.prepareToPlay (samplesPerBlockExpected, sampleRate);
}

void MainComponent::getNextAudioBlock (const AudioSourceChannelInfo& bufferToFill)
{
    if (mReaderSource.get() == nullptr)
    {
        bufferToFill.clearActiveBufferRegion();
        return;
    }
    
    mTransportSource.getNextAudioBlock(bufferToFill);
}

void MainComponent::releaseResources()
{
    mReaderSource->releaseResources();
    mTransportSource.releaseResources();
}


//==============================================================================
void MainComponent::paint (juce::Graphics& g)
{
    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));
}

void MainComponent::resized()
{
    auto bounds = getLocalBounds();
    bounds.reduce(20, 20);
    auto buttonBounds = bounds.removeFromTop(20);
    auto const buttonWidth = static_cast<int>(bounds.getWidth() * 0.25);
    auto const spacing = (bounds.getWidth() - 3*buttonWidth) / 2;
    
    mOpenButton.setBounds (buttonBounds.removeFromLeft(buttonWidth));
    buttonBounds.removeFromLeft(spacing);
    mPlayButton.setBounds (buttonBounds.removeFromLeft(buttonWidth));
    buttonBounds.removeFromLeft(spacing);
    mStopButton.setBounds (buttonBounds.removeFromLeft(buttonWidth));
    
    bounds.removeFromTop(20);
    
    auto sliderBounds = bounds.removeFromTop(200);
    auto const sliderWidth = static_cast<int>(sliderBounds.getWidth() * 0.4);
    mStretchFactorSlider.setBounds(sliderBounds.removeFromLeft(sliderWidth));
    sliderBounds.removeFromLeft(static_cast<int>(sliderBounds.getWidth() * 0.2));
    mPitchShiftSlider.setBounds(sliderBounds.removeFromLeft(sliderWidth));
    
    bounds.removeFromTop(20);
    mStretchButton.setBounds(bounds);
}

void MainComponent::openButtonClicked()
{
    if(mTransportSource.isPlaying())
    {
        changeState(Stopping);
    }
    
    juce::FileChooser chooser ("Select a Wave file to play...",
                               {},
                               "*.wav");

    if (chooser.browseForFileToOpen())
    {
        auto file = chooser.getResult();
        auto reader = std::unique_ptr<juce::AudioFormatReader>(mFormatManager.createReaderFor (file));
        if (reader != nullptr)
        {
            mFileBuffer.setSize(static_cast<int>(reader->numChannels), static_cast<int>(reader->lengthInSamples));
            reader->read(&mFileBuffer, 0, static_cast<int>(reader->lengthInSamples), 0, true, true);
            
            performOfflineStretch();
        }
    }
}

void MainComponent::playButtonClicked()
{
    changeState(Starting);
}

void MainComponent::stopButtonClicked()
{
    changeState(Stopping);
}

void MainComponent::stretchButtonClicked()
{
    changeState(Stopping);
    performOfflineStretch();
}

void MainComponent::changeState(TransportState newState)
{
    if (mState != newState)
    {
        mState = newState;

        switch (mState)
        {
            case Stopped:
                mStopButton.setEnabled (false);
                mPlayButton.setEnabled (true);
                mTransportSource.setPosition (0.0);
                break;

            case Starting:
                mPlayButton.setEnabled (false);
                mTransportSource.start();
                break;

            case Playing:
                mStopButton.setEnabled (true);
                break;

            case Stopping:
                mTransportSource.stop();
                break;
        }
    }
}

void MainComponent::changeListenerCallback(juce::ChangeBroadcaster* source)
{
    if (source == &mTransportSource)
    {
        if (mTransportSource.isPlaying())
            changeState (Playing);
        else
            changeState (Stopped);
    }
}

void MainComponent::performOfflineStretch()
{
    auto const stretchFactor = static_cast<float>(mStretchFactorSlider.getValue());
    auto const pitchFactor = static_cast<float>(mPitchShiftSlider.getValue());
    
    OfflineStretchProcessor stretchTask(mStretchedFile, mFileBuffer, stretchFactor, pitchFactor, static_cast<double>(mSampleRate));
    
    changeState(Stopping);
    mPlayButton.setEnabled(false);
    mStretchButton.setEnabled(false);

    if (stretchTask.runThread())
    {
        std::cout << "Written to: " << mStretchedFile.getFile().getFullPathName() << "\n";
        
        auto reader = std::unique_ptr<juce::AudioFormatReader>(mFormatManager.createReaderFor (mStretchedFile.getFile()));
        if (reader != nullptr)
        {
            mPlayButton.setEnabled (true);
            mStretchButton.setEnabled(true);
         
            auto newSource = std::make_unique<juce::AudioFormatReaderSource> (reader.release(), true);
            mTransportSource.setSource(newSource.get(), 0, nullptr, mSampleRate);
            mReaderSource.reset(newSource.release());
        }
    }
}
