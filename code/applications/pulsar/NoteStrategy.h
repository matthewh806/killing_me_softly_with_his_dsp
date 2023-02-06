#pragma once

#include <JuceHeader.h>

namespace OUS {

    class NoteStrategy
    {
    public:
        NoteStrategy();
        
        enum class Strategy
        {
            random,
            major,
            minor,
            harmonicMinor,
            melodicMinor,
            minorPentatonic,
            majorPentatonic,
            dorian,
            phrygian,
            lydian
        };
        
        void setStrategy(Strategy strategy);
        void setKey(std::string key);
        
        juce::Range<int> getOctaveRange() const;
        void setOctaveRange(juce::Range<int> octaveRange);
        
        std::vector<std::string> const notes =
        {
            "C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B"
        };
        
        std::map<Strategy, const std::vector<int>> const  degrees =
        {
            {Strategy::random, {0, 1, 2, 3, 4, 5 , 6, 7, 8, 9, 10, 11}},
            {Strategy::major, {0, 2, 4, 5, 7, 9, 11}},
            {Strategy::minor, {0, 2, 3, 5, 7, 8 , 10}},
            {Strategy::harmonicMinor, {0, 2, 3, 5, 7, 8, 11}},
            {Strategy::melodicMinor, {0, 2, 3, 5, 7, 9, 11}},
            {Strategy::minorPentatonic, {0,3,5,7,10}},
            {Strategy::majorPentatonic, {0,2,4,7,9}},
            {Strategy::dorian, {0,2,3,5,7,9,10}},
            {Strategy::phrygian, {0,1,3,5,7,8,10}},
            {Strategy::lydian, {0,2,4,6,7,9,11}}
        };
        
        int getMidiNote();
        
    private:
        Strategy mStrategy { Strategy::major };
        std::string mKey = "C";
        
        Random mRandom;
        
        // The range is exclusive of the max so this is really 2-7
        juce::Range<int> mOctaveRange {2, 8};
    };
}
