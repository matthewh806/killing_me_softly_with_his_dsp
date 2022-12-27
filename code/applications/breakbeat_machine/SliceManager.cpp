#include "SliceManager.h"
#include <algorithm>

SliceManager::SliceManager(juce::AudioFormatManager& formatManager, Method sliceMethod)
: SampleManager(formatManager)
, mSliceMethod(sliceMethod)
{
    performSlice();
}

SliceManager::Method SliceManager::getSliceMethod()
{
    return mSliceMethod;
}

void SliceManager::setSliceMethod(Method method)
{
    if(method != mSliceMethod)
    {
        mSliceMethod = method;
        performSlice();
    }
}

void SliceManager::setThreshold(float threshold)
{
    mThreshold = threshold;
    
    if(mSliceMethod == Method::transients)
    {
        performSlice();
    }
}

void SliceManager::setDivisions(float divisions)
{
    mDivisions = divisions;
    
    if(mSliceMethod == Method::divisions)
    {
        performSlice();
    }
}

void SliceManager::addSlice(size_t position)
{
    auto const itr = std::find_if(mSlices.begin(), mSlices.end(), [&](const Slice& slice)
    {
        auto sliceStartPos = std::get<1>(slice);
        return sliceStartPos == position;
    });
    
    if(itr == mSlices.end())
    {
        // find end point
        auto const nextSliceItr = std::find_if(mSlices.begin(), mSlices.end(), [&](const Slice& slice)
        {
            auto sliceStartPos = std::get<1>(slice);
            return sliceStartPos > std::get<1>(*itr);
        });
        
        auto const endPoint = (nextSliceItr == mSlices.end()) ? getBufferNumSamples() : std::get<1>(*nextSliceItr);
        
        mSlices.push_back({juce::Uuid(), position, endPoint});
        if(mSliceMethod == Method::manual)
        {
            performSlice();
        }
    }
}

void SliceManager::deleteSlice(juce::Uuid sliceId)
{
    auto const itr = std::find_if(mSlices.begin(), mSlices.end(), [&](const Slice& slice)
    {
        return std::get<0>(slice) == sliceId;
    });
    
    if(itr == mSlices.end())
    {
        // slice at posn not found
        std::cout << "No slice to delete \n";
        return;
    }
    
    mSlices.erase(itr);
    sendChangeMessage();
}

size_t SliceManager::getCurrentSliceIndex() const
{
    return mCurrentSliceIndex;
}

SliceManager::Slice SliceManager::getCurrentSlice() const
{
    return mCurrentSlice;
}

SliceManager::Slice* SliceManager::getSliceAtSamplePosition(size_t pos, int tolerance)
{
    auto const itr = std::find_if(mSlices.begin(), mSlices.end(), [&](const Slice& s)
    {
        auto const lowerBound = static_cast<int>(pos) - tolerance >= 0 ? pos - static_cast<size_t>(tolerance) : 0;
        auto const upperBound = pos + static_cast<size_t>(tolerance);
        auto const markerPos = std::get<1>(s);
        
        return markerPos > lowerBound && markerPos < upperBound;
    });
    decltype(&*itr) ptr;
    
    if(itr == mSlices.end())
    {
        ptr = nullptr;
    }
    else
    {
        ptr = &*itr;
    }
    
    return ptr;
}

// Sets the current slice to a random one and returns it
SliceManager::Slice SliceManager::setRandomSlice()
{
    // this is a hack to prevent a threading issue
    if(mSlices.size() == 0)
    {
        return mCurrentSlice;
    }
 
    auto const sliceIndex = static_cast<size_t>(Random::getSystemRandom().nextInt(static_cast<int>(mSlices.size())));
    mCurrentSlice = mSlices[sliceIndex];
    mCurrentSliceIndex = sliceIndex;
    
    sendChangeMessage();
    return mCurrentSlice;
}

std::vector<SliceManager::Slice> const& SliceManager::getSlices() const
{
    return mSlices;
}

