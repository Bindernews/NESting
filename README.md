# NESting
A re-creation of the NES VST by Matt Montag.

Back in 2011 Matt Montag released [NES VST][nes-vst], which attempts to emulate the NES soundchip to help musicians create
those sweet, sweet 8-bit sounds. Unfortunately times have changed, and 32-bit Windows-only VSTs are no longer
sufficient, and in some cases don't work at all in newer DAWs. 

To that end I'm trying to re-create this fantastic plugin as an open-source project, both to build it using more
modern technology, and to ensure that, even if I disappear, others will be able to update it with (hopefully)
less effort.

# Overview
Most of the actual DSP code is written in [Faust](https://faust.grame.fr/index.html). The actual VST/AU/etc. plugin
will be built using [iPlug2](https://github.com/iPlug2/iPlug2).

# Progress
* Square wave - done
* Triangle wave - done
* Noise - trying to figure it out
* DPCM - not started yet
* UI - not started yet

[nes-vst]: https://www.mattmontag.com/projects-page/nintendo-vst
