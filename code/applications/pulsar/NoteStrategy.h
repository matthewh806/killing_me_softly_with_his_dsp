#pragma once

#include <JuceHeader.h>

class NoteStrategy
{
public:
    NoteStrategy();
    
    enum class Strategy
    {
        random,
        major,
        minor
    };
    
    void setStrategy(Strategy strategy);
    void setKey(std::string key);
    
    std::vector<std::string> const notes =
    {
        "C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B"
    };
    
    std::map<Strategy, const std::vector<int>> const  degrees =
    {
        {Strategy::random, {0, 1, 2, 3, 4, 5 , 6, 7, 8, 9, 10, 11}},
        {Strategy::major, {0, 2, 4, 5, 7, 9, 11}},
        {Strategy::minor, {0, 2, 3, 5, 7, 8 , 10}}
    };
    
    int getMidiNote();
    
private:
    Strategy mStrategy { Strategy::major };
    std::string mKey = "C";
    
    Random mRandom;
};
