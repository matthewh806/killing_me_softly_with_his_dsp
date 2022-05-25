import sox
import os
from PIL import Image
import pathlib
import tempfile
import logging

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

# The VALID_FORMATS list is generated  by calling sox -h and parsing the AUDIO FILE FORMATS
# section. As the library is intended specifically for audio it obviously doesn't include 
# image formats, so this is just a hack to allow it to process bitmap files.
sox.core.VALID_FORMATS.append("bmp")

class ImageHandler():
    '''
    Class for handling images ready to be transformed via pysox.

    This is basically just a convenience class and offers a nicer
    encapsualtion than having to set up all the temporary files
    elsewhere. 

    The init method performs the step of storing the header and the actual
    image data separately

    Note: Call resize and attach_header after processing to ensure your
    image is viewable again
    '''
    def __init__(self, input_path):
        '''
        Takes a path to the input image, this doesn't have to be in bmp 
        format since any other format will be converted to this.

        This constructor will also create the temporary directory needed
        to store intermediate bmp images (separating the header from the body to
        avoid data corruption) and call the method to actually do the separation

        Note: Once your image operations are complete it is necessary to call 
        attach_header to fix the image again ready for display
        '''

        self.input_path = input_path
        if pathlib.Path(self.input_path).suffix != ".bmp":
            converted_path = os.path.splitext(self.input_path)[0] + ".bmp"
            Image.open(self.input_path).save(converted_path)
            self.input_path = converted_path

        input_file_name = pathlib.Path(self.input_path).stem

        self.temp_directory = tempfile.TemporaryDirectory()
        self.header_path = os.path.join(
            self.temp_directory.name, input_file_name + "_header.bmp")
        self.body_path = os.path.join(
            self.temp_directory.name, input_file_name + "_body.bmp")
        self.temp_body_path = os.path.join(
            self.temp_directory.name, input_file_name + "temp_body.bmp")
        self.body_length = self.separate_header()

    def __del__(self):
        self.temp_directory.cleanup()

    def separate_header(self):
        '''
        Separates the header and the body from a bmp file. 
        The address of the start of the body is indicated by the data at index 0xa

        Stores them as two separate temporary bmp files in the temp directory
        '''

        with open(self.input_path, "rb") as file:
            bmp = file.read()
            end_of_header_address = bmp[0xA]
            head, body = bmp[:end_of_header_address], bmp[end_of_header_address:]

        with open(self.header_path, 'wb') as header_file:
            header_file.write(head)

        with open(self.body_path, 'wb') as body_file:
            body_file.write(body)

        self.length = len(body)
        return self.length


    def attach_header(self, output_path):
        '''
        Rettach the header and the body into one single bmp image stored at 'output_path'
        '''

        with open(self.header_path, 'rb') as header_file, open(self.temp_body_path, 'rb') as body_file:
            header = header_file.read()
            body = body_file.read()

        with open(output_path, 'wb') as output_file:
            output_file.write(header + body)


    def resize(self):
        '''
        Resize the body of the image image so that its the same size as expected by the header 
        Note: This is the same length as the original input files body

        The method will either truncate if its larger or add empty dummy bytes if smaller
        '''

        with open(self.temp_body_path, "rb+") as body_file:
            body = body_file.read()
            body = body[:self.length]

            body = body + bytes(self.length - len(body))
            logging.debug("Resizing image: Original size=%i, new size=%i", self.length, len(body))
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

    def __init__(self, input_path, output_path, sample_rate=48000):
        self.image_handler = ImageHandler(input_path)

        self.output_path = output_path
        self.sample_rate = sample_rate
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
                                  channels=1, rate=self.sample_rate)
        self.tfm.set_output_format(
            file_type="raw", encoding="u-law", channels=1, rate=self.sample_rate)

        if effects_list:
            for effect in effects_list:
                (name, params) = list(effect.items())[0]
                self._get_transform_method(name)(**params)

        
        
        self.tfm.build_file(self.image_handler.body_path, self.image_handler.temp_body_path)

        self.image_handler.resize()
        self.image_handler.attach_header(self.output_path)

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
