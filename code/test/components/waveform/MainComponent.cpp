#include "MainComponent.h"

using namespace OUS;

//==============================================================================
WaveformComponentTest::WaveformComponentTest(juce::AudioDeviceManager& deviceManager)
: juce::Thread("backgrounndthread")
{
    juce::ignoreUnused(deviceManager);
    
    mAudioFormats.registerBasicFormats();
    addAndMakeVisible(mWaveformAndRuler);
    
    mWaveformAndRuler.onNewFileDropped = [this](juce::String& filePath) { newFileDropped(filePath); };
    
    setSize(600, 250);
}

//==============================================================================
void WaveformComponentTest::paint(juce::Graphics& g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll(getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId));
}

void WaveformComponentTest::resized()
{
    auto waveformBounds = getLocalBounds().reduced(20, 21);
    mWaveformAndRuler.setBounds(waveformBounds);
}

void WaveformComponentTest::mouseUp(juce::MouseEvent const& event)
{
    if(event.mods.isCommandDown())
    {
        mWaveformAndRuler.resetZoom();
    }
}

//==============================================================================
void WaveformComponentTest::newFileDropped(juce::String& filePath)
{
    juce::File file(filePath);
    mReader = std::unique_ptr<juce::AudioFormatReader>(mAudioFormats.createReaderFor(file));
    if(mReader == nullptr)
    {
        return;
    }

    auto const numChannels = static_cast<int>(mReader.get()->numChannels);
    auto const numSamples = static_cast<int>(mReader.get()->lengthInSamples);
    ReferenceCountedBuffer::Ptr newBuffer = new ReferenceCountedBuffer(file.getFileName(), numChannels, numSamples);
    mReader.get()->read(newBuffer->getAudioSampleBuffer(), 0, numSamples, 0, true, true);
    {
        const juce::SpinLock::ScopedLockType lock(mMutex);
        mCurrentBuffer = newBuffer;
    }
    
    mWaveformAndRuler.setThumbnailSource(mCurrentBuffer->getAudioSampleBuffer());
}

void WaveformComponentTest::run()
{
    while(!threadShouldExit())
    {
        checkForBuffersToFree();
        wait(500);
    }
}

void WaveformComponentTest::checkForBuffersToFree()
{
    for(auto i = mBuffers.size(); --i >= 0;)
    {
        ReferenceCountedBuffer::Ptr buffer(mBuffers.getUnchecked(i));

        if(buffer->getReferenceCount() == 2)
        {
            mBuffers.remove(i);
        }
    }
}
