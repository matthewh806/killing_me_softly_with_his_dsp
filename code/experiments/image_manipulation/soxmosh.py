import sox
import os
from PIL import Image
import pathlib
import tempfile

'''
A script for data moshing images using the pysox library

Note: as png, bmp, tif(f) are ignored the directories are "empty" and 
hence not checked into version control. So you'll have to make the directories
as appropriate and modify the path globals defined below before running

See the sox documentation https://pysox.readthedocs.io/en/latest/api.html for a list 
of all of the available effects

TODO:
    - funtionality to manipulate specific regions of the image
    - UI (tkinter) for viewing the image directly and selecting regions to affect
    - Refactor so that the loading of image data isnt handled by the bend function
'''

CURRENT_DIRECTORY = os.path.dirname(os.path.realpath(__file__))
TEMP_DIRECTORY = os.path.join(CURRENT_DIRECTORY, "temp")


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


class SoxMosh:
    '''
    Databend an input image using sox transformers
    The best approach is to use a bmp image, if another format is
    supplied it will be converted to bmp before applying the transformations

    To perform the actual effect operations call databend_image(...) with a list
    of effects in a dictionary structure to apply (see example json in input_json directory)
    '''

    def __init__(self, input_path, output_path):
        if pathlib.Path(input_path).suffix != ".bmp":
            converted_path = os.path.splitext(input_path)[0] + ".bmp"
            Image.open(input_path).save(converted_path)
            input_path = converted_path

        self.input_path = input_path
        self.output_path = output_path
        self.tfm = sox.Transformer()

    def databend_image(self, effects_list=None):
        '''
        effects_list is a python list with the expected format:

        [
            {'effect1_name': {'param1': value, 'param2': value, ... }}, 
            {'effect2_name': {'param1': value, 'param2': value, ...}}, 
            ...
            {'effectn_name: {...}}
        ]

        where the key in each element corresponds to a sox effect (see Transformer documentation) 
        and the dictionary or params / values will be used as kwargs to customise the effect. 
        If any of the named parameters are omitted the defaults will be used

        e.g. to create a default echo effect use:
        {"echos": {}}

        or parameterised with:

        {"echos": {"gain_in": 0.2, "gain_out": 0.88, "delays":[60], "decays":[0.5]}}
        '''

        self.tfm.set_input_format(file_type="raw", encoding="u-law",
                                  channels=1, rate=48000)
        self.tfm.set_output_format(
            file_type="raw", encoding="u-law", channels=1, rate=48000)

        if effects_list:
            for effect in effects_list:
                (name, params) = list(effect.items())[0]
                self._get_transform_method(name)(**params)

        input_file_name = pathlib.Path(self.input_path).stem

        with tempfile.TemporaryDirectory() as temp_directory:
            header_path = os.path.join(
                temp_directory, input_file_name + "_header.bmp")
            body_path = os.path.join(
                temp_directory, input_file_name + "_body.bmp")
            temp_body_path = os.path.join(
                temp_directory, input_file_name + "temp_body.bmp")
            body_length = separate_header(
                self.input_path, header_path, body_path)

            self.tfm.build_file(body_path, temp_body_path)

            resize(temp_body_path, body_length)
            attach_header(header_path, temp_body_path, self.output_path)

        self.tfm.clear_effects()

    def _get_transform_method(self, method_string):
        '''
        Returns a python attribute from the sox transform class
        The expected return value is a function called "method_string" 

        This allows us to not have to specify the specific transforms explicitly in the code
        and instead rely on external data supplied as for e.g. json

        E.g. method_string = "echo" would return self.tfm.echo from the function 
        can be called with parameters

        self.tfm.echo({"gain_in":0.2, "gain_out":0.88, ...})
        '''
        assert hasattr(self.tfm, method_string)
        return getattr(self.tfm, method_string)


if __name__ == "__main__":
    # This is just to demonstrate how to use the class directly

    input_path = os.path.join(
        CURRENT_DIRECTORY, "input_images/perfect_blue_city.bmp")
    output_path = os.path.join(
        CURRENT_DIRECTORY, "output_images/perfect_blue_city_moshed.bmp")
    sox_mosh = SoxMosh(input_path, output_path)

    effects_list = [
        {"echos": {"gain_in": 0.2, "gain_out": 0.88, "delays": [60], "decays": [0.5]}}]
    sox_mosh.databend_image(effects_list)
