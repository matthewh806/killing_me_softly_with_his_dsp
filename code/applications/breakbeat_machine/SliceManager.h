#pragma once

#include "SampleManager.h"
#include "../../analysis/AudioAnalyser.h"

class SliceManager
: public SampleManager
, public juce::ChangeBroadcaster
{
public:
        
    // this tuple should be <start, end>
    // where end is defined either by the pos of the next marker or the end of the file
    using Slice = std::tuple<size_t, size_t>;
    
    enum Method
    {
        divisions,
        transients,
        manual
    };
    
    SliceManager(juce::AudioFormatManager& formatManager, Method sliceMethod = Method::divisions);
    ~SliceManager() = default;
    
    Method getSliceMethod();
    void setSliceMethod(Method method);
    
    void setThreshold(float threshold);
    void setDivisions(float divisions);
    
    void addSlice(size_t position);
    
    void deleteSlice(size_t position);
    
    // TODO: move slice?
    
    size_t getCurrentSliceIndex() const;
    Slice getCurrentSlice() const;
    
    // Sets the current slice to a random one and returns it
    Slice setRandomSlice();
    std::vector<size_t> const& getSlices() const;
    
    size_t getNumberOfSlices() const;
    
    void performSlice();
    
private:
    
    size_t mCurrentSliceIndex {0};
    Slice mCurrentSlice;
    
    Method mSliceMethod;
    
    float mThreshold;
    float mDivisions {1};
    
    // how to make this thread safe? std vector not trivally copyable -> so cant be atomic
    std::vector<size_t> mSlicePositions;
};
