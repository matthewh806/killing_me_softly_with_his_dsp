#pragma once

#include <JuceHeader.h>
#include "../../core/ReferenceCountedForwardAndReverseBuffer.h"
#include "../../dsp/OfflineStretcher.h"

#define MAX_FILE_LENGTH 60.0 // seconds

namespace OUS 
    {
    /*
    SampleManager
    
    Perhaps the name of this class is way too unclear
    The class is also responsible for applying time stretch operations
    to the buffers.
    
    That is the reason for all of this ReferenceCountedForwardAndReverseBuffer
    Its currently holding both the original sample in a buffer as well as
    forward / reverse versions of the stretched audio
    
    Its could all be managed a bit better still I think
    
    TODO: Slice manager?
    TODO: Separate timestretch handler?
    */
    class SampleManager
    {
    public:
        SampleManager(juce::AudioFormatManager& formatManager);
        ~SampleManager() = default;
        
        bool loadNewSample(juce::String const& filePath, juce::String& error);
        
        double getFileLength() const;
        double getSampleSampleRate() const;
        juce::String getSampleFileName() const;
        
        double getBufferLength() const;
        size_t getBufferNumSamples() const;
        
        void setForwardBufferActive();
        void setReverseBufferActive();
        
        juce::AudioSampleBuffer* getForwardBuffer();
        
        juce::AudioSampleBuffer* getActiveBuffer();
        void clearFreeBuffers();
        
        void performTimestretch(float stretchFactor, float pitchFactor = 1.0f, std::function<void()> callback = nullptr);
        void onTimestretchComplete();
        
    private:
        juce::AudioFormatManager& mFormatManager;
        
        juce::TemporaryFile mTempStretchedFile {".wav"};
        
        double mSampleDuration = 0.0;
        double mSampleSampleRate = 0.0;
        juce::String mSampleFileName = juce::String();
        
        double mBufferDuration = 0.0;
        size_t mBufferNumSamples = 0;
        
        juce::ReferenceCountedArray<ReferenceCountedForwardAndReverseBuffer> mBuffers;
        ReferenceCountedForwardAndReverseBuffer::Ptr mActiveBuffer;
        
        juce::ReferenceCountedArray<ReferenceCountedForwardAndReverseBuffer> mFileBuffers;
        ReferenceCountedForwardAndReverseBuffer::Ptr mActiveFileBuffer;
        
        std::unique_ptr<OfflineStretchProcessor> mStretchTask = nullptr;
        
        std::function<void()> mCallback = nullptr;
    };
}
