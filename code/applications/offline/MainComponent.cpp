#include "MainComponent.h"
#include <RubberBandStretcher.h>

//==============================================================================
MainComponent::MainComponent(juce::AudioDeviceManager& deviceManager)
: mDeviceManager(deviceManager)
, mStretchFactorSlider("Stretch Factor", "x")
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
    mStretchFactorSlider.onValueChange = [this] { stretchValueChanged(); };
    
    addAndMakeVisible(&mPitchShiftSlider);
    mPitchShiftSlider.mLabels.add({0.0, "0.1x"});
    mPitchShiftSlider.mLabels.add({1, "10x"});
    mPitchShiftSlider.setRange(0.1, 10, 0.1);
    mPitchShiftSlider.setValue(1.0);
    mPitchShiftSlider.onValueChange = [this] { pitchShiftValueChanged(); };
    
    setSize (600, 250);
    
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
    auto bounds = getLocalBounds();
    bounds.reduce(20, 20);
    auto buttonBounds = bounds.removeFromTop(20);
    auto const buttonWidth = bounds.getWidth() * 0.25;
    auto const spacing = (bounds.getWidth() - 3*buttonWidth) / 2.0;
    
    mOpenButton.setBounds (buttonBounds.removeFromLeft(buttonWidth));
    buttonBounds.removeFromLeft(spacing);
    mPlayButton.setBounds (buttonBounds.removeFromLeft(buttonWidth));
    buttonBounds.removeFromLeft(spacing);
    mStopButton.setBounds (buttonBounds.removeFromLeft(buttonWidth));
    
    bounds.removeFromTop(20);

    auto const sliderWidth = bounds.getWidth() * 0.4;
    mStretchFactorSlider.setBounds(bounds.removeFromLeft(sliderWidth));
    bounds.removeFromLeft(bounds.getWidth() * 0.2);
    mPitchShiftSlider.setBounds(bounds.removeFromLeft(sliderWidth));
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
    auto const channels = mFileBuffer.getNumChannels();
    if(!mRubberBandStretcher || fileBufferLength <= 0)
    {
        return;
    }
    
    mRubberBandStretcher->reset();
    
    auto const factor = mStretchFactorSlider.getValue();
    mRubberBandStretcher->setTimeRatio(factor);
    mRubberBandStretcher->setPitchScale(mPitchShiftSlider.getValue());
    
    // 1. phase 1 is study
    size_t sample = 0;
    size_t percent = 0;
    
    std::cout << "Phase 1: Studying " << fileBufferLength << " samples\n";
    
    auto constexpr bufferSize = 1024;
    while(sample < fileBufferLength)
    {
        const float* readPtrs[channels];
        for(size_t ch = 0; ch < channels; ++ch)
        {
            readPtrs[ch] = mFileBuffer.getReadPointer(ch, sample);
        }
        
        auto const samplesThisTime = std::min(bufferSize, fileBufferLength - static_cast<int>(sample));
        auto const finalSamples = sample + bufferSize >= fileBufferLength;
        
        std::cout << "Studying " << samplesThisTime << " samples\n";
        if(finalSamples)
        {
            std::cout << " final frames\n";
        }
        mRubberBandStretcher->study(readPtrs, samplesThisTime, finalSamples);
        
        auto const p = static_cast<int>(static_cast<double>(sample) * 100.0 / static_cast<double>(fileBufferLength));
        if(p > percent || sample == 0)
        {
            percent = static_cast<size_t>(p);
            std::cout << "\r" << percent << "%\n";
        }
        
        sample += samplesThisTime;
    }
    
    std::cout << "Phase 1: Studying finished\n";
    
    // 2. phase 2 is stretch
    sample = 0;
    percent = 0;
    
    std::cout << "Phase 2: Stretch Armstrong " << fileBufferLength << " samples\n";
    
    // Create buffer based on estimated size...
    auto stretchedBufferSize = static_cast<int>(fileBufferLength * factor);
    std::cout << "Estimated output buffer size: " << stretchedBufferSize << "\n";
    mStretchedBuffer.setSize(channels, stretchedBufferSize, false, true);
    auto sampleOut = 0;
    
    while(sample < fileBufferLength)
    {
        auto const samplesThistime = std::min(bufferSize, fileBufferLength - static_cast<int>(sample));
        std::cout << "Reading file position: " << sample << ", to " << sample + samplesThistime << "\n";
        const float* readPtrs[channels];
        for(size_t ch = 0; ch < channels; ++ch)
        {
            readPtrs[ch] = mFileBuffer.getReadPointer(ch, sample);
        }
        
        std::cout << "Processing " << samplesThistime << " samples\n";
        auto const finalSamples = sample + bufferSize >= fileBufferLength;
        mRubberBandStretcher->process(readPtrs, samplesThistime, finalSamples);
    
        auto const available = mRubberBandStretcher->available();
        std::cout << "File buffer length: " << fileBufferLength << ", availableSamples: " << available << "\n";
        if(available > 0)
        {
            auto stretchedBuffer = juce::AudioBuffer<float>(channels, available);
            mRubberBandStretcher->retrieve(stretchedBuffer.getArrayOfWritePointers(), available);
            
            if(sampleOut + available > stretchedBufferSize)
            {
                // we need to grow the buffer a bit
                std::cout << "Increasing buffer size: " << stretchedBufferSize << " (orig) to " << (stretchedBufferSize + sampleOut + available) << "\n";
                mStretchedBuffer.setSize(channels, (stretchedBufferSize + sampleOut + available), true);
            }
            
            std::cout << "Writing to stretched buffer from position " << sampleOut << ", to " << sampleOut + available << "\n";
            for(size_t ch = 0; ch < channels; ++ch)
            {
                for(int i = 0; i < available; ++i)
                {
                    auto value = std::max(-1.0f, std::min(1.0f, stretchedBuffer.getSample(ch, i)));
                    mStretchedBuffer.addSample(ch, sampleOut + i, value);
                }
            }
            
            sampleOut += available;
        }
        
        auto const p = static_cast<int>(static_cast<double>(sample) * 100.0 / static_cast<double>(fileBufferLength));
        if(p > percent || sample == 0)
        {
            percent = static_cast<size_t>(p);
            std::cout << "\r" << percent << "%\n";
        }
        
        sample += samplesThistime;
    }
    
    std::cout << "Phase 2: Stretch Armstrong finished\n";
    
    std::cout << "Phase 3: Remaining samples\n";
    std::cout << "Num" << mRubberBandStretcher->available() << "\n";
    while(mRubberBandStretcher->available() >= 0)
    {
        auto const availableSamples = mRubberBandStretcher->available();
        std::cout << "Completing: number remaining: " << availableSamples << "\n";
        auto stretchedBuffer = juce::AudioBuffer<float>(channels, availableSamples);
        mRubberBandStretcher->retrieve(stretchedBuffer.getArrayOfWritePointers(), availableSamples);
        
        if(sampleOut + availableSamples > stretchedBufferSize)
        {
            // we need to grow the buffer a bit
            std::cout << "Increasing buffer size: " << stretchedBufferSize << " (orig) to " << (stretchedBufferSize + sampleOut + availableSamples) << "\n";
            mStretchedBuffer.setSize(channels, (stretchedBufferSize + sampleOut + availableSamples), true);
        }
        
        std::cout << "Writing to stretched buffer from position " << sampleOut << ", to " << sampleOut + availableSamples << "\n";
        for(size_t ch = 0; ch < channels; ++ch)
        {
            for(int i = 0; i < availableSamples; ++i)
            {
                auto value = std::max(-1.0f, std::min(1.0f, stretchedBuffer.getSample(ch, i)));
                mStretchedBuffer.addSample(ch, sampleOut + i, value);
            }
        }
        
        sampleOut += availableSamples;
    }
    std::cout << "Phase 3: Remaining samples complete\n";
        
    std::unique_ptr<juce::MemoryAudioSource> memSrc (new juce::MemoryAudioSource(mStretchedBuffer, false, false));
    mTransportSource.setSource(memSrc.get(), 0, nullptr, mSampleRate);
    mStretchedSrc.reset(memSrc.release());
}
