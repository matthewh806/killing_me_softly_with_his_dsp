from soxmosh import AnimationUtils
import json
import os

'''
This is a class designed as a way to generate json structures for input to the command line application to be used
when generating gifs

I think this is necessary to avoid the tedium of manually creating each frame of a data and manually modulating parameters by hand
when they could be described by simple functions like sine waves. This isn't really necessary when calling the classes directly from
other python files (like in the examples) since we can just generate the data in place while calling the SoxMosh methods.

It just requires that you provide a json like array containing the effects to be stored and then handles the 
overall json creation.
'''

DEFAULT_JSON_DIRECTORY = os.path.join(
    os.path.dirname(os.path.realpath(__file__)), "input_json")


def save_test_json(effects, output_path=None):
    if output_path is None:
        output_path = os.path.join(
            DEFAULT_JSON_DIRECTORY, "generated_effects.json")

    data = {}
    data['effects'] = effects

    with open(output_path, 'w') as json_file:
        json.dump(data, json_file)


if __name__ == "__main__":
    effects = [
        [
            {
                "echos": {"gain_in": 0.2, "gain_out": 0.88, "delays": [20 * AnimationUtils.generate_positive_sine_wave(i/20) + 0.1]}
            }] for i in range(0, 20)]
    save_test_json(effects)
