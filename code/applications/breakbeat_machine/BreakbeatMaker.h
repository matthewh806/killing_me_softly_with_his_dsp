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
, private juce::KeyListener
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
    
    // juce::KeyListener
    bool keyPressed(juce::KeyPress const& key) override;
    bool keyPressed (const juce::KeyPress& key, juce::Component* originatingComponent) override;
    
    void newFileOpened(String& filePath);
    void setFileOutputPath();
    void exportAudioSlices();
    
    void fromXml(juce::XmlElement const& xml);
    std::unique_ptr<juce::XmlElement> toXml();

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
        
        void setSlicePositions(std::vector<SliceManager::Slice> const& slicePositions, size_t activeSliceIndex);
        void setActiveSlice(size_t sliceIndex);
        
        void clear();
        
        // juce::Component
        void resized() override;
        void paint(juce::Graphics& g) override;
        void mouseDoubleClick(juce::MouseEvent const& event) override;
        void mouseDown(juce::MouseEvent const& event) override;
        void mouseUp(juce::MouseEvent const& event) override;
        void mouseDrag (const MouseEvent& event) override;
        
        // juce::FileDragAndDropTarget
        bool isInterestedInFileDrag (const StringArray& files) override;
        void filesDropped (const StringArray& files, int x, int y) override;
        
        // juce::AsyncUpdater
        void handleAsyncUpdate() override;
        
        std::function<void(int)> onWaveformDoubleClicked = nullptr;
        std::function<void(int)> onSliceMarkerRightClicked = nullptr;
        std::function<void(int)> onSliceMarkerMouseDown = nullptr;
        std::function<void(float)> onSliceMarkerDragged = nullptr;
        std::function<void()> onMouseUp = nullptr;
        
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
    
    NumberFieldWithLabel mSampleLengthSeconds {"File Length", "s", 3, false};
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
    
    std::unique_ptr<juce::FileChooser> mFileChooser = nullptr;
    
    juce::Uuid mActiveMouseMarker;
    float mPrevDragPos = 0.0f;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (BreakbeatContentComponent)
};
