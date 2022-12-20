#include "MainComponent.h"

//==============================================================================
MainComponent::MainComponent()
{
    addAndMakeVisible (&mOpenButton);
    mOpenButton.setButtonText ("Open");
    mOpenButton.onClick = [this] { openButtonClicked(); };
    
    addAndMakeVisible (&mPlayButton);
    mPlayButton.setButtonText ("Play");
    mPlayButton.onClick = [this] { playButtonClicked(mReaderSource.get()); };
    mPlayButton.setColour (juce::TextButton::buttonColourId, juce::Colours::green);
    mPlayButton.setEnabled (false);
    
    addAndMakeVisible (&mStopButton);
    mStopButton.setButtonText ("Stop");
    mStopButton.onClick = [this] { stopButtonClicked(); };
    mStopButton.setColour (juce::TextButton::buttonColourId, juce::Colours::red);
    mStopButton.setEnabled (false);
    
    addAndMakeVisible(&mPerformFFTButton);
    mPerformFFTButton.setButtonText("Perform FFT");
    mPerformFFTButton.onClick = [this] { performFFT(); };
    mPerformFFTButton.setEnabled(false);
    
    addAndMakeVisible(&mPlayReconstructedSoundButton);
    mPlayReconstructedSoundButton.setButtonText("Play Reconstructed");
    mPlayReconstructedSoundButton.onClick = [this] { playButtonClicked(mReconstructedReaderSource.get()); };
    mPlayReconstructedSoundButton.setColour(juce::TextButton::buttonColourId, juce::Colours::green);
    mPlayReconstructedSoundButton.setEnabled(false);
    
    setSize (600, 250);
    
    mFormatManager.registerBasicFormats();
    mTransportSource.addChangeListener(this);
    
    setAudioChannels (2, 2);
}

MainComponent::~MainComponent()
{
    shutdownAudio();
}   

void MainComponent::prepareToPlay (int samplesPerBlockExpected, double sampleRate)
{
    mBlockSize = samplesPerBlockExpected;
    mSampleRate = static_cast<int>(sampleRate);
    
    mTransportSource.prepareToPlay(samplesPerBlockExpected, sampleRate);
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
    auto topButtonBounds = bounds.removeFromTop(20);
    
    auto const twoColumnCompWidth = static_cast<int>(bounds.getWidth() * 0.46);
    auto const twoColumnCompSpacing = bounds.getWidth() - 2 * twoColumnCompWidth;
    
    mOpenButton.setBounds (topButtonBounds.removeFromLeft(twoColumnCompWidth));
    topButtonBounds.removeFromLeft(twoColumnCompSpacing);
    
    auto middleButtonBounds = bounds.removeFromTop(30);
    mPlayButton.setBounds (middleButtonBounds.removeFromLeft(twoColumnCompWidth));
    middleButtonBounds.removeFromLeft(twoColumnCompSpacing);
    mStopButton.setBounds (middleButtonBounds.removeFromLeft(twoColumnCompWidth));
    
    auto bottomButtonBounds = bounds.removeFromTop(30);
    mPerformFFTButton.setBounds (bottomButtonBounds.removeFromLeft(twoColumnCompWidth));
    bottomButtonBounds.removeFromLeft(twoColumnCompSpacing);
    mPlayReconstructedSoundButton.setBounds (bottomButtonBounds.removeFromLeft(twoColumnCompWidth));
}

void MainComponent::openButtonClicked()
{
    if(mTransportSource.isPlaying())
    {
        changeState(Stopping);
    }
    
    std::unique_ptr<juce::FileChooser> myChooser = std::make_unique<juce::FileChooser>("Select a Wave file to play...",
                                                                                     File::getSpecialLocation(juce::File::userHomeDirectory),
                                                                                     "*.wav");
    
    auto folderChooserFlags = FileBrowserComponent::openMode;
    myChooser->launchAsync(folderChooserFlags, [this](const juce::FileChooser& chooser)
    {
        auto file = chooser.getResult();
        auto reader = std::unique_ptr<juce::AudioFormatReader>(mFormatManager.createReaderFor (file));
        if (reader != nullptr)
        {
            mFileBuffer.setSize(static_cast<int>(reader->numChannels), static_cast<int>(reader->lengthInSamples));
            reader->read(&mFileBuffer, 0, static_cast<int>(reader->lengthInSamples), 0, true, true);
            
            auto newSource = std::make_unique<juce::AudioFormatReaderSource> (reader.release(), true);
            mReaderSource.reset(newSource.release());
            
            mPlayButton.setEnabled(true);
            mPerformFFTButton.setEnabled(true);
        }
    });
}

void MainComponent::playButtonClicked(juce::PositionableAudioSource* audioSource)
{
    if(audioSource == nullptr)
    {
        return;
    }
    
    mTransportSource.setSource(audioSource, 0, nullptr, mSampleRate);
    changeState(Starting);
}

void MainComponent::stopButtonClicked()
{
    changeState(Stopping);
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

void MainComponent::performFFT()
{
    // 1 channel for now (expand to multiple later)
    auto* fileChannelData = mFileBuffer.getReadPointer(0);
    auto const numSamples = mFileBuffer.getNumSamples();
    auto readPosition = 0;
    auto writePosition = 0;
    
    mReconstructedAudioBuffer.setSize(mFileBuffer.getNumChannels(), mFileBuffer.getNumSamples());
    
    auto remainingSamples = numSamples;
    while(remainingSamples > 0)
    {
        auto const samplesToRead = std::min(FFTWrapper::FFTSize, remainingSamples);
        mFFTData.fill(0.0f);
        std::copy(&fileChannelData[0] + readPosition, &fileChannelData[0] + readPosition + samplesToRead, mFFTData.begin());
        
        mFFTWrapper.performRealForwardTransform(mFFTData.data());
        mFFTWrapper.performRealInverseTransform(mFFTData.data());
        
        mReconstructedAudioBuffer.copyFrom(0, writePosition, mFFTData.data(), samplesToRead);
        
        readPosition += samplesToRead;
        writePosition += samplesToRead;
        remainingSamples -= samplesToRead;
    }
    
    auto newSource = std::make_unique<juce::MemoryAudioSource> (mReconstructedAudioBuffer, true);
    mReconstructedReaderSource.reset(newSource.release());
    
    mPlayReconstructedSoundButton.setEnabled(true);
}
