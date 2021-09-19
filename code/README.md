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

There is one prerequesite necessary for the Rubberband library `libsamplerate`

```
brew install libsamplerate
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

#### Experiments

This directory is just for quick and useful python scripts for testing out dsp ideas or looking at how certain transformations affect a signal graphically.
