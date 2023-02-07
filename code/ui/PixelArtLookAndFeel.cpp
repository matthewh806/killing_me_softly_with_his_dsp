#include "PixelArtLookAndFeel.h"

using namespace OUS;
using namespace OUS::UI::PixelArt;

SyncButton::SyncButton()
{
    setImages(true,
              true,
              true,
              juce::ImageCache::getFromMemory(PixelArtBinaryData::sync_button1_png, PixelArtBinaryData::sync_button1_pngSize),
              1.0f,
              juce::Colours::transparentBlack,
              juce::ImageCache::getFromMemory(PixelArtBinaryData::sync_button1_png, PixelArtBinaryData::sync_button1_pngSize),
              1.0f,
              juce::Colours::transparentBlack,
              juce::ImageCache::getFromMemory(PixelArtBinaryData::sync_button2_png, PixelArtBinaryData::sync_button2_pngSize),
              1.0f,
              juce::Colours::transparentBlack);
    
    setClickingTogglesState(true);
}
