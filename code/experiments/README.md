# Experiments

The purpose of this directory is to collect quick and dirty scripts for testing out
and plotting certain ideas.

Python seems quite ideal for this and I find numpy + matplotlib a good combination
for quickly getting results on the screen.

The code here is unlikely to be very well documented and is not intended really
to be used for anything other than understanding how certain transformations affect signals.

The dependencies will likely vary from script to script, but as a baseline we can assume
that numpy, scipy & matplotlib should all be installed.

## TODO

### Convenience functions / classes ###

There are quite a few methods which are general and useful enough to be
abstracted from the scripts themselves and should be easily importable.

- [ ] basic signal generation (sine, sqr, tri, rnd etc)
- [ ] basic signal plotting with matplotlib
- [ ] basic audio playback? (which lib to use?)
