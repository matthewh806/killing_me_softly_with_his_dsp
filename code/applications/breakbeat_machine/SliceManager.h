#pragma once

#include "SampleManager.h"
#include "../../analysis/AudioAnalyser.h"

class SliceManager
: public SampleManager
, public juce::ChangeBroadcaster
{
public:
        
    // this tuple should be <uuid, start, end>
    // where end is defined either by the pos of the next marker or the end of the file
    using Slice = std::tuple<juce::Uuid, size_t, size_t>;
    
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
    void deleteSlice(juce::Uuid sliceId);
    
    size_t getCurrentSliceIndex() const;
    Slice getCurrentSlice() const;
    
    Slice* getSliceAtSamplePosition(size_t pos, int tolerance);
    
    // Sets the current slice to a random one and returns it
    Slice setRandomSlice();
    std::vector<Slice> const& getSlices() const;
    
    size_t getNumberOfSlices() const;
    
    void performSlice();
    
private:
    
    size_t mCurrentSliceIndex {0};
    Slice mCurrentSlice;
    
    Method mSliceMethod;
    
    float mThreshold;
    float mDivisions {1};
    
    // how to make this thread safe? std vector not trivally copyable -> so cant be atomic
    std::vector<Slice> mSlices;
};
