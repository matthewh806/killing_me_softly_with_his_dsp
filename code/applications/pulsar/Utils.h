/*
  ==============================================================================

    Utils.h
    Created: 25 Mar 2020 12:18:58am
    Author:  Matthew

  ==============================================================================
*/

#pragma once

#include "../JuceLibraryCode/JuceHeader.h"

class Constants
{
    #define RAND_LIMIT 32767

    #define GRAV_MIN 0.0f
    #define GRAV_MAX 100.0f
  
    #define OCTAVE_MIN 0
    #define OCTAVE_MAX 8
    
    #define NOTE_LEN_MIN 100  // ms
    #define NOTE_LEN_MAX 1000
    
    
    #define PHYSICS_STEP_FREQ 60 // Physics / renderer refresh in Hz
};

namespace Physics
{
    class Utils
    {
    public:
        static float pixelsToMeters(float vPixel)
        {
            return vPixel / pixelsPerMeter;
        }

        static float metersToPixels(float vMeter)
        {
            return vMeter * pixelsPerMeter;
        }

        static b2Vec2 pixelsToMeters(float xPixels, float yPixels)
        {
            return b2Vec2( xPixels / pixelsPerMeter, yPixels / pixelsPerMeter);
        }

        static b2Vec2 metersToPixels(float xMeters, float yMeters)
        {
            return b2Vec2(xMeters * pixelsPerMeter, yMeters * pixelsPerMeter);
        }
        
        static float RandomFloat(float lo, float hi)
        {
            float r = (float)(rand() & (RAND_LIMIT));
            r /= RAND_LIMIT;
            r = (hi - lo) * r + lo;
            return r;
        }

        
        constexpr static const float pixelsPerMeter = 125;
    };
}
