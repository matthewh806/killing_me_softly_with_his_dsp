#pragma once

#include <JuceHeader.h>
#include "PixelArtBinaryData.h"

namespace OUS
{
    namespace UI::PixelArt
    {
        class SyncButton : public juce::ImageButton
        {
        public:
            SyncButton();
            ~SyncButton() override = default;
        };
    }
} // namespace OUS
