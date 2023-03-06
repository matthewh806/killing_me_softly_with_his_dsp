## Ideas

### Distortion
Because why not?

### Buffer overrun sampler

In this video the guy discusses buffer over / underruns related to circular buffers: https://youtu.be/uX-FVtQT0PQ
Could be interesting to sample sounds this way? For glitchy synths

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
