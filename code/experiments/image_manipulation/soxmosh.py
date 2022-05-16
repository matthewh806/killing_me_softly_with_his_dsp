import sox
import os
from PIL import Image
import pathlib
import json

'''
A script for data moshing images using the pysox library

Note: as png, bmp, tif(f) are ignored the directories are "empty" and 
hence not checked into version control. So you'll have to make the directories
as appropriate and modify the path globals defined below before running

See the sox documentation https://pysox.readthedocs.io/en/latest/api.html for a list 
of all of the available effects

TODO:
    - Add command line interface to pass input path, output path, dict (/json) of effects
    - Add json parser
    - funtionality to manipulate specific regions of the image
    - UI (tkinter) for viewing the image directly and selecting regions to affect
    - Refactor into classes ?
'''

CURRENT_DIRECTORY = os.path.dirname(os.path.realpath(__file__))
INPUT_DIRECTORY = os.path.join(CURRENT_DIRECTORY, "input_images")
OUTPUT_DIRECTORY = os.path.join(CURRENT_DIRECTORY, "output_images")
TEMP_DIRECTORY = os.path.join(CURRENT_DIRECTORY, "temp")
INPUT_JSON_DIRECTORY = os.path.join(CURRENT_DIRECTORY, "input_json")


def separate_header(input_path, header_path, body_path):
    '''
    Separates the header and the body from a bmp file. 
    The address of the start of the body is indicated by the data at index 0xa

    Stores them as two separate temporary bmp files in the temp directory
    '''

    with open(input_path, "rb") as file:
        bmp = file.read()
        end_of_header_address = bmp[0xA]
        head, body = bmp[:end_of_header_address], bmp[end_of_header_address:]

    with open(header_path, 'wb') as header_file:
        header_file.write(head)

    with open(body_path, 'wb') as body_file:
        body_file.write(body)

    return len(body)


def attach_header(header_path, body_path, output_path):
    '''
    Rettach the header and the body into one single bmp image
    '''

    with open(header_path, 'rb') as header_file, open(body_path, 'rb') as body_path:
        header = header_file.read()
        body = body_path.read()

    with open(output_path, 'wb') as output_file:
        output_file.write(header + body)


def resize(body_path, length):
    '''
    Resize the body of the image image so that its the same size as expected by the header 
    Note: This is the same length as the original input files body

    The method will either truncate if its larger or add empty dummy bytes if smaller
    '''

    with open(body_path, "rb+") as body_file:
        body = body_file.read()
        body = body[:length]

        body = body + bytes(length - len(body))
        body_file.seek(0)
        body_file.write(body)


def get_transform_method(tfm, method_string):
    assert hasattr(tfm, method_string)
    return getattr(tfm, method_string)


def databend_image(input_path, effects_dict=None):
    '''
    Databend an input image using sox transformers
    The best approach is to use a bmp image, if another format is
    supplied it will be converted to bmp before applying the transformations

    effects_dict is a python dictionary with the expected format:
    {'effect1_name': {'param1': value, 'param2': value, ... }, 'effect2_name': {'param1': value, 'param2': value, ...}, ...}

    where the key corresponds to a sox effect (see Transformer documentation) and the dictionary or params / values will be used
    as kwargs to customise the effect. If any of the named parameters are omitted the defaults will be used

    e.g. to create a default echo effect use:
    {"echos": {}}

    or parameterised with:

    {"echos": {"gain_in": 0.2, "gain_out": 0.88, "delays":[60], "decays":[0.5]}}
    '''

    if pathlib.Path(input_path).suffix != ".bmp":
        converted_path = os.path.splitext(input_path)[0] + ".bmp"
        Image.open(input_path).save(converted_path)
        input_path = converted_path

    tfm = sox.Transformer()
    tfm.set_input_format(file_type="raw", encoding="u-law",
                         channels=1, rate=48000)
    tfm.set_output_format(
        file_type="raw", encoding="u-law", channels=1, rate=48000)

    if effects_dict:
        for effect, params in effects_dict.items():
            get_transform_method(tfm, effect)(**params)

    input_file_name = pathlib.Path(input_path).stem
    header_path = os.path.join(TEMP_DIRECTORY, input_file_name + "_header.bmp")
    body_path = os.path.join(TEMP_DIRECTORY, input_file_name + "_body.bmp")
    temp_body_path = os.path.join(
        TEMP_DIRECTORY, input_file_name + "temp_body.bmp")
    body_length = separate_header(input_path, header_path, body_path)

    output_path = os.path.join(
        OUTPUT_DIRECTORY, input_file_name + "_bended.bmp")
    tfm.build_file(body_path, temp_body_path)

    resize(temp_body_path, body_length)
    attach_header(header_path, temp_body_path, output_path)


if __name__ == "__main__":
    # convert format to bmp

    with open(os.path.join(INPUT_JSON_DIRECTORY, "example_effects.json"), "r") as json_file:
        data = json.load(json_file)

    databend_image(os.path.join(INPUT_DIRECTORY,
                   "perfect_blue_face.tiff"), data)
    databend_image(os.path.join(INPUT_DIRECTORY, "perfect_blue_city.tiff"), data)
