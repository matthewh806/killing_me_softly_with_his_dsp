import tkinter as tk
from tkinter import filedialog as fd
from functools import partial
import pyaudio
import numpy as np
import frequency_domain

class FrequencySteganographyGUI:
    def __init__(self, master):
        self.master = master
        master.title("Frequency Domain Steganography")
        master.geometry("600x300")

        master.grid_columnconfigure(0, weight = 1)
        master.grid_columnconfigure(1, weight = 1)
        master.grid_rowconfigure(0, weight = 1)

        transmitter_frame = tk.Frame(master, bd=1, bg='red')
        transmitter_frame.grid(row=0, column=0, sticky = "nesw")
        transmitter_label = tk.Label(transmitter_frame, text="Transmitter")
        transmitter_label.pack(side=tk.TOP)

        self.base_sound_path = tk.StringVar(transmitter_frame)
        self.base_sound_path_button = tk.Button(transmitter_frame, text="Select Base Sound", command=partial(self.browse_sound, self.base_sound_path))
        self.base_sound_path_button.pack()

        self.secret_message_path = tk.StringVar(transmitter_frame)
        self.secret_message_path_button = tk.Button(transmitter_frame, text="Select Secret Message", command=partial(self.browse_sound, self.secret_message_path))
        self.secret_message_path_button.pack()

        self.hide_message_button = tk.Button(transmitter_frame, text="Hide Message", command=self.hide_message)
        self.hide_message_button.pack()

        self.play_combined_sound_button = tk.Button(transmitter_frame, text="Play combined sound", command=self.play_combined_sound)
        self.play_combined_sound_button.pack()

        receiver_frame = tk.Frame(master, bd=1, bg='green')
        #receiver_frame.pack()
        receiver_frame.grid(row=0, column=1, sticky = "nesw")
        receiver_label = tk.Label(receiver_frame, text="Receiver")
        receiver_label.pack()


    def browse_sound(self, out_path):
        filename = fd.askopenfilename(title="Open audio file", initialdir="/", filetypes=(('wav files', '*.wav'),))
        out_path.set(filename)

    
    def hide_message(self):
        # TODO: Check paths are valid & mono
        self.transmitter = frequency_domain.Transmitter(self.base_sound_path.get(), self.secret_message_path.get())
        self.transmitter.perform()


    def play_combined_sound(self):
        if not self.transmitter:
            return

        p = pyaudio.PyAudio()
        stream = p.open(format=pyaudio.paFloat32, channels=1, rate=44100, frames_per_buffer=1024, output=True, output_device_index=1)
        stream.write(self.transmitter.combined_signal.astype(np.float32).tostring())


if __name__ == "__main__":
    root = tk.Tk()
    gui = FrequencySteganographyGUI(root)
    root.mainloop()