import pyaudio
import numpy as np
import utils_functions as UF

class SoundRecorder:
    '''
    A simple sound recorder class based around pyaudio for recording a mono wav file from 
    the default input device

    Note:
        - Mono
        - records float_32 format
        - fixed sample rate (44100 Hz)
        - fixed (default) input device
    '''

    def __init__(self):
        self.paudio = pyaudio.PyAudio()
        self.sample_rate = 44100
        self.channels = 1
        self.buffer_size = 512
        self.record_length = 5.0
        self.recorded_frames = np.array([], dtype=np.float32)

        # For querying the device
        # print(self.paudio.get_default_host_api_info())
        # numdevices = self.paudio.get_default_host_api_info().get('deviceCount')
        # for i in range(0, numdevices):
        #     if self.paudio.get_device_info_by_host_api_device_index(0, i).get('maxInputChannels') > 0:
        #         print("Input Device id ", i, " - ", self.paudio.get_device_info_by_host_api_device_index(0, i).get('name'))


    def start_recording_non_blocking(self):
        '''
        Start recording audio from the default input device

        This will continue streaming audio input until the stop_recording() method is called
        Note: This process is non-blocking meaning it will be called on a different audio thread
        to the main one
        '''

        self.recorded_frames = np.array([], dtype=np.float32)
        self.stream = self.paudio.open(format=pyaudio.paFloat32,
                                        channels=self.channels,
                                        rate=self.sample_rate,
                                        frames_per_buffer=self.buffer_size,
                                        input=True,
                                        output=False,
                                        input_device_index=None,
                                        stream_callback=self._pyaudio_callback)

        print("Start Recording")
        self.stream.start_stream()


    def save_recording(self, output_file_path):
        '''
        Save the recording as a wav file to output_file_path
        '''
        UF.wavwrite(self.recorded_frames, sample_rate=self.sample_rate, filepath=output_file_path)


    def recording(self):
        '''
        Checks if pyaudio is current recording

        Returns True / False
        '''
        if not hasattr(self, "stream"):
            return False

        if self.stream is None:
            return False

        return self.stream.is_active()

    
    def stop_recording(self):
        '''
        If streaming will stop any current recording and close the stream object
        '''
        if not hasattr(self, "stream"):
            return

        if self.stream is None:
            return False

        self.stream.stop_stream()
        self.stream.close()
        self.stream = None


    def _pyaudio_callback(self, in_data, frame_count, time_info, status):
        self.recorded_frames = np.append(self.recorded_frames, np.frombuffer(in_data, dtype=np.float32))
        return in_data, pyaudio.paContinue


if __name__ == "__main__":
    import os, time
    current_file_directory = os.path.dirname(os.path.realpath(__file__))
    output_sounds_directory = os.path.join(current_file_directory, "sounds/output_sounds")
    output_sound_path = os.path.join(output_sounds_directory, "test_recording.wav")

    sound_recorder = SoundRecorder()
    sound_recorder.start_recording_non_blocking()

    try:
        while sound_recorder.recording():
            time.sleep(0.1)
    except KeyboardInterrupt:
        print("Stopped Recording")

    sound_recorder.save_recording(output_sound_path)