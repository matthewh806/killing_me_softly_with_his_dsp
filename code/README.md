## Code

This is where all of the source code lives

The main code is written in C++ and built on top of juce.

## Juce Applications
### Compiling

### Prerequisites
[CMake](https://cmake.org/) is used in order to compile all of the C++ applications.

Install with:

```
brew install cmake
```

Dependencies will vary between different targets within the subproject. [JUCE](https://juce.com/) is likely to be the foundation of each application so it is added as a submodule.

[Rubberband](https://breakfastquay.com/rubberband/) is a dependency of the timestretch applications is similarly included as a dependency and
compiled / linked as part of the cmake build system.

There are two prerequisites necessary `libsamplerate` which is used by the Rubberband library & `aubio` which is used for marker detection
in the Breakbeat maker (I would like to include this in the CMake build stage but haven't figured out how to make it compile)

```
brew install libsamplerate aubio
```

CMake build system

#### MacOS
```
cd code
mkdir build; cd build
cmake .. -GXcode
```

This will generate a single monolithic xcode project with individual targets for each executable.

#### Linux
This has only been tested on Ubuntu 21.04 64-bit

In order to compile install the [JUCE dependencies](https://github.com/juce-framework/JUCE/blob/master/docs/Linux%20Dependencies.md).
For rubberband to compile its also necessary to have `libsamplerate` installed

```
sudo apt-get install libsamplerate-dev
```
These can both be removed as dependencies in `dependencies/rubberband/CMakeLists.txt` but I haven't tried this out

Then to compile:
```
cd code
mkdir build; cd build
cmake .. -DCMAKE_BUILD_TYPE=Debug
```

### Contents
#### Applications
##### Breakbeat Machine
This was a project I made a while ago for chopping up an audio sample into fractional slices and play them at random. I imported this as is and need to refactor it. Though it does work as expected

###### Possible improvements
- [ ] Slice by transient - investigate a good method for this. Is a simple amp threshold sufficient?
   - [ ] https://github.com/adamstark/Gist or https://github.com/aubio/aubio to handle this perhaps
- [ ] Slice manually by clicking on the waveform
- [ ] Zoom on waveform?
- [ ] Expand / Shrink signal to fit into a certain period
  - [ ] Resampling temporal shifting (involves a pitch shift)
  - [ ] Rubberband spectral shifting
- [ ] Apply envelopes (e.g. fade in / out)
- [ ] Draw individual slices on the waveform

##### Delay
This is an implementation of a very simple delay algorithm from the Pirkle book. It works, but would benefit from the inclusion of further more interesting algorithms

##### Real time Timestretch
This is an attempt at incorporating timestretching into a real time audio application. I didn't really think the logic of doing this through before attempting to implement it and quickly realised two things:
  - Compressing time requires time travel to be invented. The algorithm is chomping through data faster than it really becomes available at the rate its provided from the soundcard. Is there a clever way of getting around this? What if we ask for more data than we need from the soundcard and then internally manage how we deliver it to the output? Will we still eventually outpace the input?
  - Expanding time requires that more and more data be stored. I think the result is that the audio buffer size becomes unbounded. I thought using a circular buffer would work here - but in any case it can't be a fixed size one anyway because we're **always** going to need to keep storing more and more samples as the output falls further and further behind the inputs position

Interesting...

##### Offline Timestretch
This is quite simple to implement. All we need to do is load in an audio file and apply the timestretch transformation & save it to a new file.

###### Possible improvements
- [ ] Currently there is only support for stretching + saving wav files, it might be nicer to allow for other formats
- [ ] I'm just using the time stretch factor + pitch shift amt sliders as parameters exposed to the user, but RubberBand has many more to play with.
- [ ] Put the pitch shift in more meaningful units perhaps? It's just a multiplication factor for now.

##### Audio Decay
This is a simple plugin with just a couple of knobs. The theory is a bit more complex though.
At the moment there are two separate effects going on: **Decimation** & **Bitcrushing**

In general there are aliasing effects in both of these proceedures which require some
slightly involved filtering to remove. Since I want some artefacts to remain & I don't fully understand the filtering theory I'm ignoring all that for now.  

**Bitcrushing**

This is the process of requantising a discrete waveform. I.e. changing the number of bits which is used to encode the analog waveform - changing the number of quantisation levels used to represent a signal between the `[-1, 1]` bounds. A 16-bit signal has `2^16 = 65,536` quantisation levels.

In order to achieve this effect I'm applying this simple equation:

```
quantisation_level: qL = 2 / (2^(bit_depth) - 1)

for input signal x(n), generate new output y(n):
y(n) = qL * (int ( x(n) / qL ))
```

[source: designing audio effect plugins in c++, pirkle, second ed]

**Decimation**

This is the more complex of the two approaches. My ultimate aim is just to try to degrade the quality of the sound further. I read from quite a few sources of varying degrees of complexity. I think for a first attempt [this](https://forum.juce.com/t/seeking-help-with-free-ratio-downsampler-plugin-dsp/18344/7)
 approach is that I will try:

> What you really want I think is downsampling then upsampling without removing the resampling artefacts / aliasing.

>To downsample with an integer ratio N, you can return one sample every (N-1) samples. This way you get your downsampled “aliased” signal, because for it to be less aliased, you have to filter it before the operation with a cutoff frequency at your sampling rate / (N-1) / 2 and a very stiff curve. Since you have not done it, it’s aliased.

> Then, you don’t want your signal to be at a lower sample rate, but at the current one. So, you need to upsample it after that operation. To do so, you can add N-1 zeroes between your samples at the lower sample rate. You obtain your signal upsampled, back at the original sample rate, with “imaging artefacts”.

>There, you need to filter the signal to remove some of the upsampling artefacts. But you can also return directly these samples with zeroes. Or, if you want something a little cleaner, but still harsh, you can do linear interpolation, or “drop sample” interpolation for the upsampling part. Linear interpolation is just going linearly from sample k to sample k+1 in the intermediate values, and “drop sample” is repeating N-1 times the sample k.

##### Pulsar

This is an import of an [older repo](https://github.com/matthewh806/Pulsar). I wanted to salvage this and continue working on it.
Put plainly its a rip off of the ["Tombola Sequencer"](https://teenage.engineering/guides/op-1/sequencers) in the Teenage Engineering OP-1.

The tombola sequencer creates random sequences based on colliding balls in a spinning hexagon. I always thought this was quite a cute idea and so wanted to create my own VST version since (at the time) I didn't come across many implementations.

I've added a few more shapes, but otherwise not really gone beyond the TE approach.

##### Granular

This is an implementation of a simple granular synthesiser based on a [paper](https://github.com/matthewh806/killing_me_softly_with_his_dsp/blob/develop/research/granular/BencinaAudioAnecdotes310801.pdf) by Ross Bencina.
This was mainly done as a learning exercise. I'd never tried to implement an architecture suggested from a paper - so wanted to see how successful I could be at translating block diagrams & pseudo code into a working synth application.

It works... but how do I know if its really working correctly? There are some nasty sound artifacts, pops etc due to (I think) very short envelope times.

##### Matt Verb

This is a VST3 implementation of the [freeverb](https://github.com/sinshu/freeverb) library built using the Steinberg VST3SDK (i.e. no Juce!)
The plugin example provided in the original freeverb repo is build using the VST SDK, so I wanted to try updating it for VST3. I've also added the ability to
add in predelay to the reverbed signal. 

##### GPTVerb 

This is another implementation of the freeverb VST3 plugin, however this time the code has been generated based on my conversations with [Chat-GPT](chat.openai.com) (Dec 15 2022 Edition) starting with an initial prompt: "can you make me a reverb VST based on the freeverb open source library?".  Many, many, many prompts & iterations later and with a significant amount of code correction on my part the plugin compiled and reverbed any input sound. 

## Experiments

This directory is just for quick and useful python scripts for testing out dsp ideas or looking at how certain transformations affect a signal graphically.
