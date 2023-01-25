#include "SampleManager.h"

SampleManager::SampleManager(juce::AudioFormatManager& formatManager)
: mFormatManager(formatManager)
{
    
}

bool SampleManager::loadNewSample(juce::String const& filePath, juce::String& error)
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

double SampleManager::getFileLength() const
{
    return mSampleDuration;
}

double SampleManager::getSampleSampleRate() const
{
    return mSampleSampleRate;
}

juce::String SampleManager::getSampleFileName() const
{
    return mSampleFileName;
}

double SampleManager::getBufferLength() const
{
    return mBufferDuration;
}

size_t SampleManager::getBufferNumSamples() const
{
    return mBufferNumSamples;
}

void SampleManager::setForwardBufferActive()
{
    ReferenceCountedForwardAndReverseBuffer::Ptr retainedBuffer(mActiveBuffer);
    if(mActiveBuffer == nullptr)
    {
        return;
    }
    
    retainedBuffer->updateCurrentSampleBuffer(false);
}

juce::AudioSampleBuffer* SampleManager::getForwardBuffer()
{
    ReferenceCountedForwardAndReverseBuffer::Ptr retainedBuffer(mActiveBuffer);
    if(mActiveBuffer == nullptr)
    {
        return nullptr;
    }
    
    return retainedBuffer->getForwardAudioSampleBuffer();
}

void SampleManager::setReverseBufferActive()
{
    ReferenceCountedForwardAndReverseBuffer::Ptr retainedBuffer(mActiveBuffer);
    if(mActiveBuffer == nullptr)
    {
        return;
    }
    
    retainedBuffer->updateCurrentSampleBuffer(true);
}

juce::AudioSampleBuffer* SampleManager::getActiveBuffer()
{
    ReferenceCountedForwardAndReverseBuffer::Ptr retainedBuffer(mActiveBuffer);
    if(mActiveBuffer == nullptr)
    {
        return nullptr;
    }
    
    return mActiveBuffer->getCurrentAudioSampleBuffer();
}

void SampleManager::clearFreeBuffers()
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

void SampleManager::performTimestretch(float stretchFactor, float pitchFactor, std::function<void()> callback)
{
    mCallback = callback;
    
    // act on the ORIGINAL file
    if(!mActiveFileBuffer)
    {
        std::cout << "Nothing in the buffer the stretch...\n";
        return;
    }
    
    std::cout << "Performing stretch: factor " << stretchFactor << "x, pitch " << pitchFactor << "\n";
    mStretchTask=std::make_unique<OfflineStretchProcessor>(mTempStretchedFile,
                                                              *mActiveFileBuffer->getForwardAudioSampleBuffer(),
                                                              stretchFactor, pitchFactor,
                                                              mSampleSampleRate,
                                                              [this]() { onTimestretchComplete(); });
    mStretchTask->launchThread();
}

void SampleManager::onTimestretchComplete()
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
        
        if(mCallback != nullptr)
        {
            mCallback();
        }
    }
}
