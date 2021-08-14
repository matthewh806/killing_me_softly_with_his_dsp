# Killing Me Softly With His DSP

A collection of small audio experiments, test applications, prototypes, a research dumping ground etc etc etc

## Compiling

Everything is built by a single CMakeLists file at the moment.
Until it becomes manageable everything will be handled this way - generating a single monolithic xcode project with individual targets for each executable.

Dependencies will vary between different targets within the subproject. [JUCE](https://juce.com/) is likely to be the foundation of each application so it is added as a submodule.

Pull repository and get submodules
```
  git clone git@github.com:matthewh806/killing_me_softly_with_his_dsp.git
  git submodule update --init --recursive
```

CMake build system

```
mkdir build; cd build
cmake .. -GXcode
```

So far this has only been tested on macOS

## Contents
### Timestretch

Simple JUCE standalone application exploring time stretch approaches.
This uses the [RubberBand](https://breakfastquay.com/rubberband/) library in order to perform the stretch / compression.
