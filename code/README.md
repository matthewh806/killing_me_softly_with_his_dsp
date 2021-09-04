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

At the moment [Rubberband](https://breakfastquay.com/rubberband/) which is a dependency of the timestretch applications is required to be installed on the users machine as its not properly compiled + integrated as part of the CMake build system. This is unfortunate and will be addressed in future. But for now:

```
brew install rubberband
```

should do the trick.

Pull repository and get submodules
```
  git clone git@github.com:matthewh806/killing_me_softly_with_his_dsp.git
  git submodule update --init --recursive
```

CMake build system

```
cd code
mkdir build; cd build
cmake .. -GXcode
```

This will generate a single monolithic xcode project with individual targets for each executable.
So far this has only been tested on macOS

### Contents
#### Applications
##### Breakbeat Machine
This was a project I made a while ago for chopping up an audio sample into fractional slices and play them at random. I imported this as is and need to refactor it. Though it does work as expected

#### Delay
This is an implementation of a very simple delay algorithm from the Pirkle book. It works, but would benefit from the inclusion of further more interesting algorithms

#### Real time Timestretch
This is an attempt at incorporating timestretching into a real time audio application. I didn't really think the logic of doing this through before attempting to implement it and quickly realised two things:
  - Compressing time requires time travel to be invented. The algorithm is chomping through data faster than it really becomes available at the rate its provided from the soundcard. Is there a clever way of getting around this? What if we ask for more data than we need from the soundcard and then internally manage how we deliver it to the output? Will we still eventually outpace the input?
  - Expanding time requires that more and more data be stored. I think the result is that the audio buffer size becomes unbounded. I thought using a circular buffer would work here - but in any case it can't be a fixed size one anyway because we're **always** going to need to keep storing more and more samples as the output falls further and further behind the inputs position

Interesting...

#### Offline Timestretch
This is quite simple to implement. All we need to do is load in an audio file and apply the timestretch transformation & save it to a new file.

At some point this seemed to work, now I'm not so sure.

## Experiments

This directory is just for quick and useful python scripts for testing out dsp ideas or looking at how certain transformations affect a signal graphically.
