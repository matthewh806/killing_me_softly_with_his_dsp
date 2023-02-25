#include "WaveformAndRuler.h"

using namespace OUS;

WaveformAndRuler::WaveformAndRuler(juce::AudioFormatManager& formatManager)
: mWaveform(formatManager)
{
    addAndMakeVisible(mWaveform);
    mWaveform.addChangeListener(this);
    mWaveform.onNewFileDropped = [this](juce::String& filePath)
    {
        if(onNewFileDropped != nullptr)
        {
            onNewFileDropped(filePath);
        }
    };
    
    addAndMakeVisible(mSampleRuler);
    mSampleRuler.setSampleRate(44100.0);
}

WaveformAndRuler::~WaveformAndRuler()
{
    mWaveform.removeChangeListener(this);
}

WaveformComponent const& WaveformAndRuler::getWaveform() const
{
    return mWaveform;
}

void WaveformAndRuler::addWaveformChangeListener(juce::ChangeListener* listener)
{
    mWaveform.addChangeListener(listener);
}

void WaveformAndRuler::removeWaveformChangeListener(juce::ChangeListener* listener)
{
    mWaveform.removeChangeListener(listener);
}

juce::Range<float> const& WaveformAndRuler::getVisibleRange() const
{
    return mWaveform.getVisibleRange();
}

juce::Range<float> const& WaveformAndRuler::getTotalRange() const
{
    return mWaveform.getTotalRange();
}

void WaveformAndRuler::setSampleRate(float sampleRate)
{
    mSampleRuler.setSampleRate(sampleRate);
}

void WaveformAndRuler::resetZoom()
{
    mWaveform.resetZoom();
}

// juce::FileDragAndDropTarget
bool WaveformAndRuler::isInterestedInFileDrag(const StringArray& files)
{
    return mWaveform.isInterestedInFileDrag(files);
}

void WaveformAndRuler::filesDropped(const StringArray& files, int x, int y)
{
    mWaveform.filesDropped(files, x, y);
}

void WaveformAndRuler::setThumbnailSource(juce::AudioSampleBuffer* audioSource)
{
    mWaveform.setThumbnailSource(audioSource);
    mSampleRuler.setTotalRange(mWaveform.getTotalRange());
    
    repaint();
}

void WaveformAndRuler::clear()
{
    mWaveform.clear();
}

void WaveformAndRuler::resized()
{
    auto bounds = getLocalBounds();
    mSampleRuler.setBounds(bounds.removeFromBottom(20));
    mWaveform.setBounds(bounds);
}

void WaveformAndRuler::mouseWheelMove(juce::MouseEvent const& event, juce::MouseWheelDetails const& wheel)
{
    mWaveform.mouseWheelMove(event, wheel);
}

void WaveformAndRuler::changeListenerCallback(juce::ChangeBroadcaster* source)
{
    if(source == &mWaveform)
    {
        mSampleRuler.setVisibleRange(mWaveform.getVisibleRange());
    }
}
