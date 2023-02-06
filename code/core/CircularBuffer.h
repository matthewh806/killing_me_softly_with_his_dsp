#pragma once

/*
	This class is taken directly from the Pirkle AudioSFX code repository
*/

namespace OUS
{
	inline double doLinearInterpolation(double y1, double y2, double fractional_X)
	{
		// --- check invalid condition
		if (fractional_X >= 1.0) return y2;

		// --- use weighted sum method of interpolating
		return fractional_X*y2 + (1.0 - fractional_X)*y1;
	}

	template <typename T>
	class CircularBuffer
	{
	public:
		CircularBuffer() {}		/* C-TOR */
		~CircularBuffer() {}	/* D-TOR */

								/** flush buffer by resetting all values to 0.0 */
		void flushBuffer(){ memset(&buffer[0], 0, bufferLength * sizeof(T)); }

		/** Create a buffer based on a target maximum in SAMPLES
		//	   do NOT call from realtime audio thread; do this prior to any processing */
		void createCircularBuffer(unsigned int _bufferLength)
		{
			// --- find nearest power of 2 for buffer, and create
			createCircularBufferPowerOfTwo((unsigned int)(pow(2, ceil(log(_bufferLength) / log(2)))));
		}

		/** Create a buffer based on a target maximum in SAMPLESwhere the size is
			pre-calculated as a power of two */
		void createCircularBufferPowerOfTwo(unsigned int _bufferLengthPowerOfTwo)
		{
			// --- reset to top
			writeIndex = 0;

			// --- find nearest power of 2 for buffer, save it as bufferLength
			bufferLength = _bufferLengthPowerOfTwo;

			// --- save (bufferLength - 1) for use as wrapping mask
			wrapMask = bufferLength - 1;

			// --- create new buffer
			buffer.reset(new T[bufferLength]);

			// --- flush buffer
			flushBuffer();
		}

		/** write a value into the buffer; this overwrites the previous oldest value in the buffer */
		void writeBuffer(T input)
		{
			// --- write and increment index counter
			buffer[writeIndex++] = input;

			// --- wrap if index > bufferlength - 1
			writeIndex &= wrapMask;
		}

		/** read an arbitrary location that is delayInSamples old */
		T readBuffer(int delayInSamples)//, bool readBeforeWrite = true)
		{
			// --- subtract to make read index
			//     note: -1 here is because we read-before-write,
			//           so the *last* write location is what we use for the calculation
			unsigned int readIndex = (writeIndex - 1) - static_cast<unsigned int>(delayInSamples);

			// --- autowrap index
			readIndex &= wrapMask;

			// --- read it
			return buffer[readIndex];
		}

		/** read an arbitrary location that includes a fractional sample */
		T readBuffer(double delayInFractionalSamples)
		{
			// --- truncate delayInFractionalSamples and read the int part
			T y1 = readBuffer((int)delayInFractionalSamples);

			// --- if no interpolation, just return value
			if (!interpolate) return y1;

			// --- else do interpolation
			//
			// --- read the sample at n+1 (one sample OLDER)
			T y2 = readBuffer((int)delayInFractionalSamples + 1);

			// --- get fractional part
			double fraction = delayInFractionalSamples - (int)delayInFractionalSamples;

			// --- do the interpolation (you could try different types here)
			return doLinearInterpolation(y1, y2, fraction);
		}

		/** enable or disable interpolation; usually used for diagnostics or in algorithms that require strict integer samples times */
		void setInterpolate(bool b) { interpolate = b; }

	private:
		std::unique_ptr<T[]> buffer = nullptr;	///< smart pointer will auto-delete
		unsigned int writeIndex = 0;		///> write index
		unsigned int bufferLength = 1024;	///< must be nearest power of 2
		unsigned int wrapMask = 1023;		///< must be (bufferLength - 1)
		bool interpolate = true;			///< interpolation (default is ON)
	};
}

