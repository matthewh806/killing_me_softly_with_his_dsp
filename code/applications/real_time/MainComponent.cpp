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
    
    mInput.setSize(2, samplesPerBlockExpected);
    
    mOutputBuffer = new RubberBand::RingBuffer<float> *[2];
    auto bufferSize = samplesPerBlockExpected + 1024 + 8192;
    mOutputBuffer[0] = new RubberBand::RingBuffer<float>(bufferSize);
    mOutputBuffer[1] = new RubberBand::RingBuffer<float>(bufferSize);
    
    mScratchBuffer.setSize(2, bufferSize);
    
    mTransportSource.prepareToPlay (samplesPerBlockExpected, sampleRate);
    
    RubberBand::RubberBandStretcher::Options ops = RubberBand::RubberBandStretcher::OptionProcessRealTime | RubberBand::RubberBandStretcher::OptionPitchHighConsistency;
    std::unique_ptr<RubberBand::RubberBandStretcher> newBand (new RubberBand::RubberBandStretcher(static_cast<size_t>(sampleRate), static_cast<size_t>(2), ops));
    mRubberBand.reset(newBand.release());
    //mRubberBand->setMaxProcessSize(samplesPerBlockExpected);
    mRubberBand->setTimeRatio(8.0);
    //mRubberBand->setPitchScale(2.0);
//    mRubberBand->setDebugLevel(3);
}

void MainComponent::getNextAudioBlock (const AudioSourceChannelInfo& bufferToFill)
{
    bufferToFill.clearActiveBufferRegion();
    
    if(mReaderSource == nullptr)
    {
        // no file loadeed
        return;
    }
    
    auto const samples = bufferToFill.numSamples;
    auto processedSamples = 0;
    
    mInput.clear();
    if(mState != Stopped)
    {
        // fill input buffer with new samples
        auto as = AudioSourceChannelInfo(&mInput, 0, samples);
        mTransportSource.getNextAudioBlock(as);
    
        while(processedSamples < samples)
        {
            // Provide new samples and process
            auto required = mRubberBand->getSamplesRequired();
            auto inChunk = std::min(samples - processedSamples, static_cast<int>(required));
            
            const float* readPtrs[2] = {mInput.getReadPointer(0, processedSamples), mInput.getReadPointer(1, processedSamples)};
            mRubberBand->process(readPtrs, static_cast<size_t>(inChunk), false);
            processedSamples += inChunk;
            
            // write into output buffer
            auto availableSamples = mRubberBand->available();
            auto writableSamples = mOutputBuffer[0]->getWriteSpace();
            
            if(writableSamples == 0)
            {
                std::cout << "OutputBuffer is full! - no space left to write" << "\n";
            }
            
            auto outChunk = std::min(availableSamples, writableSamples);
            
            float* writePtrs[2] = {mScratchBuffer.getWritePointer(0, 0), mScratchBuffer.getWritePointer(1, 0)};
            auto retrieved = mRubberBand->retrieve(writePtrs, static_cast<size_t>(outChunk));
            
            std::cout << "available: " << availableSamples << ", outChunk: " << outChunk;
            if(retrieved != outChunk)
            {
                std::cout << " (" << retrieved << ")";
            }
            std::cout << "\n";
            
            outChunk = static_cast<int>(retrieved);
            
            for(size_t c = 0; c < 2; ++c)
            {
                if(mOutputBuffer    [c]->getWriteSpace() < outChunk)
                {
                    std::cerr << "RubberBandPitchShifter::runImpl: buffer overrun: chunk = "
                        << outChunk << ", space = " << mOutputBuffer[c]->getWriteSpace() << std::endl;
                }
                mOutputBuffer[c]->write(mScratchBuffer.getReadPointer(static_cast<int>(c)), outChunk);
            }
        }
    }
    
    // Finally read back the data from the output buffer
    for(size_t c = 0; c < 2; ++c)
    {
        auto const toRead = mOutputBuffer[c]->getReadSpace();
        if(toRead < samples && c == 0)
        {
            std::cerr << "RubberBandPitchShifter::runImpl: buffer underrun: required = " << samples << ", available = " << toRead << std::endl;
        }
        
        auto chunk = std::min(toRead, samples);
        mOutputBuffer[c]->read(bufferToFill.buffer->getWritePointer(c), chunk);
    }
    
    if (mMinfill == 0)
    {
        mMinfill = mOutputBuffer[0]->getReadSpace();
    }
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
    juce::FileChooser chooser ("Select a Wave file to play...",
                               {},
                               "*.wav");

    if (chooser.browseForFileToOpen())
    {
        auto file = chooser.getResult();
        auto* reader = mFormatManager.createReaderFor (file);

        if (reader != nullptr)
        {
            std::unique_ptr<juce::AudioFormatReaderSource> newSource (new juce::AudioFormatReaderSource (reader, true));
            mTransportSource.setSource (newSource.get(), 0, nullptr, reader->sampleRate);
            mPlayButton.setEnabled (true);
            mReaderSource.reset (newSource.release());
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
    // TODO: Not thread safe...
    mRubberBand->setTimeRatio(mStretchFactorSlider.getValue());
}

void MainComponent::pitchShiftValueChanged()
{
    mRubberBand->setPitchScale(mPitchShiftSlider.getValue());
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
                mRubberBand->reset();
                mOutputBuffer[0]->reset();
                mOutputBuffer[1]->reset();
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
