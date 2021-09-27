#pragma once

#include <JuceHeader.h>
#include "../../core/ReferenceCountedForwardAndReverseBuffer.h"
#include "../../dsp/OfflineStretcher.h"

#define MAX_FILE_LENGTH 60.0 // seconds

class SampleManager
{
public:
    SampleManager(juce::AudioFormatManager& formatManager)
    : mFormatManager(formatManager)
    {
        
    }
    
    bool loadNewSample(juce::String const& filePath, juce::String& error)
    {
        juce::File file(filePath);
        std::unique_ptr<AudioFormatReader> reader { mFormatManager.createReaderFor(file) };
        
        error = juce::String();
        
        if(reader != nullptr)
        {
            // get length
            mSampleDuration = static_cast<double>(reader->lengthInSamples) / reader->sampleRate;
            if(mSampleDuration < MAX_FILE_LENGTH)
            {
                // set up the file buffer
                mSampleFileName = file.getFileName();
                mSampleSampleRate = reader->sampleRate;
                
                ReferenceCountedForwardAndReverseBuffer::Ptr newFileBuffer = new ReferenceCountedForwardAndReverseBuffer(mSampleFileName, reader.get());
                jassert(newFileBuffer != nullptr);
                        
                mActiveFileBuffer = newFileBuffer;
                mFileBuffers.add(mActiveFileBuffer);
                
                return true;
            }
            else
            {
                error = juce::translate("The file is ") + juce::String(mSampleDuration) + juce::translate("seconds long. ") +
                        juce::String(MAX_FILE_LENGTH) + "Second limit!";
                return false;
            }
        }
        
        error = "Failed to initialise file reader";
        return false;
    }
    
    double getFileLength() const
    {
        return mSampleDuration;
    }
    
    double getSampleSampleRate() const
    {
        return mSampleSampleRate;
    }
    
    juce::String getSampleFileName() const
    {
        return mSampleFileName;
    }
    
    double getBufferLength() const
    {
        return mBufferDuration;
    }
    
    size_t getBufferNumSamples() const
    {
        return mBufferNumSamples;
    }
    
    juce::AudioSampleBuffer* getActiveBuffer()
    {
        ReferenceCountedForwardAndReverseBuffer::Ptr retainedBuffer(mActiveBuffer);
        if(mActiveBuffer == nullptr)
        {
            return nullptr;
        }
        
        return mActiveBuffer->getForwardAudioSampleBuffer();
    }
    
    void clearFreeBuffers()
    {
        for(auto i = mBuffers.size(); --i >= 0;)
        {
            ReferenceCountedForwardAndReverseBuffer::Ptr buffer(mBuffers.getUnchecked(i));
            
            if(buffer->getReferenceCount() == 2)
            {
                mBuffers.remove(i);
            }
        }
        
        for(auto i = mFileBuffers.size(); --i >= 0;)
        {
            ReferenceCountedForwardAndReverseBuffer::Ptr buffer(mFileBuffers.getUnchecked(i));
            
            if(!buffer)
            {
                continue;
            }
            
            if(buffer->getReferenceCount() == 2)
            {
                mFileBuffers.remove(i);
            }
        }
    }
    
    bool performTimestretch(float stretchFactor, float pitchFactor = 1.0f, std::function<void()> callback = nullptr)
    {
        // act on the ORIGINAL file
        if(!mActiveFileBuffer)
        {
            std::cout << "Nothing in the buffer the stretch...\n";
            return false;
        }
        
        std::cout << "Performing stretch: factor " << stretchFactor << "x, pitch " << pitchFactor << "\n";
        OfflineStretchProcessor stretchTask(mTempStretchedFile,
                                            *mActiveFileBuffer->getForwardAudioSampleBuffer(),
                                            stretchFactor, pitchFactor,
                                            mSampleSampleRate);
        
        if(stretchTask.runThread())
        {
            std::cout << "Stretch complete. Temp file: " <<  mTempStretchedFile.getFile().getFullPathName() << "\n";
            std::unique_ptr<AudioFormatReader> reader { mFormatManager.createReaderFor(mTempStretchedFile.getFile()) };

            if(reader != nullptr)
            {
                // get length
                mBufferNumSamples = static_cast<size_t>(reader->lengthInSamples);
                mBufferDuration = static_cast<double>(reader->lengthInSamples) / reader->sampleRate;
                std::cout << "New duration: " << mBufferDuration << "\n";
                
                // TODO: clear existing?
                ReferenceCountedForwardAndReverseBuffer::Ptr newActiveBuffer = new ReferenceCountedForwardAndReverseBuffer(mSampleFileName + "stretched", reader.get());
                jassert(newActiveBuffer != nullptr);
                            
                mActiveBuffer = newActiveBuffer;
                mBuffers.add(mActiveBuffer);
                
                if(callback)
                {
                    callback();
                }
                
                return true;
            }
        }
        
        std::cerr << "Something went wrong with stretching...\n";
        return false;
    }
    
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
};

class BreakbeatAudioSource
: public PositionableAudioSource
{
public:
    BreakbeatAudioSource(SampleManager& sampleManager);
    ~BreakbeatAudioSource() override;
    
    int getNumSlices() const;
    int64_t getSliceSize() const;
    int64_t getStartReadPosition() const;
    
    void setSampleChangeThreshold(float threshold);
    void setReverseSampleThreshold(float threshold);
    void setBlockDivisionFactor(int factor);
    
    void toggleRandomPosition();
    void toggleRandomDirection();
    
    // juce::AudioAppComponent
    void prepareToPlay (int, double) override;
    void getNextAudioBlock (const AudioSourceChannelInfo& bufferToFill) override;
    void releaseResources() override;
    
    // juce::PositionableAudioSource
    void setNextReadPosition (int64 newPosition) override;
    int64 getNextReadPosition() const override;
    int64 getTotalLength() const override;
    bool isLooping() const override;
    
    void clear();
    
    void updateSliceSizes();
    
private:
    
    SampleManager& mSampleManager;
    
    std::atomic<int64_t> mNextReadPosition {0};
    std::atomic<int64_t> mSliceStartPosition {0};
    
    std::atomic<float> mSampleChangeThreshold {0.7f};
    std::atomic<float> mReverseSampleThreshold {0.7f};
    
    std::atomic<bool> mRandomPosition {false};
    std::atomic<bool> mRandomDirection {false};
    
    std::atomic<int> mNumSlices {1};
    std::atomic<int> mSliceSampleSize {1}; // in samples
    std::atomic<int> mBlockDivisionFactor {1};
};
