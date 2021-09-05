import numpy as np
import matplotlib.pyplot as plt
from scipy.fft import fft, fftfreq

if __name__ == "__main__":
    N = 600 # no sample points.
    T = 1.0 / 800.0 # sample spacing
    x = np.linspace(0.0, N*T, N, endpoint=False)
    y = np.sin(5.0*2.0*np.pi*x) + 0.5*np.sin(80.0*2.0*np.pi*x)

    yf = fft(y)
    xf = fftfreq(N, T)[:N//2]

    plt.plot(xf, 2.0/N * np.abs(yf[0:N//2]))
    plt.show()



    

