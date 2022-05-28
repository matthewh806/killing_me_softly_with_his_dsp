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

Supply the parameter --gif to make an animated gif (note this requires a slightly different 
json structure - see examples). The optional parameter supplied with --gif specifies the
individual image frame times (in ms) for the animation

Its possible to use the class directly bypassing the json approach, look at
examples.py for lots of use cases
'''


def run_cli(input_image_path, output_image_path, effects_data_path, sample_rate, gif_duration=None):
    with open(effects_data_path, "r") as json_file:
        data = json.load(json_file)

    sox_mosh = SoxMosh(input_path=input_image_path, sample_rate=sample_rate)

    if not gif_duration:
        sox_mosh.databend_image(output_image_path, data['effects'])
    else:
        sox_mosh.databend_to_gif(output_image_path, data['effects'], duration=gif_duration)


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument(
        "input_image", help="The path to the input image to be datamoshed")
    parser.add_argument(
        "output_image", help="The path to where the output image will be saved")
    parser.add_argument(
        "--sample-rate", default=44100, type=int, help="The sample rate to use for the effects"
    )
    parser.add_argument("effects", help="The path to the effects json file")
    parser.add_argument("--gif", type=int, const=30, nargs="?",
                        help="Generate a gif made of frames of individual moshed images. The value supplied corresponds to the individual frame time in ms")
    parser.add_argument("--log", default="INFO",
                        help="Set the logging level to be used for stdout")
    args = parser.parse_args()

    numeric_level = getattr(logging, args.log.upper(), None)
    if not isinstance(numeric_level, int):
        raise ValueError('Invalid log level: %s' % args.log)
    logging.basicConfig(level=numeric_level)

    run_cli(input_image_path=args.input_image,
            output_image_path=args.output_image, effects_data_path=args.effects, sample_rate=args.sample_rate, gif_duration=args.gif)


if __name__ == "__main__":
    main()
