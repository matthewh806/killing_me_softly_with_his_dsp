#include <cmath>
#include <iostream>

#define PI 3.14159265

float calculateDoppler(float sourceSpeed, float sourceDirection, float sourcePositionX, float observerPositionY)
{
    auto const sourceVelocity = sourceSpeed * sourceDirection;
    auto const calculateFrequencyShift = [](float srcVelocity, float srcPosX, float obsPosY) -> std::tuple<float, float, float>
    {
        float constexpr speedOfSound = 344.0f;

        // work out angle based on source distance
        float angleRelativeToObserver = std::atan2(obsPosY, std::abs(srcPosX));
        if(srcPosX > 0.0f)
        {
            angleRelativeToObserver = PI - angleRelativeToObserver;
        }

        float radialSpeed = srcVelocity * std::cos(angleRelativeToObserver);
        auto const freqShift = speedOfSound / (speedOfSound - radialSpeed);
        return {freqShift, angleRelativeToObserver, radialSpeed};
    };

    auto const newSourceXPositon = sourcePositionX + sourceVelocity;
    auto const oldfreqShiftTuple = calculateFrequencyShift(sourceVelocity, sourcePositionX, observerPositionY);
    auto const newFrequencyTuple = calculateFrequencyShift(sourceVelocity, newSourceXPositon, observerPositionY);
    auto const travelDirection = sourceDirection > 0 ? "+ve" : "-ve";
    auto const increaseOrDecrease = std::get<0>(newFrequencyTuple) > std::get<0>(oldfreqShiftTuple) ? "increase" : "decrease";

    std::cout
        << "radialSpeed=" << std::get<2>(newFrequencyTuple)
        << ", dir=" << travelDirection
        << ", startPos=" << sourcePositionX
        << ", newPos=" << newSourceXPositon
        << ", angle=" << std::get<1>(newFrequencyTuple) * 180.0f / PI
        << ", prevFrequencyRatio=" << std::get<0>(oldfreqShiftTuple)
        << ", frequencyRatio=" << std::get<0>(newFrequencyTuple)
        << ", pitch=" << increaseOrDecrease
        << "\n";

    return std::get<0>(newFrequencyTuple);
}

int main(int argc, char* argv[])
{
    calculateDoppler(10, 1.0, -10, 30);
    calculateDoppler(10, 1.0, -11, 30);
    calculateDoppler(10, 1.0, -9, 30);
    calculateDoppler(10, -1.0, 11, 30);
    calculateDoppler(10, -1.0, -40, 30);
}
