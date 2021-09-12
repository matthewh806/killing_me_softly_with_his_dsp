import numpy as np
import matplotlib.pyplot as plt
from enum import Enum

class InterpolationMethod(Enum):
    NONE = 1
    LINEAR = 2
    DROP_SAMPLE = 3

def zeroInterpolate(input_signal, output_signal_length, downsample_factor):
    '''
    Simple repopulate with zeros in between
    '''
    output_signal = np.zeros(output_signal_length)

    j = 0
    for i in range(0, output_signal_length, downsample_factor):
        output_signal[i] = input_signal[j]
        j += 1

    return output_signal

def linearInterpolate(input_signal, output_signal_length, downsample_factor):
    '''
    Do a linear interpolation between neighbouring values in the signal

    Doesn't handle signals where the final index will not be the end of the
    output signal length. (See next_value var and figure out how to assign it properly)
    '''
    output_signal = np.zeros(output_signal_length)

    j = 0
    for i in range(0, output_signal_length, downsample_factor):
        value = input_signal[j]
        next_value = input_signal[j+1] if (j+1 < len(input_signal)) else 0.0

        end_idx = min([i+downsample_factor, output_signal_length])

        step_inc = (next_value - value) / (float(downsample_factor))
        interpolated_value = value
        for k in range(i, end_idx):
            output_signal[k] = interpolated_value
            interpolated_value += step_inc
        j += 1

    return output_signal

def dropSampleInterpolate(input_signal, output_signal_length, downsample_factor):
    '''
    Repopulate inbetween values with the value at each index of the input signal
    '''
    output_signal = np.zeros(output_signal_length)

    j = 0
    for i in range(0, output_signal_length, downsample_factor):
        value = input_signal[j]

        end_idx = min([i+downsample_factor, output_signal_length])
        for k in range(i, end_idx):
            output_signal[k] = value
        j += 1

    return output_signal

def decimateSignal(input_signal, downsample_factor, interpolation_method = InterpolationMethod.NONE):
    '''
    The most simple approach to decimating a signal

    Given an input_signal & an integer downsample_factor

    - Create a downsampled signal consisting of every Nth sample (where N is the decimation factor)
      I.e. take samples at indexes: 0, N, 2N, ... , kN where k is the last index divisible by N
    - Upsample it again to the size of the original signal by inserting
      (downsample_factor - 1) zeros between the values in the downsampled signal_length

      Results in:
      Input -> downsampler -> output

      [ 1 -1  2 -2  3 -3  4 -4  5] -> downsampler -> [1 0 0 0 3 0 0 0 5]
      The intermediary downsampled signal would be: [1 3 5] in this example.

      A more advanced scheme would introduce interpolation to the upsampled signal,
      e.g. [1 1.5 2 2.5 3 3.5 4 4.5 5] or drop sample interpolation [1 1 1 1 3 3 3 3 5]
    '''

    signal_length = len(input_signal)

    downsampled_signal = input_signal[::downsample_factor]
    downsampled_signal_length = len(downsampled_signal)

    print("Downsampled signal: {}".format(downsampled_signal))

    if interpolation_method == InterpolationMethod.LINEAR:
        return linearInterpolate(downsampled_signal, signal_length, downsample_factor)
    elif interpolation_method == InterpolationMethod.DROP_SAMPLE:
        return dropSampleInterpolate(downsampled_signal, signal_length, downsample_factor)

    return zeroInterpolate(downsampled_signal, signal_length, downsample_factor)


if __name__=="__main__":
    input_signal = np.array([1, -1, 2, -2, 3, -3, 4, -4, 5])
    downsample_factor = 2

    print("Input signal: {}".format(input_signal))
    print("Signal Length: {}, Downsample Factor: {}".format(len(input_signal), downsample_factor))

    upsampled_signal = decimateSignal(input_signal, downsample_factor, InterpolationMethod.LINEAR)
    print("Upsampled signal: {}".format(upsampled_signal))
