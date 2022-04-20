#include "DopplerShiftPlugin.h"

AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new DopplerShiftProcessor();
}
