# General code TODO's

- [x] **FIX THE WARNINGS YOU LAZY BASTARD**

- [ ] Once thats done (lol never) implement a proper solution in cmake for defining what warnings want to see

- [x] Rubber band is a requirement for successful compilation - currently it requires that rubberband is found on the users machine in standard paths, which is totally pointless as I'm including it directly as a depdendency. -> so figure out how to compile this and provide it as a library in cmake

- [ ] Refactor Breakbeat maker

- [ ] Fix stretch projects
  - [x] Offline
  - [ ] Real Time (can it be done?)

- [ ] Don't start new things before you've finished another...

- [ ] Customise look and feel further.

- [x] AudioDeviceManager should save settings somewhere (globally or per app?) because its very tedious to have to set them each time.

## Ideas for applications

### Reverb
Reverb is great, everyone loves it, I am no exception. But I don't really understand how it works - so have a go at implementing one to understanding

### Distortion
Because why not?

### "Playground"
Some kind of modular playground. As I'm creating all effects as `juce::AudioProcessor` subclasses these can easily be packaged into standalone applications, vsts etc

Which means I could also make my own small application for linking these together in some kind of modular way.

Is there any point in doing this? Probably not, but it could be quite a fun exercise and although JUCE offers similar with the "PluginTester" I really dislike that for some reason (no reason).

It would be nice to also provide meters, scopes, spectral representations all implemented as processors. Kind of like a shit Reaktor.

#### Prerequisites
- [ ] I've implemented the UI for the processors in the application classes directly, I think (but I'm not certain) this should be handled by subclassing: https://docs.juce.com/master/classAudioProcessorEditor.html
- [ ] No point doing this until I've at least made more functional effects

For future reference:
- https://docs.juce.com/master/classAudioProcessorGraph.html I think this class can really tie the room together
- https://docs.juce.com/master/tutorial_audio_processor_graph.html juce already has a tutorial for creating this kind of application

## CI
This is about as basic as can be for now...

- [ ] Add a windows target
- [ ] Add a debian target
- [ ] Add tests (hmm...)
- [ ] Deployment script (or just upload binaries to some place)
  - [ ] macos - dmg? zip?
  - [ ] linux - zip? tar?
  - [ ] windows
