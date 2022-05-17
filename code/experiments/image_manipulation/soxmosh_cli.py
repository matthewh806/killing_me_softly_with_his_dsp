from soxmosh import SoxMosh
import argparse
import json

'''
This is a script which defines the CLI interface for the soxmosh class

It makes use of the json library in order to supply the effects data externally
Requires the user to supply an input image path, output image path and an effects data path

Its possible to use the class directly bypassing the json approach, look at
the __main__ block in soxmosh.py for an example
'''


def main(input_image_path, output_image_path, effects_data_path):
    with open(effects_data_path, "r") as json_file:
        data = json.load(json_file)

    sox_mosh = SoxMosh(input_path=input_image_path,
                       output_path=output_image_path)
    sox_mosh.databend_image(data['effects'])


if __name__ == "__main__":
    parser = argparse.ArgumentParser()
    parser.add_argument(
        "input_image", help="The path to the input image to be datamoshed")
    parser.add_argument(
        "output_image", help="The path to where the output image will be saved")
    parser.add_argument("effects", help="The path to the effects json file")
    args = parser.parse_args()

    main(input_image_path=args.input_image,
         output_image_path=args.output_image, effects_data_path=args.effects)
