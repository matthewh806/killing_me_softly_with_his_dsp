#include "MainComponent.h"
#include <RubberBandStretcher.h>

//==============================================================================
MainComponent::MainComponent()
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
    mStretchFactorSlider.setRange(0.1, 4, 0.1);
    mStretchFactorSlider.setValue(1.0);
    mStretchFactorSlider.onValueChange = [this] { stretchValueChanged(); };
    
    addAndMakeVisible(&mStretchFactorLabel);
    mStretchFactorLabel.setText("Stretch Factor", juce::NotificationType::dontSendNotification);
    mStretchFactorLabel.attachToComponent(&mStretchFactorSlider, true);
    
    addAndMakeVisible(&mPitchShiftSlider);
    mPitchShiftSlider.setRange(0.1, 10, 0.1);
    mPitchShiftSlider.setValue(1.0);
    mPitchShiftSlider.onValueChange = [this] { pitchShiftValueChanged(); };
    
    addAndMakeVisible(&mPitchShiftLabel);
    mPitchShiftLabel.setText("Pitch Shift", juce::NotificationType::dontSendNotification);
    mPitchShiftLabel.attachToComponent(&mPitchShiftSlider, true);
    
    setSize (600, 400);
    
    mFormatManager.registerBasicFormats();
    mTransportSource.addChangeListener(this);
    setAudioChannels (0, 2);
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
    
    std::unique_ptr<RubberBand::RubberBandStretcher> newBand (new RubberBand::RubberBandStretcher(static_cast<size_t>(sampleRate), static_cast<size_t>(2)));
    mRubberBandStretcher.reset(newBand.release());
}

void MainComponent::getNextAudioBlock (const AudioSourceChannelInfo& bufferToFill)
{
    if (mStretchedSrc.get() == nullptr)
    {
        bufferToFill.clearActiveBufferRegion();
        return;
    }
    
    mTransportSource.getNextAudioBlock(bufferToFill);
}

void MainComponent::releaseResources()
{
    mTransportSource.releaseResources();
}


//==============================================================================
void MainComponent::paint (juce::Graphics& g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));
}

void MainComponent::resized()
{
    mOpenButton.setBounds (10, 10, getWidth() - 20, 20);
    mPlayButton.setBounds (10, 40, getWidth() - 20, 20);
    mStopButton.setBounds (10, 70, getWidth() - 20, 20);
    mStretchFactorLabel.setSize(100, 20);
    mStretchFactorSlider.setBounds(mStretchFactorLabel.getWidth() + 10, 100, getWidth() - 20, 20);
    mPitchShiftLabel.setSize(100, 20);
    mPitchShiftSlider.setBounds(mPitchShiftLabel.getWidth() + 10, 130, getWidth() - 20, 20);
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
            
            // stretch source...!
            performOfflineStretch();
            
            mPlayButton.setEnabled (true);
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

void MainComponent::stretchValueChanged()
{
    changeState(Stopping);
    performOfflineStretch();
}

void MainComponent::pitchShiftValueChanged()
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
    auto const fileBufferLength = mFileBuffer.getNumSamples();
    
    if(!mRubberBandStretcher || fileBufferLength <= 0)
    {
        return;
    }
    
    mRubberBandStretcher->reset();
    
    auto const factor = mStretchFactorSlider.getValue();
    mRubberBandStretcher->setTimeRatio(factor);
    mRubberBandStretcher->setPitchScale(mPitchShiftSlider.getValue());
    
    // 1. phase 1 is study
    size_t frame = 0;
    size_t percent = 0;
    
    // For now pass through in one big chunk
    //while(frame < fileBufferLength)
    //{
    mRubberBandStretcher->study(mFileBuffer.getArrayOfReadPointers(), fileBufferLength, true);
    mRubberBandStretcher->process(mFileBuffer.getArrayOfReadPointers(), fileBufferLength, true);
    
    std::cout << "Buf length: " << fileBufferLength << ", availableSamples: " << mRubberBandStretcher->available() << "\n";
    
    auto const available = mRubberBandStretcher->available();
    if(available > 0)
    {
        auto stretchedBuffer = juce::AudioBuffer<float>(2, available);
        mRubberBandStretcher->retrieve(stretchedBuffer.getArrayOfWritePointers(), available);
        
        std::unique_ptr<juce::MemoryAudioSource> memSrc (new juce::MemoryAudioSource(stretchedBuffer, false, false));
        mTransportSource.setSource(memSrc.get(), 0, nullptr, mSampleRate);
        mStretchedSrc.reset(memSrc.release());
    }
    
    //}
}
