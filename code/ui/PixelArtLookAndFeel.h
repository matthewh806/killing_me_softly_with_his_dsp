#pragma once

#include <JuceHeader.h>
#include "PixelArtBinaryData.h"

namespace OUS
{
    namespace UI::PixelArt
    {
        class FontManager
        {
        public:
            static constexpr float defaultFontSize = 15.0f;
            static constexpr float defaultTitleSize = 40.0f;
            
            static juce::Font getDefaultLabelFont();
            static juce::Font getDefaultTitleFont();
        };
    
        class CustomLookAndFeel : public juce::LookAndFeel_V4
        {
        public:
            CustomLookAndFeel();
            
        private:
            
        };
        
        class SyncButton : public juce::ImageButton
        {
        public:
            SyncButton();
            ~SyncButton() override = default;
        };
    
        class PlayButton : public juce::ImageButton
        {
        public:
            PlayButton();
            ~PlayButton() override = default;
        };
        
        class PauseButton : public juce::ImageButton
        {
        public:
            PauseButton();
            ~PauseButton() override = default;
        };
    
        class RecordButton : public juce::ImageButton
        {
        public:
            RecordButton();
            ~RecordButton() override = default;
        };
    
        class TitleLabel : public juce::Label
        {
        public:
            TitleLabel();
            ~TitleLabel() override = default;
        };
        
    }
} // namespace OUS
