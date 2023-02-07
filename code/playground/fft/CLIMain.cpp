#include "JuceHeader.h"
#include "FFTWrapper.h"

namespace OUS
{
    void createSineWavetable(std::array<float, FFTWrapper::FFTSize * 2>& sineTable, size_t size)
    {
        auto angleDelta = juce::MathConstants<double>::twoPi / (double) (size - 1);
        auto currentAngle = 0.0;

        for (unsigned int i = 0; i < size; ++i)
        {
            sineTable[i] = static_cast<float>(std::sin (currentAngle));
            currentAngle += angleDelta;
        }
    }
}

using namespace OUS;

int main (int argc, char* argv[])
{
    std::array<float, FFTWrapper::FFTSize * 2> sineTable;
    createSineWavetable(sineTable, static_cast<size_t>(FFTWrapper::FFTSize));
    
    FFTWrapper fftWrapper;
    fftWrapper.performRealForwardTransform(sineTable.data());
    
    auto const& magnitudes = fftWrapper.getMagnitudes();
    auto const& phases = fftWrapper.getPhases();
    
    std::cout << "Magnitudes: \n";
    for(auto magnitude : magnitudes)
    {
        std::cout << magnitude << ", ";
    }
    std::cout << "\n";
    
    std::cout << "Phases: \n";
    for(auto phase : phases)
    {
        std::cout << phase << ",";
    }
    
    return 0;
}
