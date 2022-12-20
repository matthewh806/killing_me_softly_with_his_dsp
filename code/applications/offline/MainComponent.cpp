#include "MainComponent.h"

//==============================================================================
MainComponent::MainComponent()
: mStretchFactorSlider("Stretch Factor", "x")
, mPitchShiftSlider("Pitch Shift", "x")
{
    addAndMakeVisible (&mOpenButton);
    mOpenButton.setButtonText ("Open");
    mOpenButton.onClick = [this] { openButtonClicked(); };
    
    addAndMakeVisible(&mSaveButton);
    mSaveButton.setButtonText("Save");
    mSaveButton.onClick = [this] { saveButtonClicked(); };

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
    if(mReaderSource != nullptr)
    {
        mReaderSource->releaseResources();
    }
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
    mSaveButton.setBounds(topButtonBounds.removeFromLeft(twoColumnCompWidth));
    
    bounds.removeFromTop(20);
    auto sliderBounds = bounds.removeFromTop(200);
    mStretchFactorSlider.setBounds(sliderBounds.removeFromLeft(twoColumnCompWidth));
    sliderBounds.removeFromLeft(twoColumnCompSpacing);
    mPitchShiftSlider.setBounds(sliderBounds.removeFromLeft(twoColumnCompWidth));
    
    auto bottomButtonBounds = bounds.removeFromTop(30);
    mPlayButton.setBounds (bottomButtonBounds.removeFromLeft(twoColumnCompWidth));
    bottomButtonBounds.removeFromLeft(twoColumnCompSpacing);
    mStopButton.setBounds (bottomButtonBounds.removeFromLeft(twoColumnCompWidth));
    
    bounds.removeFromTop(20);
    mStretchButton.setBounds(bounds);
}

void MainComponent::openButtonClicked()
{
    if(mTransportSource.isPlaying())
    {
        changeState(Stopping);
    }
    
    mFileChooser = std::make_unique<juce::FileChooser>("Select a Wav file to play...",
                                                       File::getSpecialLocation(juce::File::userHomeDirectory),
                                                       "*.wav");
    
    auto folderChooserFlags = FileBrowserComponent::openMode | FileBrowserComponent::canSelectFiles;
    mFileChooser->launchAsync(folderChooserFlags, [this](const juce::FileChooser& chooser)
    {
        auto file(chooser.getResult());
        auto reader = std::unique_ptr<juce::AudioFormatReader>(mFormatManager.createReaderFor (file));
        if (reader != nullptr)
        {
            mFileBuffer.setSize(static_cast<int>(reader->numChannels), static_cast<int>(reader->lengthInSamples));
            reader->read(&mFileBuffer, 0, static_cast<int>(reader->lengthInSamples), 0, true, true);

            performOfflineStretch();
        }
    });
}

void MainComponent::saveButtonClicked()
{
    mFileChooser = std::make_unique<juce::FileChooser>("Please choose a destination for the file...",
                                                       File::getSpecialLocation(juce::File::userHomeDirectory),
                                                       "*.wav");
    auto folderChooserFlags = FileBrowserComponent::saveMode | FileBrowserComponent::canSelectFiles;
    mFileChooser->launchAsync(folderChooserFlags, [this](const juce::FileChooser& chooser)
    {
        auto file(chooser.getResult());
        if(!mStretchedFile.getFile().copyFileTo(file))
        {
            std::cerr << "Saving file to " << file.getFullPathName() << " failed!\n";
        }
    });
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
    
    mStretchTask = std::make_unique<OfflineStretchProcessor>(mStretchedFile,
                                                             mFileBuffer,
                                                             stretchFactor,
                                                             pitchFactor,
                                                             static_cast<double>(mSampleRate),
                                                             [this]() { stretchComplete(); });
    
    changeState(Stopping);
    mPlayButton.setEnabled(false);
    mStretchButton.setEnabled(false);
    
    mStretchTask->launchThread();
}

void MainComponent::stretchComplete()
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
