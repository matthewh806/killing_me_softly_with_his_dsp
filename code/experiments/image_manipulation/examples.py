from soxmosh import SoxMosh
import os
import logging

# These examples show direct use of using the SoxMosh class 
# to bend images. Bypassing the CLI.

logging.basicConfig(logging.INFO)

CURRENT_DIRECTORY = os.path.dirname(os.path.realpath(__file__))

INPUT_IMAGE = os.path.join(
    CURRENT_DIRECTORY, "input_images/perfect_blue_city.bmp")

# Basic example of a single effect being applied. 
sox_mosh = SoxMosh(INPUT_IMAGE)
sox_mosh.databend_image(os.path.join(CURRENT_DIRECTORY, "output_images/perfect_blue_city_echo.bmp"), 
                            [{"echos": {"gain_in": 0.2, "gain_out": 0.88, "delays": [60], "decays": [0.5]}}])

# Basic example of a single effect with a variable parameter to create a gif
sox_mosh.databend_to_gif(os.path.join(CURRENT_DIRECTORY, "output_images/perfect_blue_city_echo.gif"), 
                            [{"echos": {"gain_in": 0.2, "gain_out": 0.88, "delays": [10*i], "decays": [0.5]}} for i in range(1, 100)])