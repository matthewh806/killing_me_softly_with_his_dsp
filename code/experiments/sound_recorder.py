import pyaudio
import numpy as np
import utils_functions as UF

class SoundRecorder:
    '''
    '''
    def __init__(self):
        self.paudio = pyaudio.PyAudio()
        self.sample_rate = 44100
        self.channels = 1
        self.buffer_size = 512
        self.record_length = 5.0

        print(self.paudio.get_default_host_api_info())
        numdevices = self.paudio.get_default_host_api_info().get('deviceCount')
        for i in range(0, numdevices):
            if self.paudio.get_device_info_by_host_api_device_index(0, i).get('maxInputChannels') > 0:
                print("Input Device id ", i, " - ", self.paudio.get_device_info_by_host_api_device_index(0, i).get('name'))
                

    def start_recording(self):
        self.stream = self.paudio.open(format=pyaudio.paFloat32,
                                        channels=1,
                                        rate=self.sample_rate,
                                        frames_per_buffer=self.buffer_size,
                                        input=True,
                                        output=False,
                                        input_device_index=None)

        #self.stream.start_stream()
        self.recorded_frames = np.array([], dtype=np.float32)

        for i in range(0, int(self.sample_rate / self.buffer_size * self.record_length)):
            data = self.stream.read(self.buffer_size)
            self.recorded_frames = np.append(self.recorded_frames, np.frombuffer(data, dtype=np.float32))

        self.stream.stop_stream()
        self.stream.close()


    def save_recording(self, output_file_path):
        UF.wavwrite(self.recorded_frames, sample_rate=self.sample_rate, filepath=output_file_path)


if __name__ == "__main__":
    import os
    current_file_directory = os.path.dirname(os.path.realpath(__file__))
    output_sounds_directory = os.path.join(current_file_directory, "sounds/output_sounds")
    output_sound_path = os.path.join(output_sounds_directory, "test_recording.wav")

    sound_recorder = SoundRecorder()
    sound_recorder.start_recording()
    sound_recorder.save_recording(output_sound_path)