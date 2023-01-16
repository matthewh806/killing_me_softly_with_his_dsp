#include "NoteStrategy.h"

NoteStrategy::NoteStrategy()
{
    
}

void NoteStrategy::setStrategy(Strategy strategy)
{
    if(mStrategy != strategy)
    {
        mStrategy = strategy;
    }
}

void NoteStrategy::setKey(std::string key)
{
    if(std::find(notes.begin(), notes.end(), key) == notes.end())
    {
        std::cerr << "Error: Key " << key << " is not valid\n";
        return;
    }
    
    mKey = key;
}

int NoteStrategy::getMidiNote()
{
    auto const rootNoteIndex = std::distance(notes.begin(), std::find(notes.begin(), notes.end(), mKey));
    if(rootNoteIndex > static_cast<int>(notes.size()))
    {
        std::cerr << "Error: Note index out of range\n";
        return 32;
    }
    
    auto const& scaleDegrees = degrees.at(mStrategy);
    // get a random value in the strategy
    auto const degree = scaleDegrees[static_cast<size_t>(mRandom.nextInt(static_cast<int>(scaleDegrees.size())))];
    auto const octave = mRandom.nextInt(juce::Range<int>(2, 7));
    auto const noteIdx = (rootNoteIndex + degree) % static_cast<int>(notes.size());
    auto const midiNote = octave * 12 + noteIdx;
    std::cout << "Note: " << notes[static_cast<size_t>(noteIdx)] << octave << ", " << midiNote << "\n";
    
    return static_cast<int>(midiNote);
    
}
