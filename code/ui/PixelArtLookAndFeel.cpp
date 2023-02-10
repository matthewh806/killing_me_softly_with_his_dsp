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
    setColour(juce::Label::textColourId, juce::Colours::black);
    setColour(juce::ComboBox::backgroundColourId, juce::Colours::white);
    setColour(juce::ComboBox::textColourId, juce::Colours::black);
    
    setColour(juce::PopupMenu::backgroundColourId, juce::Colours::white);
    setColour(juce::PopupMenu::textColourId, juce::Colours::black);
}

juce::Font OUS::UI::PixelArt::CustomLookAndFeel::getComboBoxFont(juce::ComboBox& box)
{
    return juce::Font("Retro Gaming", jmin (15.0f, (float) box.getHeight() * 0.85f), juce::Font::FontStyleFlags::plain);
}

juce::Font OUS::UI::PixelArt::CustomLookAndFeel::getPopupMenuFont()
{
    return juce::Font("Retro Gaming", 17.0f, juce::Font::FontStyleFlags::plain);
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
    
    setImages(true,
              true,
              true,
              juce::ImageCache::getFromMemory(PixelArtBinaryData::play_button1_png, PixelArtBinaryData::play_button1_pngSize),
              1.0f,
              juce::Colours::transparentBlack,
              juce::ImageCache::getFromMemory(PixelArtBinaryData::play_button1_png, PixelArtBinaryData::play_button1_pngSize),
              1.0f,
              juce::Colours::transparentBlack,
              juce::ImageCache::getFromMemory(PixelArtBinaryData::play_button2_png, PixelArtBinaryData::play_button2_pngSize),
              1.0f,
              juce::Colours::transparentBlack);
}

PauseButton::PauseButton()
{
    setImages(true,
              true,
              true,
              juce::ImageCache::getFromMemory(PixelArtBinaryData::pause_button1_png, PixelArtBinaryData::pause_button1_pngSize),
              1.0f,
              juce::Colours::transparentBlack,
              juce::ImageCache::getFromMemory(PixelArtBinaryData::pause_button1_png, PixelArtBinaryData::pause_button1_pngSize),
              1.0f,
              juce::Colours::transparentBlack,
              juce::ImageCache::getFromMemory(PixelArtBinaryData::pause_button2_png, PixelArtBinaryData::pause_button2_pngSize),
              1.0f,
              juce::Colours::transparentBlack);
}

RecordButton::RecordButton()
{
    setClickingTogglesState(true);
    
    setImages(true,
              true,
              true,
              juce::ImageCache::getFromMemory(PixelArtBinaryData::record_button1_png, PixelArtBinaryData::record_button1_pngSize),
              1.0f,
              juce::Colours::transparentBlack,
              juce::ImageCache::getFromMemory(PixelArtBinaryData::record_button1_png, PixelArtBinaryData::record_button1_pngSize),
              1.0f,
              juce::Colours::transparentBlack,
              juce::ImageCache::getFromMemory(PixelArtBinaryData::record_button2_png, PixelArtBinaryData::record_button2_pngSize),
              1.0f,
              juce::Colours::transparentBlack);
}

TitleLabel::TitleLabel()
{
    setFont(FontManager::getDefaultTitleFont());
    setEditable(false, false, false);
    setJustificationType(juce::Justification::centred);
}

SelectorComponent::ArrowButton::ArrowButton(bool flipped)
{
    
    auto& normalImage = flipped ? PixelArtBinaryData::down_arrow_button1_png : PixelArtBinaryData::up_arrow_button1_png;
    auto& downImage = flipped ? PixelArtBinaryData::down_arrow_button2_png : PixelArtBinaryData::up_arrow_button2_png;
    auto normalSize = flipped ? PixelArtBinaryData::down_arrow_button1_pngSize : PixelArtBinaryData::up_arrow_button1_pngSize;
    auto downSize = flipped ? PixelArtBinaryData::down_arrow_button2_pngSize : PixelArtBinaryData::up_arrow_button2_pngSize;
    
    setImages(true,
              true,
              true,
              juce::ImageCache::getFromMemory(normalImage, normalSize),
              1.0f,
              juce::Colours::transparentBlack,
              juce::ImageCache::getFromMemory(normalImage, normalSize),
              1.0f,
              juce::Colours::transparentBlack,
              juce::ImageCache::getFromMemory(downImage, downSize),
              1.0f,
              juce::Colours::transparentBlack);
    
}

SelectorComponent::SelectorComponent()
{
    addAndMakeVisible(mTitle);
    mTitle.setFont(FontManager::getDefaultLabelFont());
    mTitle.setEditable(false);
    
    addAndMakeVisible(mComboBox);
    
    addAndMakeVisible(mUpButton);
    mUpButton.onClick = [this]()
    {
        // prev item in the list
        auto const selectedIndex = mComboBox.getSelectedItemIndex();
        auto const numberItems = mComboBox.getNumItems();
        
        if(selectedIndex <= 0)
        {
            mComboBox.setSelectedItemIndex(numberItems - 1, juce::NotificationType::sendNotification);
        }
        else
        {
            mComboBox.setSelectedItemIndex(selectedIndex - 1, juce::NotificationType::sendNotification);
        }
    };
    
    addAndMakeVisible(mDownButton);
    mDownButton.onClick = [this]()
    {
        // next item in the list
        auto const selectedIndex = mComboBox.getSelectedItemIndex();
        auto const numberItems = mComboBox.getNumItems();
        
        if(selectedIndex < 0)
        {
            mComboBox.setSelectedItemIndex(0, juce::NotificationType::sendNotification);
        }
        else
        {
            mComboBox.setSelectedItemIndex((selectedIndex + 1) % numberItems, juce::NotificationType::sendNotification);
        }
    };
}

void SelectorComponent::paint(juce::Graphics &g)
{
    // draw background
    g.setColour(juce::Colours::white);
    auto const bounds = getLocalBounds();
    g.fillRect(bounds);
}

void SelectorComponent::resized()
{
    auto bounds = getLocalBounds();
    auto const width = bounds.getWidth();
    mTitle.setBounds(bounds.removeFromLeft(static_cast<int>(width * 0.4f)));
    bounds.removeFromLeft(static_cast<int>(width * 0.1));
    
    auto comboBoxBounds = bounds.removeFromLeft(static_cast<int>(width * 0.4f));
    mComboBox.setBounds(comboBoxBounds);
    
    auto buttonBounds = bounds;
    mUpButton.setBounds(buttonBounds.removeFromTop(static_cast<int>(getHeight() * 0.5f)));
    mDownButton.setBounds(buttonBounds);
}

void SelectorComponent::setText(const juce::String& newText, juce::NotificationType notification)
{
    mTitle.setText(newText, notification);
}

void SelectorComponent::addItem(const String& newItemText, int newItemId)
{
    mComboBox.addItem(newItemText, newItemId);
}
