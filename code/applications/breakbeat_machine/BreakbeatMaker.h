#pragma once

#include <iostream>
#include "../../core/ReferenceCountedForwardAndReverseBuffer.h"
#include "../../ui/CustomLookAndFeel.h"
#include "BreakbeatAudioSource.h"
#include "../../utils/FileRecorder.h"
#include "SliceExporter.h"
//==============================================================================

#define MAX_FILE_LENGTH 15.0 // seconds

class SliceRotarySlider : public RotarySliderWithLabels
{
public:
    using RotarySliderWithLabels::RotarySliderWithLabels;
    
    juce::String getDisplayString() const override
    {
        return juce::String(std::pow(2.0, getValue()), 0);
    }
};

class MainContentComponent
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
    
    MainContentComponent(juce::AudioDeviceManager& audioDeviceManager, juce::RecentlyOpenedFilesList& recentFiles);
    ~MainContentComponent() override;
    
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
        WaveformComponent(MainContentComponent& parent, juce::AudioFormatManager& formatManager);
        ~WaveformComponent() override;
        
        juce::AudioThumbnail& getThumbnail();
        
        void setSampleStartEnd(int64_t start, int64_t end);
        
        // juce::Component
        void resized() override;
        void paint(juce::Graphics& g) override;
        
        // juce::FileDragAndDropTarget
        bool isInterestedInFileDrag (const StringArray& files) override;
        void filesDropped (const StringArray& files, int x, int y) override;
        
        // juce::AsyncUpdater
        void handleAsyncUpdate() override;
        
    private:
        MainContentComponent& mParentComponent;
        
        juce::AudioFormatManager& mAudioFormatManager;
        juce::AudioThumbnailCache mThumbnailCache;
        juce::AudioThumbnail mThumbnail;
        
        int64_t mStartSample = 0;
        int64_t mEndSample = 0;
        
        double mSampleRate = 44100.0;
    };
    
    void checkForBuffersToFree();
    void checkForPathToOpen();
    
    void changeState(TransportState state);
    //==========================================================================
    NumberFieldWithLabel mSampleBpmField {"BPM", "", true};
    
    SliceRotarySlider mSliceDivsorSlider;
    RotarySliderWithLabels mChangeSampleProbabilitySlider;
    RotarySliderWithLabels mReverseSampleProbabilitySlider;
    
    juce::Label mFileNameLabel;
    juce::Label mFileSampleRateLabel;
    
    juce::TextButton mStopButton;
    juce::TextButton mPlayButton;
    juce::TextButton mRecordButton;
        
    juce::String mFileName;
    double mFileSampleRate;
    
    TransportState mState;
    
    juce::RecentlyOpenedFilesList& mRecentFiles;
    juce::AudioFormatManager mFormatManager;
    
    std::unique_ptr<juce::FileInputSource> mFileSource;
    BreakbeatAudioSource mAudioSource;
    juce::AudioTransportSource mTransportSource;
    
    std::vector<float*> mTemporaryChannels;
    
    FileRecorder mRecorder {mFormatManager};
    bool mRecording = false;
    
    SliceExporter mSliceExporter {mFormatManager};
    
    WaveformComponent mWaveformComponent { *this, mFormatManager };
    
    juce::String mChosenPath;
    
    bool mRandomPosition;
    int mSampleBPM = 120;
    
    float mSampleChangeThreshold = 0.7f;
    float mReverseSampleThreshold = 0.7f;
    
    double mBlockDivisionPower = 1.0; // This should be stored as powers of 2 (whole = 1, half = 2, quarter = 4 etc)
    
    juce::File mRecordedFile {juce::File::getSpecialLocation(juce::File::SpecialLocationType::tempDirectory).getChildFile("toous").getChildFile("temp_recording.wav")};

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainContentComponent)
};
