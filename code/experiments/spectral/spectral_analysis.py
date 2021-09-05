import numpy as np
import matplotlib.pyplot as plt
from scipy.io import wavfile

filepath='/home/matthew/.local/share/SuperCollider/downloaded-quarks/Dirt-Samples/bleep/shortsaxish.wav'

if __name__ == "__main__":
    sr, data = wavfile.read(filepath)
    print(data.shape[0])

    plt.subplot(211)
    plt.title("Spectrogram of wav file")
    plt.xlabel('sample')
    plt.ylabel('amplitude')
    plt.plot(data)

    plt.subplot(212)
    plt.xlabel('sample')
    plt.ylabel('Frequency')
    plt.specgram(data[:, 0], Fs=sr)

    plt.show()

