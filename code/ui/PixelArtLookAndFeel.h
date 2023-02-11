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
            
            juce::Font getComboBoxFont(juce::ComboBox& box) override;
            juce::Font getPopupMenuFont() override;
            
            void drawRotarySlider(juce::Graphics&,
                                  int x, int y, int width, int height,
                                  float sliderPosProportional,
                                  float rotaryStartAngle,
                                  float rotaryEndAngle,
                                  juce::Slider& slider) override;
            
            void drawComboBox(Graphics &, int width, int height, bool isButtonDown, int buttonX, int buttonY, int buttonW, int buttonH, ComboBox &) override;
            
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
    
        class SelectorComponent : public juce::Component
        {
            /*
             TODO:
                - Paint black border
                - Set up arrow buttons properly
                - Override combo box paint method
             
             */
        public:
            SelectorComponent();
            ~SelectorComponent() override = default;
            
            void paint(juce::Graphics& g) override;
            void paintOverChildren (Graphics& g) override;
            void resized() override;
            
            // Label methods
            void setText(const juce::String& newText, juce::NotificationType notification);
            
            // ComboBox methods
            void addItem (const String& newItemText, int newItemId);
            
        private:
            class ArrowButton : public juce::ImageButton
            {
            public:
                ArrowButton(bool flipped = false);
                ~ArrowButton() override = default;
            };
            
            juce::Label mTitle;
            juce::ComboBox mComboBox;
            ArrowButton mUpButton;
            ArrowButton mDownButton{true};
        };
    
        class RotarySliderWithLabels
        : public juce::Slider
        {
        public:
            RotarySliderWithLabels(juce::String const& paramName, juce::String const& unitSuffix, double defaultValue = 0.0);
            ~RotarySliderWithLabels() override;
            
            struct LabelPos
            {
                float pos;
                juce::String label;
            };

            void paint(juce::Graphics& g) override;
            juce::Rectangle<int> getSliderBounds() const;
            int getTextHeight() const;
            virtual juce::String getDisplayString() const;
            juce::String getParameterName() const;

            juce::Array<LabelPos> mLabels;
            
        private:
            CustomLookAndFeel mLookAndFeel;
            
            juce::String mParamName;
            juce::String mSuffix;
        };
        
    }
} // namespace OUS
