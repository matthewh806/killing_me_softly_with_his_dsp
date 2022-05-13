#pragma once

#include "../JuceLibraryCode/JuceHeader.h"
#include "../../ui/CustomLookAndFeel.h"

class DopplerShiftProcessor;

class DopplerScene
: public juce::Component
{
public:
    
    void updateSourcePosition(juce::Point<float> increment)
    {
        mSourcePosition += increment;
        repaint();
    }
    
    void setSourcePosition(juce::Point<float> position)
    {
        mSourcePosition = position;
        repaint();
    }
    
    void setObserverPosition(juce::Point<float> position)
    {
        mObserverPosition = position;
    }
    
    void paint(Graphics& g) override
    {
        auto const halfWidth = getWidth() / 2.0f;
        auto const halfHeight = getHeight() / 2.0f;
        
        auto drawCircle = [&g]( float centerX, float centerY, float radius, juce::Colour colour)
        {
            auto const topLeftX = centerX - radius;
            auto const topLeftY = centerY - radius;
            
            g.setColour(colour);
            g.drawEllipse(topLeftX, topLeftY, radius * 2.0f, radius * 2.0f, 2.0f);
        };
        
        drawCircle(mSourcePosition.getX() + halfWidth, mSourcePosition.getY() + halfHeight, mCircleRadius, juce::Colours::yellow);
        drawCircle(mObserverPosition.getX() + halfWidth, mObserverPosition.getY() + halfHeight, mCircleRadius, juce::Colours::red);
    }
    
private:
    float static constexpr mCircleRadius = 5.0f;
    
    juce::Point<float> mSourcePosition {0.0f, 0.0f};
    juce::Point<float> mObserverPosition {0.0f, 0.0f}; // fixed for now
};

class DopplerShiftPluginEditor
: public juce::AudioProcessorEditor
/*, private juce::Timer*/
{
public:
    DopplerShiftPluginEditor (AudioProcessor& audioProcessor, juce::AudioProcessorValueTreeState& state)
    : AudioProcessorEditor(audioProcessor)
    , mSourceSpeedAttachment(state, "sourceSpeed", mSourceSpeedSlider)
    , mObserverYPositionAttachment(state, "observerY", mObserverYPositionSlider)
    {
        addAndMakeVisible(mSourceSpeedSlider);
        mSourceSpeedSliderLabel.attachToComponent(&mSourceSpeedSlider, true);
        
        addAndMakeVisible(mObserverYPositionSlider);
        mObserverYPositionSliderLabel.attachToComponent(&mObserverYPositionSlider, true);
        
        addAndMakeVisible(mDopplerScene);
        setSize(400, 400);
    }
    
    ~DopplerShiftPluginEditor() override
    {
        
    }

    //==============================================================================
    void paint(Graphics&) override
    {
        
    }
    
    void resized() override
    {
        auto bounds = getLocalBounds();
        
        mObserverYPositionSlider.setBounds(bounds.removeFromBottom(20).removeFromRight(static_cast<int>(bounds.getWidth() * 0.66f)));
        mSourceSpeedSlider.setBounds(bounds.removeFromBottom(20).removeFromRight(static_cast<int>(bounds.getWidth() * 0.66f)));
        mDopplerScene.setBounds(bounds);
    }
    
    void setSourcePosition(juce::Point<float> position)
    {
        mDopplerScene.setSourcePosition(position);
    }
    
    void setObserverPosition(juce::Point<float> position)
    {
        mDopplerScene.setObserverPosition(position);
    }
    
    void updatePositions(juce::Point<float> increment)
    {
        // update the positions based on the speed of the source
        mDopplerScene.updateSourcePosition(increment);
    }
    
    float getDrawingViewWidth()
    {
        return static_cast<float>(mDopplerScene.getWidth());
    }

private:
    //=============================================================================
    
    juce::Slider mSourceSpeedSlider;
    juce::AudioProcessorValueTreeState::SliderAttachment mSourceSpeedAttachment;
    
    juce::Slider mObserverYPositionSlider;
    juce::AudioProcessorValueTreeState::SliderAttachment mObserverYPositionAttachment;
    
    juce::Label mSourceSpeedSliderLabel {{}, "Source Speed"};
    juce::Label mObserverYPositionSliderLabel {{}, "Observer Y"};
    
    DopplerScene mDopplerScene;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (DopplerShiftPluginEditor)
    
private:
};
