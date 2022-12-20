// Delayline implementation
//
// Written by Matthew in Paris, December 2022
// http://theofficeofunspecifiedservices.com/
// This code is (also) public domain

#include "delayline.hpp"
#include <cmath>

delayline::delayline()
{
    mBufferIdx = 0;
    setDelayTime(30);
}

void delayline::setDelayTime(float ms)
{
    mBuffer.resize(int(std::ceil(ms/1000.0f * 44100.0f)), 0.0f); // For a 30 msec pre delay
    
    if(mBufferIdx >= mBuffer.size())
    {
        // This is a dumb naive approach, FIX
        mBufferIdx = 0.0;
    }
}

float delayline::getDelayTime()
{
    return (mBuffer.size() / 44100.0f) * 1000.0f;
}


