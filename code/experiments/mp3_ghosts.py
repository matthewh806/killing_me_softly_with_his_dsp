'''
Inspired by the work done by Ryan Maguire:
https://www.theghostinthemp3.com/theghostinthemp3.html
'''

'''
Steps:
    1. Convert WAV to mp3
    2. Perform STFT of a chunk of the original & mp3
    3. Subtract the spectrum of the WAV from the MP3 to get the "residual"
    4. Perform ISTFT to resynthesize the sound and recover the "ghost audio"
    5. Save ghost mp3 to output location
    6. Plot everything

    TODO
        1. Handle case where there's more than 1 channel
        2. Handle case where bit depth != 16 (sample_width=2 assumed here!)
        3. Figure out a way to convert the in memory stream to mp3 rather than creating the intermediary file
        4. Expose FFT params as arguments, 
        5. Specify compression amount
        6. CLI
'''

from pydub import AudioSegment
from scipy.signal import stft, istft
import os
import numpy as np
import matplotlib.pyplot as plt
import logging
import tempfile

CUR_DIR=os.path.dirname(os.path.realpath(__file__))

logging.basicConfig()
logger = logging.getLogger('mp3_ghosts')
logger.setLevel(level=logging.DEBUG)

def generate_ghosts(input_path, output_path, start=0, end=-1, bitrate='320', fft_size=2048, window="hamming", window_size=129, overlap=4):
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
        bitrate: the bitrate to use for wav->mp3 conversion (kbps)
        fft_size: the size of the FFT sample buffer (should be a power of 2)
        window; the type of window to use in the STFT (string)
        window_size: the size of the window to use in the STFT
        overlap: The overlap factor of the windows

    Output:
        The resulting ghost file is saved at output_path in mp3 format
    '''

    # 1. Load uncompressed audio, create compressed mp3
    original_wav = AudioSegment.from_wav(input_path)
    logger.info("-----------INPUT INFO-----------")
    logger.info("Generating audio data from: %s", input_path)
    logger.info("Original Size: %i", len(original_wav.get_array_of_samples()))
    logger.info("Sample Rate: %i Hz", original_wav.frame_rate)
    logger.info("Channels: %i", original_wav.channels)
    logger.info("Sample width: %i, Bit depth: %i", original_wav.sample_width, original_wav.sample_width*8)
    logger.info("--------------------------------")

    # Use signal length if end not specified 
    if end==-1:
        end=len(original_wav) # AudioSegment.__len__ returns length in ms

    if start >= end:
        raise ValueError("Slice start must be < slice end")

    if start < 0:
        logger.warn("Slice start < 0")

    input_slice = original_wav[start:end]
    logger.info("Extracting slice: start=%fs, end=%fs", start/1000.0, end/1000.0)
    logger.info("Sliced Size: %i", len(input_slice.get_array_of_samples()))  
    input_slice_data = np.float32(np.array(input_slice.get_array_of_samples())) / (2**15 - 1)

    temp_file = tempfile.NamedTemporaryFile(suffix='.mp3')
    input_slice.export(temp_file, format="mp3", bitrate=bitrate)

    compressed_slice = AudioSegment.from_mp3(temp_file)
    compressed_data = np.float32(np.array(compressed_slice.get_array_of_samples())) / (2**15 - 1)
    compressed_data = compressed_data[:len(input_slice.get_array_of_samples())] # force same size

    # 2. Perform STFTs of both original and compressed signal
    N=fft_size      # fft size
    M=window_size   # window size
    noverlap=window_size//overlap

    logger.info("-----------FFT INFO-------------")
    logger.info("FFT size: %i", N)
    logger.info("Window size: %i", M)
    logger.info("Window type: %s", window)
    logger.info("Overlap factor: %i, samples: %i", overlap, noverlap)
    logger.info("--------------------------------")

    (freqs_i, times_i, ys_i) = stft(input_slice_data, fs=original_wav.frame_rate, window=window, nfft=N, nperseg=M, noverlap=noverlap, return_onesided=True)
    mX_orig = 20 * np.log10(np.abs(ys_i))
    pX_orig = np.unwrap(np.angle(ys_i))

    (freqs_c, times_c, ys_c) = stft(compressed_data, fs=original_wav.frame_rate, window=window, nfft=N, nperseg=M, noverlap=noverlap, return_onesided=True)
    mX_comp = 20 * np.log10(np.abs(ys_c))
    pX_comp = np.unwrap(np.angle(ys_c))

    # 3. Subtract the spectra
    residual_spectra = ys_i - ys_c

    # Just for plotting
    mX_residual = 20 * np.log10(np.abs(residual_spectra)) 
    pX_residual = pX_orig - pX_comp

    # 4. Resynthesize ghost spectra 
    # (what to use for phase? subtraction? regen?)
    (t_out, x_out) = istft(residual_spectra, fs=original_wav.frame_rate, window=window, nperseg=M, noverlap=noverlap, nfft=N, input_onesided=True)

    # 6. Save ghost mp3
    y = np.int16(x_out * 2 ** 15)
    ghost_slice = AudioSegment(y.tobytes(), frame_rate=original_wav.frame_rate, sample_width=2, channels=1)
    ghost_slice.export(output_path, format="mp3")

    # 5. plot everything
    plt.figure(1, figsize=(15, 9))
    times=np.linspace(start / 1000.0, end / 1000.0, num=len(input_slice_data))

    # Uncompressed
    plt.subplot(3,3,1)
    plt.xlabel('Time (s)')
    plt.ylabel('Amplitude')
    plt.title('Uncompressed Signal')
    plt.plot(times, input_slice_data)
    plt.subplot(3,3,2)
    plt.pcolormesh(times_i, freqs_i, mX_orig, shading='auto')
    plt.xlabel('Time (s)')
    plt.ylabel('Frequency (Hz)')
    plt.title('Uncompressed Magnitude Spectrum')
    plt.subplot(3,3,3)
    plt.pcolormesh(times_i, freqs_i, pX_orig, shading='auto')
    plt.xlabel('Time (s)')
    plt.ylabel('Frequency (Hz)')
    plt.title('Uncompressed Phase Spectrum')

    # Compressed    
    plt.subplot(3,3,4)
    plt.plot(times, compressed_data)
    plt.xlabel('Time (s)')
    plt.ylabel('Amplitude')    
    plt.title('Compressed Signal')
    plt.subplot(3,3,5)
    plt.pcolormesh(times_c, freqs_c, mX_comp, shading='auto')
    plt.xlabel('Time (s)')
    plt.ylabel('Frequency (Hz)')
    plt.title('Compressed Frequency Spectrum')
    plt.subplot(3,3,6)
    plt.pcolormesh(times_c, freqs_c, pX_comp, shading='auto')
    plt.xlabel('Time (s)')
    plt.ylabel('Frequency (Hz)')
    plt.title('Compressed Phase Spectrum')

    # Ghost
    plt.subplot(3,3,7)
    plt.plot(times, x_out[:len(input_slice_data)])
    plt.xlabel('Time (s)')
    plt.ylabel('Amplitude')
    plt.title("Ghost Signal")
    plt.subplot(3,3,8)
    plt.pcolormesh(times_i, freqs_i, mX_residual, shading='auto')
    plt.xlabel('Time (s)')
    plt.ylabel('Frequency (Hz)')
    plt.title('Ghost Magnitude Spectrum')
    plt.title
    plt.subplot(3,3,9)
    plt.pcolormesh(times_i, freqs_i, pX_residual, shading='auto')
    plt.xlabel('Time (s)')
    plt.ylabel('Frequency (Hz)')
    plt.title('Ghost Phase Spectrum')

    plt.tight_layout()
    plt.show()

if __name__ == "__main__":
    print("Boo")
    input_file=os.path.join(CUR_DIR, "sounds/raga_hop.wav")

    import pathlib
    infile = pathlib.Path(input_file).stem
    output_path=os.path.join(CUR_DIR, "sounds/output_sounds/{}_ghost.mp3".format(infile))

    N=2048
    M=511
    window="hamming"
    overlap=2
    bitrate="8"

    generate_ghosts(input_file, output_path, fft_size=N, window=window, window_size=M, overlap=overlap, bitrate=bitrate)
