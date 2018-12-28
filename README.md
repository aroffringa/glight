# glight
Glight allows a computer with DMX interface to act as a DMX controller for live control of stage lighting. It basically turns your computer into a traditional lighting console, and adds some extra features. It makes use of the [Open Light Architecture](https://www.openlighting.org/ola/) (OLA) for connecting to the DMX interface, and can therefore make use of a wide variety of DMX interfaces and other connections that OLA offers.

Glight is specifically aimed at shows or events in which the lighting is controlled live. It can control any DMX device, but is in particular handy for less advanced lighting devices, such as light spots and pars, RGB LED lights/uplighting, etc. Glight does currently not have specific features for scanners, mostly because the author does not have any scanners :). The development of Glight is focussed on Linux and makes use of the GTK toolkit. The following features are provided:

* Create scenes and chases and control these live, or design fully audio-annotated shows that run automatically.
* Accurate beat finder, which enables chases to follow the beat of the music. Glight makes use of the [Aubio](https://aubio.org/) beatfinding library, which enables true "musically"-aware chases (in contrast to the "switch lights on loud sounds" feature which is found on many light controllers).
* The interface is carefully designed, with clearly visible key-bindings for controlling sliders, for careful and precise live control of lights.
* Lights can be visualized on-screen. This makes it possible to follow the state / show on the screen (although the real lights are much more exiting to watch!), and also allows dry testing without having a DMX interface connected. 
