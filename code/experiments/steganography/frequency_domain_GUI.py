import tkinter as tk
from tkinter import filedialog as fd
from functools import partial
import os, sys
sys.path.append(os.path.join(os.path.dirname(os.path.realpath(__file__)), '../'))
from sound_player import NumpySoundPlayer
from sound_recorder import SoundRecorder
import numpy as np
import json
import frequency_domain

class FrequencySteganographyGUI:
    def __init__(self, master):
        self.master = master
        master.title("Frequency Domain Steganography")
        master.geometry("600x300")

        # load config file if it exists
        
        config_json = self.load_config_file()
        if config_json:
            # set up defaults
            # TODO: check paths still exist
            self.most_recent_input_directory = config_json['input_file_directory']
            self.most_recent_output_directory = config_json['output_file_directory']

        self.numpy_player = NumpySoundPlayer()
        self.sound_recorder = SoundRecorder()

        master.grid_columnconfigure(0, weight = 1)
        master.grid_columnconfigure(1, weight = 1)
        master.grid_rowconfigure(0, weight = 1)

        # TRANSMITTER UI
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

        self.record_secret_message_button = tk.Button(transmitter_frame, text="Record Secret Message", command=self.record_secret_message)
        self.record_secret_message_button.pack()

        self.hide_message_button = tk.Button(transmitter_frame, text="Hide Message", command=self.hide_message, state='disabled')
        self.hide_message_button.pack()

        self.play_combined_sound_button = tk.Button(transmitter_frame, text="Play combined sound", command=self.play_combined_sound, state='disabled')
        self.play_combined_sound_button.pack()

        self.save_combined_sound_button = tk.Button(transmitter_frame, text="Save combined sound", command=self.save_combined_sound, state='disabled')
        self.save_combined_sound_button.pack()
        
        # RECEIVER UI
        receiver_frame = tk.Frame(master, bd=1, bg='green')
        receiver_frame.grid(row=0, column=1, sticky = "nesw")
        receiver_label = tk.Label(receiver_frame, text="Receiver")
        receiver_label.pack()

        self.steganographed_sound_path = tk.StringVar(receiver_frame)
        self.load_steganographed_sound_path_button = tk.Button(receiver_frame, text="Select Steganographed Sound", command=partial(self.browse_sound, self.steganographed_sound_path))
        self.load_steganographed_sound_path_button.pack()

        self.recover_message_button = tk.Button(receiver_frame, text="Recover Message", command=self.recover_message, state='disabled')
        self.recover_message_button.pack()

        self.play_recovered_message_button = tk.Button(receiver_frame, text="Play hidden message", command=self.play_recovered_sound, state='disabled')
        self.play_recovered_message_button.pack()

        self.save_recovered_sound_button = tk.Button(receiver_frame, text="Save recovered sound", command=self.save_recovered_sound, state='disabled')
        self.save_recovered_sound_button.pack()


    def browse_sound(self, out_path):
        input_directory = self.most_recent_input_directory if hasattr(self, 'most_recent_input_directory') else os.path.dirname(os.path.realpath(__file__))
        filename = fd.askopenfilename(title="Open audio file", initialdir=input_directory, filetypes=(('wav files', '*.wav'),))
        self.most_recent_input_directory = os.path.dirname(filename)
        out_path.set(filename)
        self.update_button_states()


    def record_secret_message(self):
        if self.sound_recorder.recording():
            self.secret_message_path.set(os.path.join(os.path.dirname(os.path.realpath(__file__)), "secret_message.wav"))
            self.sound_recorder.stop_recording()
            self.sound_recorder.save_recording(self.secret_message_path.get())
        else:
            self.sound_recorder.start_recording_non_blocking()
        
        self.update_button_states()

    
    def hide_message(self):
        # TODO: Check paths are valid & mono
        self.steganographed_sound_path.set(os.path.join(os.path.dirname(os.path.realpath(__file__)), "steganographed_audio.wav"))
        self.transmitter = frequency_domain.Transmitter(self.base_sound_path.get(), self.secret_message_path.get(), lpf_cutoff=14000.0, order=96)
        self.transmitter.perform()
        self.transmitter.write(self.steganographed_sound_path.get())
        self.update_button_states()


    def recover_message(self):
        self.receiver = frequency_domain.Receiver(self.steganographed_sound_path.get(), order=48, bpf_lowcutoff=18500.0, bpf_highcutoff=21500.0)
        self.receiver.perform(carrier_frequency = 20000.0)
        self.update_button_states()


    def play_combined_sound(self):
        if not hasattr(self, 'transmitter'):
            return

        if not hasattr(self.transmitter, 'combined_signal'):
            return

        self.play_sound(self.transmitter.combined_signal)


    def play_recovered_sound(self):
        if not hasattr(self, 'receiver'):
            return

        if not hasattr(self.receiver, "recovered_message_signal"):
            return

        self.play_sound(self.receiver.recovered_message_signal)


    def play_sound(self, numpy_array):
        if self.numpy_player.processing():
            return

        self.numpy_player.start_processing_non_blocking(numpy_array, sample_rate=44100)


    def save_combined_sound(self):
        if not hasattr(self, 'transmitter'):
            return

        output_directory = self.most_recent_output_directory if hasattr(self, 'most_recent_output_directory') else os.path.dirname(os.path.realpath(__file__))
        path = fd.asksaveasfilename(initialdir=output_directory)
        self.most_recent_output_directory = os.path.dirname(path)
        self.transmitter.write(path, True)


    def save_recovered_sound(self):
        if not self.receiver:
            return

        output_directory = self.most_recent_output_directory if hasattr(self, 'most_recent_output_directory') else os.path.dirname(os.path.realpath(__file__))
        path = fd.asksaveasfilename(initialdir=output_directory)
        self.most_recent_output_directory = os.path.dirname(path)
        self.receiver.write(path)


    def update_button_states(self):
        if self.sound_recorder.recording():
            self.record_secret_message_button.config(text="Stop Recording")
        else:
            self.record_secret_message_button.config(text="Start Recording")

        have_base_sound = self.base_sound_path.get() is not ''
        have_message_sound = self.secret_message_path.get() is not ''
        self.hide_message_button.config(state='normal' if have_base_sound and have_message_sound else 'disabled')

        performed_hiding = hasattr(self, 'transmitter')
        self.play_combined_sound_button.config(state='normal' if performed_hiding else 'disabled')
        self.save_combined_sound_button.config(state='normal' if performed_hiding else 'disabled')
        
        have_combined_sound = self.steganographed_sound_path.get() is not ''
        self.recover_message_button.config(state='normal' if have_combined_sound else 'disabled')

        performed_recovery = hasattr(self, 'receiver')
        self.play_recovered_message_button.config(state='normal' if performed_recovery else 'disabled')
        self.save_recovered_sound_button.config(state='normal' if performed_recovery else 'disabled')


    def save_config_file(self):
        # store in current directory for now
        current_file_directory = os.path.dirname(os.path.realpath(__file__))
        config_file_path = os.path.join(current_file_directory, ".config.json")

        current_config = self.load_config_file()
        if current_config:
            input_directory = current_config['input_file_directory']
            output_directory = current_config['output_file_directory']

            data = {
                'input_file_directory': self.most_recent_input_directory if hasattr(self, 'most_recent_input_directory') else input_directory,
                'output_file_directory': self.most_recent_output_directory if hasattr(self, 'most_recent_output_directory') else output_directory
            }
        else:
            data = {
                'input_file_directory': self.most_recent_input_directory if hasattr(self, 'most_recent_input_directory') else os.path.dirname(os.path.realpath(__file__)),
                'output_file_directory': self.most_recent_output_directory if hasattr(self, 'most_recent_output_directory') else os.path.dirname(os.path.realpath(__file__))
            }

        with open(config_file_path, 'w') as json_file:
            json.dump(data, json_file)


    def load_config_file(self):
        # returns json object
        current_file_directory = os.path.dirname(os.path.realpath(__file__))
        config_file_path = os.path.join(current_file_directory, ".config.json")
        if not os.path.exists(config_file_path):
            return None

        with open(config_file_path, 'r') as json_file:
            return json.load(json_file)


    def on_destroy(self, event):
        if event.widget == self.master:
            self.save_config_file()


if __name__ == "__main__":
    root = tk.Tk()
    gui = FrequencySteganographyGUI(root)
    root.bind('<Destroy>', func=gui.on_destroy)
    root.mainloop()