'''
Inspired by the work done by Ryan Maguire:
https://www.theghostinthemp3.com/theghostinthemp3.html
'''

'''
Steps:
    1. Convert WAV to mp3
    2. Perform STFT of a chunk of the original & mp3
    3. Compare the FFTs visually
    4. Subtract the spectrum of the WAV from the MP3 to get the "residual"
    5. Perform ISTFT to resynthesize the sound and recover the "ghost audio"

    TODO
        1. Handle case where there's more than 1 channel
        2. Handle case where bit depth != 16 (sample_width=2 assumed here!)
        3. figure out a way to convert the in memory stream to mp3 rather than creating the intermediary file
'''

from pydub import AudioSegment
from scipy.signal import stft, istft
import os
import numpy as np
import matplotlib.pyplot as plt

CUR_DIR=os.path.dirname(os.path.realpath(__file__))
INPUT_WAV=os.path.join(CUR_DIR, "../audio/sax-phrase.wav")
INPUT_MP3=os.path.join(CUR_DIR, "../audio/output/sax-phrase.mp3")
OUTPUT_GHOST=os.path.join(CUR_DIR, "../audio/output/sax-ghost.mp3")

def generate_ghosts(input_path, output_path, start=0, end=-1):
    '''
    Generate the ghosts of an mp3

    The general approach here is to subtract the magnitude spectrum 
    (obtained from the STFT) of an uncompressed WAV file from the magnitude 
    spectrum of a compressed MP3 file of the same signal. 
    The residual spectrum is then resynthesised using an ISTFT.

    Note: Both the forward and reverse STFT stages use identical 
          fft / window parameters

    Parameters:
        input_path: path on disk to the uncompressed input file
        output_path: location to store the output ghost mp3 file
        start: start slice position (ms) within the input file
        end: end slice position (ms) within the input file

    Output:
        The resulting ghost file is saved at output_path in mp3 format
    '''

    original_wav = AudioSegment.from_wav(input_path)
    print("Original Size: ", len(original_wav.get_array_of_samples()))
    print("Sample width: ", original_wav.sample_width)

    if end==-1:
        end=len(original_wav.get_array_of_samples())

    original_slice = original_wav[start:end]
    print("Sliced Size: ", len(original_slice.get_array_of_samples()))  

    orig_data = np.float32(np.array(original_slice.get_array_of_samples())) / (2**15 - 1)
    original_slice.export(INPUT_MP3, format="mp3")

    compressed_slice = AudioSegment.from_mp3(INPUT_MP3)
    compressed_data = np.float32(np.array(compressed_slice.get_array_of_samples())) / (2**15 - 1)

    print("original: ", orig_data)
    print("compressed: ", compressed_data)

    plt.figure(1, figsize=(9, 5))
    plt.subplot(4,3,1)
    plt.plot(orig_data, 'b', lw=1.5)

    N=2048  # fft size
    M=128  # window size
    window = "hamming"
    overlap=4

    (freqs_o, times_o, ys_o) = stft(orig_data, fs=original_wav.frame_rate, window=window, nfft=N, nperseg=M, noverlap=overlap, return_onesided=True)
    print(ys_o.shape)
    mX_orig = 20 * np.log10(np.abs(ys_o))
    pX_orig = np.unwrap(np.angle(ys_o))
    plt.subplot(3,3,2)
    plt.pcolormesh(times_o, freqs_o, mX_orig, shading = 'flat')
    plt.xlabel('Time (s)')
    plt.ylabel('Frequency (Hz)')
    plt.subplot(3,3,3)
    plt.pcolormesh(times_o, freqs_o, pX_orig, shading = 'flat')
    plt.xlabel('Time (s)')
    plt.ylabel('Frequency (Hz)')

    plt.subplot(3,3,4)
    plt.plot(compressed_data, 'b', lw=1.5)    
    plt.subplot(3,3,5)
    (freqs_c, times_c, ys_c) = stft(compressed_data, fs=original_wav.frame_rate, window=window, nfft=N, nperseg=M, noverlap=overlap, return_onesided=True)
    mX_comp = 20 * np.log10(np.abs(ys_c))
    pX_comp = np.unwrap(np.angle(ys_o))
    plt.pcolormesh(times_c, freqs_c, mX_comp, shading = 'flat')
    plt.xlabel('Time (s)')
    plt.ylabel('Frequency (Hz)')
    plt.subplot(3,3,6)
    plt.pcolormesh(times_o, freqs_o, pX_comp, shading = 'flat')
    plt.xlabel('Time (s)')
    plt.ylabel('Frequency (Hz)')

    # Subtract the spectra and inverse fft to hear the ghost
    mx_residual = mX_orig - mX_comp
    px_residual = pX_orig - pX_comp
    plt.subplot(3,3,8)
    plt.pcolormesh(times_o, freqs_o, mx_residual)
    plt.xlabel('Time (s)')
    plt.ylabel('Frequency (Hz)')

    plt.subplot(3,3,9)
    plt.pcolormesh(times_o, freqs_o, px_residual)
    plt.xlabel('Time (s)')
    plt.ylabel('Frequency (Hz)')

    # resynthesize residual spectra (what to use for phase? subtraction? regen?)
    residualY = 10**(mx_residual / 20) * np.exp(1j*pX_orig)
    (t_out, x_out) = istft(residualY, fs=original_wav.frame_rate, window=window, nperseg=M, noverlap=overlap, nfft=N, input_onesided=True)
    plt.subplot(3,3,7)
    plt.plot(t_out, x_out)
    plt.xlabel('Time (s)')
    plt.ylabel('Amplitude')
    plt.show()

    # ys_residual = ys_o - ys_c
    # mx_residual = 20 * np.log10(np.abs(ys_residual))
    # plt.pcolormesh(times_o, freqs_o, mx_residual)
    # plt.xlabel('Time (s)')
    # plt.ylabel('Frequency (Hz)')
    # plt.show()

    # # resynthesize residual spectra (what to use for phase? subtraction? regen?)
    # # residualY = 10**(mx_residual / 20) * np.exp(1j*pX_orig)
    # (t_out, x_out) = istft(ys_residual, fs=original_wav.frame_rate, window=window, nperseg=M, noverlap=overlap, nfft=N, input_onesided=True)
    # plt.plot(t_out, x_out)
    # plt.xlabel('Time (s)')
    # plt.ylabel('Amplitude')
    # plt.show()

    y = np.int16(x_out * 2 ** 15)
    ghost_slice = AudioSegment(y.tobytes(), frame_rate=original_wav.frame_rate, sample_width=2, channels=1)
    ghost_slice.export(output_path, format="mp3")


if __name__ == "__main__":
    print("Boo")

    START= 0.75 * 1000
    END  = 1.25 * 1000

    generate_ghosts(INPUT_WAV, OUTPUT_GHOST, START, END)
