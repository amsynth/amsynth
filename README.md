# amsynth

amsynth is is a free and open-source analog modeling (a.k.a virtual analog)
software synthesizer.

It mimics the operation of early analog subtractive synthesizers with classic
oscillator waveforms, envelopes, filter, modulation and effects.

The aim is to make it easy to create and modify sounds.

## Features

- 2 oscillators (sine / saw / square / noise) with hard sync
- 12/24 dB/oct resonant filter (low-pass / high-pass / band-pass / notch)
- 2 ADSR envelope generators (for filter & amplitude)
- LFO which can modulate the oscillators, filter, and amplitude
- Mono, Poly and Legato keyboard modes
- Distortion and reverb
- Hundreds of presets

There are currently several different ways to run amsynth:

- Stand-alone application using JACK, ALSA or OSS
- LV2 plug-in
- VST2 plug-in
- DSSI plug-in

## Presets

amsynth stores presets in banks, each containing 128 presets.

The default user bank is located at `~/.local/share/amsynth/banks/default`
and any preset banks found in the following directories will also be
available to select in the GUI:

- `~/.local/share/amsynth/banks` (user banks)
- `/usr/share/amsynth/banks` (read-only "factory" banks)

# Installation

## Linux

While the simplest way to install amsynth is from your Linux distribution's
package repository, the available versions tend to be quite old.

Package versions: https://repology.org/project/amsynth/versions

More recent builds of amsynth for Ubuntu related distributions are available
through the amsynth PPA:

https://code.launchpad.net/~nick-nickdowell/+archive/ubuntu/amsynth-develop

## FreeBSD

A FreeBSD port of amsynth is available:

https://www.freshports.org/audio/amsynth

## Building from source

### 1. Install Prerequisites

In order to successfully build amsynth you will need compiler with support for
C++14 language features, GNU make, and the following development libraries:

- curl
- Freetype
- GNU autoconf & automake
- GNU gettext & intltool
- GNU make
- libpng
- pandoc
- xcursor
- xinerama
- xrandr
- zlib

The following packages are optional but recommended:

- JACK for audio output
- ALSA for midi & audio
- DSSI and liblo

On Debian and Ubuntu based systems, the recommended packages may be installed
with the following command:

```sh
sudo apt install autopoint dssi-dev intltool g++ libasound2-dev libcurl4-openssl-dev \
	libfreetype-dev libjack-dev liblo-dev libpng-dev libtool libxcursor-dev \
	libxinerama-dev libxrandr-dev pandoc zlib1g-dev
```

### 2. Prepare the source tree

If using a Git checkout of the source code, first run the `autogen.sh` script
to prepare the source tree and build system:

```sh
./autogen.sh
```

### 3. Configure the build

Run the `configure` script to generate Makefiles adapted to your build
environment. Run with `--help` to learn about the available options.

```sh
./configure
```

### 4. Build

Run `make` to build the standalone app and all configured plugin targets. On
multicore systems passing `-j $(nproc)` will significantly speed up the build.

```sh
make -j $(nproc)
```

### 5. Install

To install the software and data in the configured location (which can be
changed using the `--prefix` configure option) run:

```sh
sudo make install
```
