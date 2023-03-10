#pragma once

#include "CircularBuffer.h"

namespace OUS
{
    template <typename T, const size_t N>
    class MultitapCircularBuffer
    {
    public:
        MultitapCircularBuffer(unsigned int bufferLength, std::array<float, N> tapDelays)
        {
            mTapDelays = tapDelays;
            mCircularBuffer.createCircularBuffer(bufferLength);
        }
    
        ~MultitapCircularBuffer()
        {
        }
        
        void clearBuffer()
        {
            mCircularBuffer.flushBuffer();
        }
        
        size_t getNumberOfTaps()
        {
            return mTapDelays.size();
        }
        
        void setTapDelay(size_t tapIndex, float delaySamples)
        {
            assert(tapIndex < mTapDelays.size());
            if(std::abs(mTapDelays[tapIndex] - delaySamples) > std::numeric_limits<float>::epsilon())
            {
                mTapDelays[tapIndex] = delaySamples;
            }
        }
        
        // forwarding methods
        void writeBuffer(T input)
        {
            mCircularBuffer.writeBuffer(input);
        }
        
        T readTap(size_t tapIndex)
        {
            // TODO: range checking
            assert(tapIndex < mTapDelays.size());
            return mCircularBuffer.readBuffer(mTapDelays[tapIndex]);
        }

    private:
        std::array<float, N> mTapDelays;
        CircularBuffer<T> mCircularBuffer;
    };    
}
