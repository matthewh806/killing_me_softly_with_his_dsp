import numpy as np
import matplotlib.pyplot as plt
import matplotlib.animation as animation
from matplotlib.lines import Line2D
from matplotlib.offsetbox import AnchoredOffsetbox, TextArea, VPacker
import os, sys
sys.path.append(os.path.join(os.path.dirname(os.path.realpath(__file__)), '../'))
import utils_functions as UF

'''
Read static sine signal over time into a multitap circular buffer 
and read from this buffer into an output buffers using different delay times
'''

if __name__  == "__main__":

    sample_rate = 1000
    signal = UF.generateSineSignal(10, 0.4, sample_rate)

    pos_signal = 0 # The current sample position we're reading from in the signal itself (into circular buffer)
    len_signal = len(signal)

    circular_buffer = UF.MultitapCircularBuffer(50, 5, 8.25, 14.7, 45.5)

    output_buffer = np.zeros(len(signal))
    pos_output = 0 # The current sample position we're writing to in the output signal (from the circular buffer)

    circ_xdata = np.arange(0, circular_buffer.size)
    circ_ydata = np.zeros(circular_buffer.size)

    output1_xdata = np.arange(0, len(output_buffer))
    output1_ydata = np.zeros(len(output_buffer))
    output2_xdata = np.arange(0, len(output_buffer))
    output2_ydata = np.zeros(len(output_buffer))
    output3_xdata = np.arange(0, len(output_buffer))
    output3_ydata = np.zeros(len(output_buffer))
    output4_xdata = np.arange(0, len(output_buffer))
    output4_ydata = np.zeros(len(output_buffer))

    output_mix_xdata = np.arange(0, len(output_buffer))
    output_mix_ydata = np.zeros(len(output_buffer))

    fig = plt.figure(figsize=(10, 10))
    fig.tight_layout()
    ax1 = plt.subplot2grid(shape=(3, 3), loc=(0, 0), colspan=3)
    line1, = ax1.plot(signal)
    signal_pos_line = Line2D([], [], color='red', linewidth=2)
    ax1.add_line(signal_pos_line)
    ax1.set_xlabel("Sample")
    ax1.set_ylabel("Amp")
    ax1.set_title("Signal")
    ax1.grid()

    ax2 = plt.subplot2grid(shape=(3, 3), loc=(1, 0), colspan=1)
    line2, = ax2.plot(circ_xdata, circ_ydata)
    write_pos_line = Line2D([], [], color='red', linewidth=2)
    read1_pos_line = Line2D([], [], color='blue', linewidth=2)
    read2_pos_line = Line2D([], [], color='purple', linewidth=2)
    read3_pos_line = Line2D([], [], color='magenta', linewidth=2)
    read4_pos_line = Line2D([], [], color='violet', linewidth=2)
    write_text = ax2.text(circular_buffer.head, 0, "Write", color="white", rotation=90, bbox=dict(facecolor='red'))
    read1_text = ax2.text(circular_buffer.head, 0, "Read1", color="white", rotation=90, bbox=dict(facecolor='blue'))
    read2_text = ax2.text(circular_buffer.head, 0, "Read2", color="white", rotation=90, bbox=dict(facecolor='purple'))
    read3_text = ax2.text(circular_buffer.head, 0, "Read3", color="white", rotation=90, bbox=dict(facecolor='magenta'))
    read4_text = ax2.text(circular_buffer.head, 0, "Read4", color="white", rotation=90, bbox=dict(facecolor='violet'))
    ax2.add_line(write_pos_line)
    ax2.add_line(read1_pos_line)
    ax2.add_line(read2_pos_line)
    ax2.add_line(read3_pos_line)
    ax2.add_line(read4_pos_line)
    ax2.set_xlim(0, circular_buffer.size)
    ax2.set_xlabel("Sample")
    ax2.set_ylabel("Amp")
    ax2.set_title("Circular Buffer (size={})".format(circular_buffer.size))
    ax2.grid()

    read_speed_text = TextArea("Read Speed: ")
    write_sample_text = TextArea("Write Sample:")
    read_sample_text = TextArea("Read Sample:")

    box = VPacker(children=[read_speed_text, write_sample_text, read_sample_text])
    anchored_box = AnchoredOffsetbox(loc=3, child=box, bbox_to_anchor=(1.1, 0.75),
                                     bbox_transform=ax2.transAxes,
                                     borderpad=0.)
    ax2.add_artist(anchored_box)

    ax3 = plt.subplot2grid(shape=(3, 3), loc=(2, 0), colspan=3)
    line3, = ax3.plot(output1_ydata, color="blue", label="tap1")
    line4, = ax3.plot(output2_ydata, color="purple", label="tap2")
    line5, = ax3.plot(output3_ydata, color="magenta", label="tap3")
    line6, = ax3.plot(output4_ydata, color="violet", label="tap4")
    line7, = ax3.plot(output_mix_ydata, color="black", label="summed")

    ax3.set_xlabel("Sample")
    ax3.set_ylabel("Amp")
    ax3.set_title("Output Signal")
    ax3.legend()
    ax3.grid()

    def init():
        ax1.set_ylim(-1, 1)
        ax2.set_ylim(-1, 1)
        ax3.set_ylim(-1, 1)

        read1_pos = circular_buffer.get_tap_position(0)
        read2_pos = circular_buffer.get_tap_position(1)
        read3_pos = circular_buffer.get_tap_position(2)
        read4_pos = circular_buffer.get_tap_position(3)

        signal_pos_line.set_data([pos_signal, pos_signal], [-1,1])
        write_pos_line.set_data([circular_buffer.head, circular_buffer.head], [-1,1])
        read1_pos_line.set_data([read1_pos, read1_pos], [-1,1])
        read2_pos_line.set_data([read2_pos, read2_pos], [-1,1])
        read3_pos_line.set_data([read3_pos, read3_pos], [-1,1])
        read4_pos_line.set_data([read4_pos, read4_pos], [-1,1])

        circ_ydata = np.zeros(circular_buffer.size)
        line2.set_data(circ_xdata, circ_ydata)
        write_text.set_x(circular_buffer.head)
        read1_text.set_x(read1_pos)
        read2_text.set_x(read2_pos)
        read3_text.set_x(read3_pos)
        read4_text.set_x(read4_pos)

        read_speed_text.set_text("Read Speed: {}".format(1))
        write_sample_text.set_text("Write Sample: {}".format(circular_buffer.head))
        read_sample_text.set_text("Read Sample: {}".format(read1_pos))

        output_ydata = np.zeros(len(output_buffer))
        line3.set_data(output1_xdata, output_ydata)
        line4.set_data(output1_xdata, output_ydata)
        line5.set_data(output1_xdata, output_ydata)
        line6.set_data(output1_xdata, output_ydata)
        line7.set_data(output1_xdata, output_ydata)

        return signal_pos_line, line2, line3, line4, line5, line6, write_pos_line, read1_pos_line, write_text, read1_text

    def run(data):
        global pos_signal 
        global pos_output

        # Get the current signal value
        pos_signal = (pos_signal + 1) % len_signal
        signal_value = signal[pos_signal]

        # Get the read head positionsss
        read1_pos = circular_buffer.get_tap_position(0)
        read2_pos = circular_buffer.get_tap_position(1)
        read3_pos = circular_buffer.get_tap_position(2)
        read4_pos = circular_buffer.get_tap_position(3)

        # Write into the value circular buffer 
        circular_buffer.write(signal_value)
    
        output1_value = circular_buffer.read(0)
        output2_value = circular_buffer.read(1)
        output3_value = circular_buffer.read(2)
        output4_value = circular_buffer.read(3)
        pos_output = (pos_output + 1) % len(output1_xdata)

        # Calculate the mixed tap output (just sum the sines for now)
        # bring into the range [-1,1]?
        output_mix_value = output1_value + output2_value + output3_value + output4_value
        output_mix_value = UF.convert_value_from_to_range(output_mix_value, -4, 4, -1, 1)

        # Update the plotting structures
        signal_pos_line.set_data([pos_signal, pos_signal], [-1, 1])
        write_pos_line.set_data([circular_buffer.head, circular_buffer.head], [-1,1])
        read1_pos_line.set_data([read1_pos, read1_pos], [-1,1])
        read2_pos_line.set_data([read2_pos, read2_pos], [-1,1])
        read3_pos_line.set_data([read3_pos, read3_pos], [-1,1])
        read4_pos_line.set_data([read4_pos, read4_pos], [-1,1])
        write_text.set_x(circular_buffer.head-1)
        read1_text.set_x(read1_pos-1)
        read2_text.set_x(read2_pos-1)
        read3_text.set_x(read3_pos-1)
        read4_text.set_x(read4_pos-1)
        circ_ydata[circular_buffer.head] = circular_buffer.peek()
        output1_ydata[pos_output] = output1_value
        output2_ydata[pos_output] = output2_value
        output3_ydata[pos_output] = output3_value
        output4_ydata[pos_output] = output4_value
        output_mix_ydata[pos_output] = output_mix_value

        read_speed_text.set_text("Read Speed: {}".format(1))
        write_sample_text.set_text("Write Sample: {}".format(circular_buffer.head))
        read_sample_text.set_text("Read Sample: {}".format(read1_pos))

        line2.set_ydata(circ_ydata)
        line3.set_ydata(output1_ydata)
        line4.set_ydata(output2_ydata)
        line5.set_ydata(output3_ydata)
        line6.set_ydata(output4_ydata)
        line7.set_ydata(output_mix_ydata)

        return signal_pos_line, line2, line3, line4, line5, line6, line7, write_pos_line, read1_pos_line, write_text, read1_text

    ani = animation.FuncAnimation(fig, run, blit=False, interval=100, 
                                  repeat=False, init_func=init)

    plt.show()
