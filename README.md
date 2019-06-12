# Glight
Glight allows a computer with DMX interface to act as a DMX controller for live control of stage lighting. It basically turns your computer into a traditional lighting console, and adds some extra features. It makes use of the [Open Light Architecture](https://www.openlighting.org/ola/) (OLA) for connecting to the DMX interface, and can therefore make use of a wide variety of DMX interfaces and other connections that OLA offers.

Glight is specifically aimed at shows or events in which the lighting is controlled live. Any DMX device can be controlled, but Glight was in particular written for controlling less advanced lighting devices, such as light spots and pars, RGB LED lights/uplighting, etc. Glight does currently not have specific features for scanners / moving heads. The development of Glight is focussed on Linux, makes use of the GTK toolkit and is written in C++. The following features are provided:

* Create scenes and chases and control these live; or design fully audio-annotated shows that run automatically.
* Several audio effects, including an accurate beat finder. This enables chases to follow the beat of the music. Glight makes use of the [Aubio](https://aubio.org/) beatfinding library, which enables true "musically"-aware chases that run very stably (in contrast to the "trigger on loud sounds" feature which is found on many light controllers).
* High performance C++ code requires minimal system performance.
* The interface is carefully designed, with clearly visible key-bindings to adjust sliders, for careful and precise live control of lights.
* Lights can be visualized on-screen. This makes it possible to follow the state / show on the screen (although the real lights are much more exiting to watch!), and also allows dry testing without having a DMX interface connected. 

<img src="https://raw.githubusercontent.com/aroffringa/glight/master/doc/Screenshot-2019-06-12-glight-0.8.png" alt="Screenshot" title="Screenshot of Glight in action"/>

## Installation
The source of Glight can be downloaded from Github, currently from https://github.com/aroffringa/glight/. Glight has a few dependencies. Fortunately, these are all available as precompiled packages in Debian, Ubuntu and most other distributions. Apart from system tools such as cmake, these are the important dependencies:

- [Gtkmm](https://www.gtkmm.org/), the C++ interface to GTK+. `libgtkmm-3.0-dev` on Debian and Ubuntu.
- [Boost](https://www.boost.org/). `libboost-system-dev` on Debian and Ubuntu.
- [Aubio](https://aubio.org/). `libaubio-dev` on Debian and Ubuntu.
- [Flac++](https://xiph.org/flac/). `libflac++-dev` on Debian and Ubuntu.
- [alsa library](https://www.alsa-project.org/). `libasound2-dev` on Debian and Ubuntu.
- [libola](https://www.openlighting.org/ola/). `libola-dev` on Debian and Ubuntu.
- [libxml2](http://xmlsoft.org/). `libxml2-dev` on Debian and Ubuntu.

Once these prerequisited are installed, glight can be compiled by running the following from the source directory:

    mkdir build
    cd build
    cmake ../
    make -j 4

To also install glight in the system path, `sudo make install` can be run afterwards.

Glight is written by André Offringa. Feedback and bugreports are welcome; please use Github for this.
