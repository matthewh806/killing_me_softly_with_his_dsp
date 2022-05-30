from soxmosh import SoxMosh, AnimationUtils
import os
import logging
import math

# These examples show direct use of using the SoxMosh class
# to bend images. Bypassing the CLI.

logging.basicConfig(level=logging.INFO)

CURRENT_DIRECTORY = os.path.dirname(os.path.realpath(__file__))

INPUT_IMAGE = os.path.join(
    CURRENT_DIRECTORY, "input_images/perfect_blue_face.bmp")

# Basic example of a single effect being applied.
sox_mosh = SoxMosh(INPUT_IMAGE)
sox_mosh.databend_image(os.path.join(CURRENT_DIRECTORY, "output_images/perfect_blue_face_echo.bmp"),
                        [{"echos": {"gain_in": 0.2, "gain_out": 0.88, "delays": [60], "decays": [0.5]}}])

# Basic example of a single effect with a variable parameter to create a gif
sox_mosh.databend_to_gif(os.path.join(CURRENT_DIRECTORY, "output_images/perfect_blue_face_echo_60ms.gif"),
                         [[{"echos": {"gain_in": 0.2, "gain_out": 0.88, "delays": [0.5*i], "decays": [0.5]}}] for i in range(1, 10)], 60)

# Basic example of manually specifying frames in order to create a gif
sox_mosh.databend_to_gif(os.path.join(CURRENT_DIRECTORY, "output_images/perfect_blue_manual.gif"),
                         [
    [
        {"echos": {"gain_in": 0.2, "gain_out": 0.88,
                   "delays": [2], "decays": [0.5]}},
        {"phaser": {"gain_in": 0.8, "delay": 1}}
    ],
    [
        {"echos": {"gain_in": 0.2, "gain_out": 0.88,
                   "delays": [4], "decays": [0.5]}}
    ],
    [
        {"echos": {"gain_in": 0.2, "gain_out": 0.88,
                   "delays": [6], "decays": [0.5]}},
        {"phaser": {"gain_in": 0.8, "delay": 3}}
    ],
    [
        {"echos": {"gain_in": 0.2, "gain_out": 0.88,
                   "delays": [8], "decays": [0.5]}}
    ],
    [
        {"echos": {"gain_in": 0.2, "gain_out": 0.88,
                   "delays": [8], "decays": [0.5]}},
        {"phaser": {"gain_in": 0.8, "delay": 5}}
    ],
    [
        {"echos": {"gain_in": 0.2, "gain_out": 0.88,
                   "delays": [8], "decays": [0.5]}},
    ]
]
)

# Basic example of applying a smoothly looping effect using a sine wave
sox_mosh.databend_to_gif(os.path.join(CURRENT_DIRECTORY, "output_images/perfect_blue_face_echo_smooth.gif"),
                         [[{"echos": {"gain_in": 0.2, "gain_out": 0.88, "delays": [20 * AnimationUtils.generate_positive_sine_wave(i/20) + 0.1]}}] for i in range(0, 20)], 120)

# An example of gradually reverbing an image
sox_mosh.databend_to_gif(os.path.join(CURRENT_DIRECTORY, "output_images/perfect_blue_face_reverb_smooth.gif"),
                         [[{"reverb": {"reverberance": 100 * AnimationUtils.generate_positive_sine_wave(i/20)}}] for i in range(0, 20)], 120)

sox_mosh.databend_to_gif(os.path.join(CURRENT_DIRECTORY, "output_images/perfect_blue_face_distortion_random.gif"),
                         [[{"overdrive": {"gain_db": AnimationUtils.generate_uniform_random(10, 100)}},
                           {"echos": {"gain_in": 0.2, "gain_out": 0.88, "delays": [30]}}] for _ in range(0, 20)], 120)
