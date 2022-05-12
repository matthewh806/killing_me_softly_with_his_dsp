import pyaudio
import numpy as np

class NumpySoundPlayer:
    '''
    A simple sound player class based around pyaudio for streaming a 
    numpy array to the default sound device

    Note: 
        - Works only for single channel data
        - expects the numpy array to be in float32
        - block size is fixed at 1024
    '''

    def __init__(self):
        self.paudio = pyaudio.PyAudio()
        self.cycle_count = 0


    def start_processing_non_blocking(self, sound_array, sample_rate=44100.0):
        '''
        Start streaming the audio contained in sound_array using the provided
        sample rate

        In order to terminate the before the end processing call stop_processing()
        Note this process is non-blocking meaning it will be called on a different thread 
        to the main one
        '''
        self.sound_source = sound_array
        self.stream = self.paudio.open(format=pyaudio.paFloat32, 
                                        channels=1, 
                                        rate=sample_rate, 
                                        frames_per_buffer=1024, 
                                        input=False,
                                        output=True, 
                                        output_device_index=None, 
                                        stream_callback=self._pyaudio_callback)

        self.stream.start_stream()


    def processing(self):
        if not hasattr(self, "stream"):
            return False

        if self.stream is None:
            return False

        return self.stream.is_active()
        

    def stop_processing(self):
        '''
        Abort the audio streaming if currently running
        '''
        if not hasattr(self, "stream"):
            return

        if self.stream is None:
            return False

        self.stream.stop_stream()
        self.stream.close()
        self.stream = None

        self.cycle_count = 0
        self.sound_source = np.array([], dtype=np.float32)


    def _pyaudio_callback(self, in_data, frame_count, time_info, status):
        audio_len = len(self.sound_source)

        if frame_count * self.cycle_count > audio_len:
            print("processing complete")
            # TODO: This could be handled better, currently stop_processing is not called automatically - that would be better!
            self.cycle_count = 0
            return (None, pyaudio.paComplete)

        start_pos = frame_count * self.cycle_count
        end_pos = start_pos + frame_count

        remaining_frames = len(self.sound_source) - end_pos
        audio_data = self.sound_source[start_pos:end_pos]

        if remaining_frames < frame_count:
            pad_length = frame_count - remaining_frames
            audio_data = np.pad(audio_data, (0, pad_length))

        audio_data = np.float32(audio_data)
        self.cycle_count += 1

        return (np.float32(audio_data).tobytes(), pyaudio.paContinue)


if __name__ == "__main__":
    import utils_functions as UF
    import time

    sample_rate = 44100
    sine_wave = UF.generateSineSignal(440.0, 3.0, sample_rate)

    sound_player = NumpySoundPlayer()
    sound_player.start_processing_non_blocking(sine_wave, sample_rate)

    while sound_player.processing():
        time.sleep(0.1)

    sound_player.stop_processing()