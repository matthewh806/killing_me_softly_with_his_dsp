from soxmosh import SoxMosh
import argparse
import json
import logging

'''
This is a script which defines the CLI interface for the soxmosh class

It makes use of the json library in order to supply the effects data externally
Requires the user to supply an input image path, output image path and an effects data path

The sample rate to use for the image transformations can optionally be specified with 
the --sample-rate parameter. If this is not provided the default of 44100 Hz will be used

Its possible to use the class directly bypassing the json approach, look at
the __main__ block in soxmosh.py for an example
'''


def main(input_image_path, output_image_path, effects_data_path, sample_rate):
    with open(effects_data_path, "r") as json_file:
        data = json.load(json_file)

    sox_mosh = SoxMosh(input_path=input_image_path,
                       output_path=output_image_path, sample_rate=sample_rate)
    sox_mosh.databend_image(data['effects'])


if __name__ == "__main__":
    parser = argparse.ArgumentParser()
    parser.add_argument(
        "input_image", help="The path to the input image to be datamoshed")
    parser.add_argument(
        "output_image", help="The path to where the output image will be saved")
    parser.add_argument(
        "--sample-rate", default=44100, type=int, help="The sample rate to use for the effects"
    )
    parser.add_argument("effects", help="The path to the effects json file")
    parser.add_argument("--log", default="INFO", help="Set the logging level to be used for stdout")
    args = parser.parse_args()

    numeric_level = getattr(logging, args.log.upper(), None)
    if not isinstance(numeric_level, int):
        raise ValueError('Invalid log level: %s' % args.log)
    logging.basicConfig(level=numeric_level)

    main(input_image_path=args.input_image,
         output_image_path=args.output_image, effects_data_path=args.effects, sample_rate=args.sample_rate)
