# Glight
Glight allows a computer with DMX interface to act as a DMX controller for live control of stage lighting. It basically turns your computer into a traditional lighting console, but also adds several enhanced features. It makes use of the [Open Light Architecture](https://www.openlighting.org/ola/) (OLA) for connecting to the DMX interface, and can therefore make use of a wide variety of DMX interfaces and other connections that OLA offers.

Glight's website with documentation can be found here: https://glight.readthedocs.io/

Glight is specifically aimed at shows or events in which the lighting is controlled live. Any DMX device can be controlled, but Glight was in particular written for controlling less advanced lighting devices, such as light spots and pars, RGB LED lights/uplighting, etc. The development of Glight is focussed on Linux, makes use of the GTK toolkit and is written in C++. The following features are provided:

* Create scenes and chases and control these live; or design fully audio-annotated shows that run automatically.
* Several audio effects, including an accurate beat finder. This enables chases to follow the beat of the music. Glight makes use of the [Aubio](https://aubio.org/) beatfinding library, which enables true "musically"-aware chases that run very stable (in contrast to the "trigger on loud sounds" feature which is found on many light controllers).
* High performance C++ code requires minimal system performance.
* The interface is carefully designed, with clearly visible key-bindings to adjust sliders, for careful and precise live control of lights.
* Faders can be grouped, fader groups can be quickly hidden and shown, and faders can be set to auto-fade (up/down separately) and solo (for cross-fading).
* Lights can be visualized on-screen. This makes it possible to follow their state on the screen (although the real lights are much more exiting to watch!), and also allows dry testing without having a DMX interface connected.
* It is possible to 'disconnect' the real lighting fixtures from the controls. In those mode, the real lights continue their show (chases, etc.) while the use can change faders until happy with the new config, and only then affect the real lights.

<img src="https://raw.githubusercontent.com/aroffringa/glight/master/doc/Screenshot-2019-08-07-glight-0.8.1.png" alt="Screenshot" title="Screenshot of Glight in action" />

## Installation
The source of Glight can be downloaded from Github, currently from https://github.com/aroffringa/glight/. Glight has a few dependencies. Fortunately, these are all available as precompiled packages in Debian, Ubuntu and most other distributions. Apart from system tools such as cmake, these are the important dependencies:

- [Gtkmm](https://www.gtkmm.org/), the C++ interface to GTK+. `libgtkmm-3.0-dev` on Debian and Ubuntu.
- [Aubio](https://aubio.org/). `libaubio-dev` on Debian and Ubuntu.
- [Flac++](https://xiph.org/flac/). `libflac++-dev` on Debian and Ubuntu.
- [Alsa library](https://www.alsa-project.org/). `libasound2-dev` on Debian and Ubuntu.
- [libola](https://www.openlighting.org/ola/). `libola-dev` and `libprotobuf-dev` on Debian and Ubuntu.
- [libxml2](http://xmlsoft.org/). `libxml2-dev` on Debian and Ubuntu.

Once these prerequisited are installed, glight can be compiled by running the following from the source directory:

    mkdir build
    cd build
    cmake ../
    make -j 4

To also install glight in the system path, `sudo make install` can be run afterwards.

[Boost](https://www.boost.org/) is required to also compile the tests. (`libboost-test-dev` on Debian and Ubuntu).

## Contact
Glight is written by André Offringa. Feedback and bugreports are welcome; please use Github for this, or mail me at offringa@gmail.com.
