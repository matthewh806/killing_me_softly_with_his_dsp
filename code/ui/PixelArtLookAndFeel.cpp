#include "PixelArtLookAndFeel.h"

using namespace OUS;
using namespace OUS::UI::PixelArt;

juce::Font FontManager::getDefaultLabelFont()
{
    return juce::Font { "Retro Gaming", defaultFontSize, juce::Font::FontStyleFlags::plain };
}

juce::Font FontManager::getDefaultTitleFont()
{
    return juce::Font { "Retro Gaming", defaultTitleSize, juce::Font::FontStyleFlags::plain };
}

OUS::UI::PixelArt::CustomLookAndFeel::CustomLookAndFeel()
{
    
}

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

PlayButton::PlayButton()
{
    setClickingTogglesState(true);
}

PauseButton::PauseButton()
{
    setClickingTogglesState(true);
}

RecordButton::RecordButton()
{
    setClickingTogglesState(true);
}

TitleLabel::TitleLabel()
{
    setFont(FontManager::getDefaultTitleFont());
    setEditable(false, false, false);
    setJustificationType(juce::Justification::centred);
}