size_t SliceManager::getNumberOfSlices() const
{
    return mSlices.size();
}

void SliceManager::performSlice()
{
    auto const bufferLength = getBufferNumSamples();
    if(bufferLength <= 0)
    {
        // no file loaded
        return;
    }
    
    if(mSliceMethod == divisions)
    {
        mSlices.clear();
        
        // divide up
        auto const sliceSampleSize = static_cast<size_t>(static_cast<double>(bufferLength) / mDivisions);
        auto const numSlices = bufferLength / sliceSampleSize;
        
        jassert(numSlices > 0);
        
        mSlices.resize(numSlices);
        for(size_t i = 0; i < numSlices; ++i)
        {
            auto const start = i * sliceSampleSize;
            auto const end = (i == numSlices - 1) ? getBufferNumSamples() : (i + 1) * sliceSampleSize;
            jassert(start < end);
            
            mSlices[i] = {juce::Uuid(), i * sliceSampleSize, (i+1) * sliceSampleSize};
        }
        
        mCurrentSliceIndex = 0;
        mCurrentSlice = mSlices[mCurrentSliceIndex];
    }
    else if(mSliceMethod == transients)
    {
        /*
         TODO:
         This happens on the message thread and can cause problems
         for the audio thread still trying to access it 
         */
        mSlices.clear();
        
        AudioAnalyser::DetectionSettings detectionSettings;
        detectionSettings.sampleRate = static_cast<int>(getSampleSampleRate());
        detectionSettings.threshold = mThreshold;
        
        auto const onsetPositions = AudioAnalyser::getOnsetPositions(*getActiveBuffer(), detectionSettings);
        auto const numSlices = onsetPositions.size();
        jassert(numSlices > 0);
        
        mSlices.resize(numSlices);
        for(size_t i = 0; i < onsetPositions.size(); ++i)
        {
            auto const start = onsetPositions[i];
            auto const end = (i == onsetPositions.size() - 1) ? getBufferNumSamples() : onsetPositions[i+1];
            jassert(start < end);
            
            mSlices[i] = {juce::Uuid(), start, end};
        }
        
        mCurrentSliceIndex = 0;
        mCurrentSlice = mSlices[mCurrentSliceIndex];
    }
    else if(mSliceMethod == manual)
    {
        // hmmm? check slice still exists - update index etc as necessary
        
        // sort in ascending order as we add them into a random position
        std::sort(mSlices.begin(), mSlices.end(), [](const Slice& lhs, const Slice& rhs)
        {
            return std::get<1>(lhs) < std::get<1>(rhs);
        });
    }
    
    sendChangeMessage();
}

void SliceManager::fromXml(juce::XmlElement const& xml)
{
    assert(xml.hasTagName("SliceManager::Slices"));
    
    for(auto* child : xml.getChildIterator())
    {
        if(!child->hasTagName("slice"))
        {
            continue;
        }
        
        auto uuid = juce::Uuid(child->getStringAttribute("uuid", juce::Uuid().toString()));
        auto start = static_cast<size_t>(child->getIntAttribute("start"));
        auto end = static_cast<size_t>(child->getIntAttribute("end"));
        mSlices.push_back({uuid, start, end});
    }
    
    setSliceMethod(manual);
    sendChangeMessage();
}

std::unique_ptr<juce::XmlElement> SliceManager::toXml() const
{
    auto sliceList = std::make_unique<juce::XmlElement>("SliceManager::Slices");
    if(sliceList == nullptr)
    {
        return nullptr;
    }
    
    for(auto const& slice : mSlices)
    {
        auto sliceXml = new juce::XmlElement("slice");
        sliceXml->setAttribute("uuid", std::get<0>(slice).toString());
        sliceXml->setAttribute("start", static_cast<int>(std::get<1>(slice)));
        sliceXml->setAttribute("end", static_cast<int>(std::get<2>(slice)));
        sliceList->addChildElement(sliceXml);
    }
    
    return sliceList;
}
