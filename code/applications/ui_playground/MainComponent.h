#pragma once

#include "JuceHeader.h"
#include "../../ui/CustomLookAndFeel.h"

//==============================================================================
class MainComponent   : public juce::Component
{
public:
    //==============================================================================
    MainComponent();
    ~MainComponent();

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;
    
private:
    //==============================================================================

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainComponent)
};
