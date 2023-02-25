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

void WaveformAndRuler::resetZoom()
{
    mWaveform.resetZoom();
}

void WaveformAndRuler::setThumbnailSource(juce::AudioSampleBuffer* audioSource)
{
    mWaveform.setThumbnailSource(audioSource);
    mSampleRuler.setTotalRange(mWaveform.getTotalRange());
    
    repaint();
}

void WaveformAndRuler::resized()
{
    auto bounds = getLocalBounds();
    mSampleRuler.setBounds(bounds.removeFromBottom(20));
    mWaveform.setBounds(bounds);
}

void WaveformAndRuler::changeListenerCallback(juce::ChangeBroadcaster* source)
{
    if(source == &mWaveform)
    {
        mSampleRuler.setVisibleRange(mWaveform.getVisibleRange());
    }
}
