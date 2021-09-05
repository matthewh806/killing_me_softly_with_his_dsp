import numpy as np
import matplotlib.pyplot as plt 

def plot_data(signal_data, sample_rate, key_press_fn):
    input_length = signal_data.size
    cmap = plt.get_cmap("viridis")

    fig, ax = plt.subplots()
    pxx, freq, t, cax = ax.specgram(signal_data, Fs=sample_rate, cmap=cmap)
    cbar = plt.colorbar(cax)
    cbar.set_label("Intensidy (dB)")

    ax.set_xlabel("time (s)")
    ax.set_ylabel("frequency (Hz)")

    fig.canvas.mpl_connect("key_press_event", key_press_fn)

    plt.show()


