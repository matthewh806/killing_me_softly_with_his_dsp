import numpy as np
import matplotlib.pyplot as plt
import utils_functions as UF

def getQuantisationLevel(bit_depth = 16):
    return 2.0 / (pow(2.0, bit_depth) - 1.0)

def bitCrushSignal(signal, bit_depth = 16):
    ql = getQuantisationLevel(bit_depth)
    bit_crush_fn = lambda x : ql * int(x / ql)
    fn = np.vectorize(bit_crush_fn)

    return fn(signal)

def plotSignal(signal, title="Signal"):
    fig = plt.figure()
    ax1 = fig.add_subplot(111)
    ax1.set_title(title)
    ax1.set_xlabel("Sample")
    ax1.set_ylabel("Amplitude")
    ax1.plot(signal)
    plt.show()

def plotSignals(signal_1, signal_2, title="Signal", signal_1_title="Input Signal", signal_2_title = "Output Signal"):
    # plot multiple signals
    fig = plt.figure()
    ax1 = fig.add_subplot(211)
    ax1.set_title(signal_1_title)
    ax1.set_xlabel("Sample")
    ax1.set_ylabel("Amplitude")
    ax1.plot(signal_1)

    ax2 = fig.add_subplot(212)
    ax2.set_title(signal_2_title)
    ax2.set_xlabel("Sample")
    ax2.set_ylabel("Amplitude")
    ax2.plot(signal_2)

    plt.show()


if __name__=="__main__":
    input_signal = UF.generateSineSignal(frequency=3)
    crushed_signal = bitCrushSignal(input_signal, 2)
    plotSignals(input_signal, crushed_signal)
