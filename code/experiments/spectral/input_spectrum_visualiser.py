import sounddevice as sd
from scipy.io import wavfile
import numpy as np
import os
from pynput import keyboard
import specgram_plot

'''
    This script just gives some very basic plotting visualisation
    data for input audio. The input audio can either be specified
    with the optional argument '--file / -i" or if this is omitted
    uses the default input device to record audio

    TODO: Allow arbitrary length input recordings
    TODO: Use better mechanisms for playing / recording
    TODO: Allow the ability to specify log scaling in the output
'''


fs = 44100
seconds = 3
data = []


def on_key_press(event):
    if event.key == "p":
        play_input(data)


def play_input(input_data):
    sd.play(input_data, fs)
    # sd.wait()


def record_input():
    recording = sd.rec(int(seconds * fs), samplerate=fs, channels=1)
    sd.wait()

    return recording


def save_input(out_path):
    wavfile.write(out_path, fs, data)


def validate_file(filepath):
    if not os.path.exists(filepath):
        raise argparse.ArgumentTypeError("{0} does not exist".format(filepath))


def read_file_data(filepath):
    sr, data = wavfile.read(filepath)
    return data


if __name__ == "__main__":
    import argparse
    import sys

    print(sys.argv)
    parser = argparse.ArgumentParser()
    parser.add_argument("--file", "-i", metavar="FILENAME",
                        help="use an input (wav) file to analyse")
    parser.add_argument("--save", "-s", metavar="FILENAME",
                        help="location to save output audio file")
    args = parser.parse_args()

    if args.file:
        print("Use audio file {}".format(args.file))
        validate_file(args.file)
        data = read_file_data(args.file)

        if data.shape[1] == 2:
            data = data[:, 0]

        specgram_plot.plot_data(data, fs, on_key_press)
    else:
        print("Record input")
        data = record_input()
        if args.save:
            print("Saving audio to {}".format(args.save))
            save_input(args.save)

        specgram_plot.plot_data(data[:, 0], fs, on_key_press)

    listener = keyboard.Listener(
        on_press=on_key_press)
    listener.start()
