# SoxMosh

This is a python library for manipulating images by applying audio effects to them.

## Background

The idea was inspired by some initial approaches I tried [using Audacity](http://datamoshing.com/2016/06/15/how-to-glitch-images-using-audio-editing-software/), which while fun is
quite a tedious process and prone to errors. Since the approach is completely non-visual anyway
I figured that writing some simple code to handle the transformations wouldn't go against
the spirit of the endeavour. 

The project is based around [pysox](https://github.com/rabitt/pysox) which is itself a wrapper around the [SoX](http://sox.sourceforge.net/) tool. 

Note: I am far from the first to try this idea out. Two resources I used for inspiration & reference are:

- [sockbend](https://github.com/Roachbones/sockbend) - Roachbones
- [Databending images with a command line audio processing utlity](https://shailendra.me/blog/tutorial/databending-on-command-line-audio-sox/) - Shailendra Paliwal



## Installing

### Requirements

```
brew install sox
pip3 install sox
pip3 install pillow
```

Pillow is used for image conversion

### Project


```
git clone https://github.com/matthewh806/killing_me_softly_with_his_dsp
cd code/experiments/image_manipulation
```

Note: This is part of a monolithic dumping ground repository, so you're going to get a lot 
of (probably irrelevant) crap along with it.


## Running

There are two ways to run the code:

### Directly

This just involves importing the class in a python script and defining input image path, output image path and some effects to apply. 

For example:

```
from soxmosh import SoxMosh

if __name__ == "__main__":

    input_path = 'path/to/input.bmp'
    output_path = 'path/to/output.bmp'
    sox_mosh = SoxMosh(input_path, output_path)

    sox_mosh.databend_image([{"echos": {}}])
```

This is a very simple example which creates a SoxMosh object and calls the `databend_image`
method with a single echo transformation applied with all the default parameters

The result will be stored at output_path.

Note: Its not necessary to supply effects in this format. The argument to `databend_image` is optional. An alternative approach is to
use the pysox API methods directly after creatring the SoxMosh object - the pysox Transform object can be accessed via `sox_mosh.tfm` and hence effects can be manually added up like:

```
sox_mosh.tfm.echos(...)
sox_mosh.tfm.reverb(...)
...
sox_most.databend_image()
```

### Command Line Interface

`soxmosh_cli.py` is an alternative and simpler way to call the code. 

```
usage: soxmosh_cli.py [-h] [--sample-rate SAMPLE_RATE] [--gif [GIF]]
                      [--log LOG]
                      input_image output_image effects

positional arguments:
  input_image           The path to the input image to be datamoshed
  output_image          The path to where the output image will be saved
  effects               The path to the effects json file

optional arguments:
  -h, --help            show this help message and exit
  --sample-rate SAMPLE_RATE
                        The sample rate to use for the effects
  --gif [GIF]           Generate a gif made of frames of individual moshed
                        images. The value supplied corresponds to the
                        individual frame time in ms
  --log LOG             Set the logging level to be used for stdout
```

The `effects` parameter should be structured like:

```
{
   "effects":
   [
      {
         "overdrive" : {"gain_db":10, "colour":7.0}
      },
      {
         "phaser": {}
      },
      {
         "bandreject": {"frequency":300, "width_q": 0.7}
      },
      {
         "phaser": {"gain_in": 0.8, "delay": 5}
      }
   ]   
}
```

This will apply an overdrive, phaser, bandreject & another effect in that order. 
See `input_json` directory for more examples. 

The sample rate to use for the image transformations can optionally be specified with 
the `--sample-rate` parameter. If this is not provided the default of 44100 Hz will be used

Supply the parameter `--gif` to make an animated gif (note this requires a slightly different 
json structure - see examples). The optional parameter supplied with `--gif` specifies the
individual image frame times (in ms) for the animation

Note: That if you use the gif parameter the json formatted effects structure is slightly different

```
{
	"effects": [
		[{
			"echos": {
				"gain_in": 0.2,
				"gain_out": 0.88,
				"delays": [0.1]
			},
         {
            "phaser": {}
         },
         ...
		}],
		[{
			"echos": {
				"gain_in": 0.2,
				"gain_out": 0.88,
				"delays": [0.5894348370484647]
			}
		}],
		[{
			"echos": {
				"gain_in": 0.2,
				"gain_out": 0.88,
				"delays": [2.0098300562505265]
			}
		}],
      ...
   ]
}
```

This structure has an additional layer of nesting to allow for specifying transformations on a frame by frame basis. 
Each inner array will apply all the effects contained within it to the current frame. 
There will be as many frames of data in the output gif as there are elements in the `"effects": []` array

## Usage

See the [pysox Transformer documentation](https://pysox.readthedocs.io/en/latest/api.html#) for a full list of the effects which can be applied. All of the effect parameters can be expressed with "keyword": param inside the effect dictionary. Any which are omitted will simpy use the default values. 

The code works directly with bmp images - other formats may be provided, but they will be internally converted to bmp using the Pillow library

## Future work

- Specify an area of the image to apply an effect to - currently its applied to the whole image
- Add a GUI to view the transformations in real time
- Transform GIFs / Video
- Web interface
