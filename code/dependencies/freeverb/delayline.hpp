// Delayline declaration
//
// Written by Matthew in Paris, December 2022
// http://theofficeofunspecifiedservices.com/
// This code is (also) public domain

#ifndef _delayline_
#define _delayline_

#include <vector>

class delayline
{
public:
    delayline();
    void setDelayTime(float ms);
    float getDelayTime();
    inline float process(float inp);
    
private:
    std::vector<float> mBuffer;
    int mBufferIdx;
};

inline float delayline::process(float input)
{
    float output = mBuffer[mBufferIdx];
    mBuffer[mBufferIdx] = input;
    
    if(++mBufferIdx >= mBuffer.size())
    {
        mBufferIdx = 0;
    }
    
    return output;
}

#endif
