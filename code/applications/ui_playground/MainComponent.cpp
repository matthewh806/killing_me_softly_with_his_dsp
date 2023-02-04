#include "MainComponent.h"

//==============================================================================
MainComponent::MainComponent()
{
    setSize (600, 250);
}

MainComponent::~MainComponent()
{
}   

//==============================================================================
void MainComponent::paint (juce::Graphics& g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));
    
    g.setColour (juce::Colours::darkblue);
    g.setFont (20.0f);
    g.drawText ("Some people just do nothing", getLocalBounds(), juce::Justification::centred, true);
}

void MainComponent::resized()
{
}
