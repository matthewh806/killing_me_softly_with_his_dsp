#include "SliceManager.h"
#include <algorithm>

SliceManager::SliceManager(juce::AudioFormatManager& formatManager, Method sliceMethod)
: SampleManager(formatManager)
, mSliceMethod(sliceMethod)
{
    performSlice();
    sanitiseSlices();
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
        sanitiseSlices();
    }
}

void SliceManager::setThreshold(float threshold)
{
    mThreshold = threshold;
    
    if(mSliceMethod == Method::transients)
    {
        performSlice();
        sanitiseSlices();
    }
}

void SliceManager::setDivisions(float divisions)
{
    mDivisions = divisions;
    
    if(mSliceMethod == Method::divisions)
    {
        performSlice();
        sanitiseSlices();
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
        
        sanitiseSlices();
    }
}

void SliceManager::moveSlice(juce::Uuid sliceid, int sampleDelta)
{
    auto* slice = getSliceById(sliceid);
    // check?
    
    // Make sure it doesn't go less than > beyond buffer length
    auto const newPos = static_cast<int>(std::get<1>(*slice)) + sampleDelta;
    std::get<1>(*slice) = static_cast<size_t>(std::max(0, std::min(newPos, static_cast<int>(getBufferNumSamples()))));
    sanitiseSlices();
    sendChangeMessage();
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
    sanitiseSlices();
    
    sendChangeMessage();
}

size_t SliceManager::getCurrentSliceIndex() const
{
    return mCurrentSliceIndex;
}

SliceManager::Slice& SliceManager::getCurrentSlice()
{
    return mCurrentSlice;
}

SliceManager::Slice* SliceManager::getSliceById(juce::Uuid& id)
{
    auto const itr = std::find_if(mSlices.begin(), mSlices.end(), [&](const Slice& s)
    {
        return std::get<0>(s) == id;
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
        
        auto const onsetPositions = AudioAnalyser::getOnsetPositions(*getForwardBuffer(), detectionSettings);
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
        if(mSlices.size() == 0)
        {
            // Create just one from beginning to end when there are no slices
            mSlices.push_back({juce::Uuid(), 0, getBufferNumSamples()});
            
            mCurrentSliceIndex = 0;
            mCurrentSlice = mSlices[mCurrentSliceIndex];
        }
        
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

void SliceManager::sanitiseSlices()
{
    std::sort(mSlices.begin(), mSlices.end(), [](const Slice& lhs, const Slice& rhs)
    {
        return std::get<1>(lhs) < std::get<1>(rhs);
    });
    
    // 1. fix start / end times
    // 2. sort in order
    
    // Set end value first
    
    for(auto itr = mSlices.begin(); itr != mSlices.end(); ++itr)
    {
        auto const nextItr = std::next(itr);
        if(nextItr == mSlices.end())
        {
            // last slice so set to file end sample
            std::get<2>(*itr) = getBufferNumSamples();
        }
        else
        {
            std::get<2>(*itr) = std::get<1>(*nextItr);
        }
        
//        jassert(std::get<1>(*itr) < std::get<2>(*itr));
    }
}

void SliceManager::clearSlices()
{
    mSlices.clear();
}
