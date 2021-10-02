#pragma once

#include <iostream>
#include "../../core/ReferenceCountedForwardAndReverseBuffer.h"
#include "../../ui/CustomLookAndFeel.h"
#include "BreakbeatAudioSource.h"
#include "../../utils/FileRecorder.h"
#include "SliceExporter.h"

//==============================================================================

class SliceRotarySlider : public RotarySliderWithLabels
{
public:
    using RotarySliderWithLabels::RotarySliderWithLabels;
    
    juce::String getDisplayString() const override
    {
        return juce::String(std::pow(2.0, getValue()), 0);
    }
};

class BreakbeatContentComponent
: public juce::AudioAppComponent
, private juce::Thread
, private juce::ChangeListener
, private juce::AsyncUpdater
{
public:
    enum ColourIds
    {
        backgroundColourId      = 0x3004000,
        playingButtonColourId   = 0x3004001,
        recordingButtonColourId = 0x3004003,
        defaultButtonColourId   = 0x3004002
    };
    
    BreakbeatContentComponent(juce::AudioDeviceManager& audioDeviceManager, juce::RecentlyOpenedFilesList& recentFiles);
    ~BreakbeatContentComponent() override;
    
    // juce::Component
    void resized() override;
    void paint(juce::Graphics& g) override;
    void lookAndFeelChanged() override;
    
    // juce::AudioAppComponent
    void prepareToPlay (int, double) override;
    void getNextAudioBlock (const AudioSourceChannelInfo& bufferToFill) override;
    void releaseResources() override;
    
    // juce::Thread
    void run() override;
    
    // juce::ChangeListener
    void changeListenerCallback(juce::ChangeBroadcaster* source) override;
    
    // juce::AsyncUpdater
    void handleAsyncUpdate() override;
    
    void newFileOpened(String& filePath);
    void setFileOutputPath();
    void exportAudioSlices();

private:
    enum class TransportState
    {
        Stopped,
        Starting,
        Playing,
        Stopping
    };
    
    class WaveformComponent
    : public juce::Component
    , public juce::FileDragAndDropTarget
    , private juce::AsyncUpdater
    {
    public:
        WaveformComponent(BreakbeatContentComponent& parent, juce::AudioFormatManager& formatManager);
        ~WaveformComponent() override;
        
        juce::AudioThumbnail& getThumbnail();
        
        void setSlicePositions(std::vector<size_t> const& slicePositions, size_t activeSliceIndex);
        void setActiveSlice(size_t sliceIndex);
        
        void clear();
        
        // juce::Component
        void resized() override;
        void paint(juce::Graphics& g) override;
        
        // juce::FileDragAndDropTarget
        bool isInterestedInFileDrag (const StringArray& files) override;
        void filesDropped (const StringArray& files, int x, int y) override;
        
        // juce::AsyncUpdater
        void handleAsyncUpdate() override;
        
    private:
        BreakbeatContentComponent& mParentComponent;
        
        juce::AudioFormatManager& mAudioFormatManager;
        juce::AudioThumbnailCache mThumbnailCache;
        juce::AudioThumbnail mThumbnail;
        
        std::vector<size_t> mSlicePositions;
        size_t mActiveSliceIndex {0};
        
        double mSampleRate = 44100.0;
    };
    
    void checkForBuffersToFree();
    void checkForPathToOpen();
    
    void changeState(TransportState state);
    
    void updateWaveform();
    //==========================================================================
    
    ComboBoxWithLabel mSliceTypeCombobox {"Slice type"};
    
    RotarySliderWithLabels mPitchShiftSlider;
    RotarySliderWithLabels mCrossFadeSlider;
    SliceRotarySlider mSliceDivsorSlider;
    RotarySliderWithLabels mSliceTransientThresholdSlider;
    RotarySliderWithLabels mChangeSampleProbabilitySlider;
    RotarySliderWithLabels mReverseSampleProbabilitySlider;
    RotarySliderWithLabels mRetriggerSampleProbabilitySlider;
    
    juce::Label mFileNameLabel;
    juce::Label mFileSampleRateLabel;
    
    NumberFieldWithLabel mSampleLengthSeconds {"Original Length", "s", 3, false};
    NumberFieldWithLabel mSampleDesiredLengthSeconds {"New Length", "s", 3, true};
    
    juce::TextButton mStopButton;
    juce::TextButton mPlayButton;
    juce::TextButton mRecordButton;
    
    TransportState mState;
    
    juce::RecentlyOpenedFilesList& mRecentFiles;
    juce::AudioFormatManager mFormatManager;
    
    BreakbeatAudioSource mAudioSource {mFormatManager};
    juce::AudioTransportSource mTransportSource;
    
    std::vector<float*> mTemporaryChannels;
    
    FileRecorder mRecorder {mFormatManager};
    bool mRecording = false;
    
    SliceExporter mSliceExporter {mFormatManager};
    
    WaveformComponent mWaveformComponent { *this, mFormatManager };
    
    juce::String mChosenPath;
    
    float mSampleChangeThreshold = 0.7f;
    float mReverseSampleThreshold = 0.7f;
    float mRetriggerSampleThreshold = 0.7f;
    
    juce::File mRecordedFile {juce::File::getSpecialLocation(juce::File::SpecialLocationType::tempDirectory).getChildFile("toous").getChildFile("temp_recording.wav")};

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (BreakbeatContentComponent)
};
