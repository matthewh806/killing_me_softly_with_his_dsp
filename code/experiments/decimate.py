import numpy as np
import matplotlib.pyplot as plt

def decimateSignal(input_signal, downsample_factor):
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
      e.g. [1 1.5 2 2.5 3 3.5 4 4.5 5] or drop sample interpolation [1 1 1 1 3 3 3 3 5
    '''

    signal_length = len(input_signal)

    downsampled_signal = input_signal[::downsample_factor]
    downsampled_signal_length = len(downsampled_signal)

    upsampled_signal = np.zeros(signal_length)
    j = 0
    for i in range(0, signal_length, downsample_factor):
        upsampled_signal[i] = downsampled_signal[j]
        j += 1

    return upsampled_signal


if __name__=="__main__":
    input_signal = np.array([1, -1, 2, -2, 3, -3, 4, -4, 5])
    downsample_factor = 4

    print("Input signal: {}".format(input_signal))
    print("Signal Length: {}, Downsample Factor: {}".format(len(input_signal), downsample_factor))

    upsampled_signal = decimateSignal(input_signal, downsample_factor)
    print("Upsampled signal: {}".format(upsampled_signal))
