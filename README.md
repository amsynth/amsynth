# amsynth [![Build Status](https://travis-ci.org/amsynth/amsynth.svg?branch=master)](https://travis-ci.org/amsynth/amsynth)

amsynth is an analog modelling (a.k.a virtual analog) software synthesizer.

It mimics the operation of early analog subtractive synthesizers with
classic oscillator waveforms, envelopes, filter, modulation and effects.

The aim is to make it easy to create and modify sounds.

## Features

* Dual oscillators (sine / saw / square / noise) with hard sync
* 12/24 dB/oct resonant filter (low-pass / high-pass / band-pass / notch)
* Mono / poly / legato keyboard modes
* Dual ADSR envelope generators (filter & amplitude)
* LFO which can modulate the oscillators, filter, and amplitude
* Distortion and reverb
* Hundreds of presets

There are currently several different ways to run amsynth:

* Stand-alone application using JACK, ALSA or OSS
* DSSI plug-in
* LV2 plug-in
* VST plug-in

## Building VST plug-in

* Download Steinberg's VST SDK and extract files from archive
* run ./configure VST_SDK=/path/to/vstsdk2.4
* run `make` and `make install`
* `amsynth_vst.so` will be installed to `$(prefix)/lib/vst`


## Preset bank management

The default user bank is located at `~/.amSynth.presets`

amsynth now allows you to quickly access other preset banks that are stored in
the following directories:

* `~/.amsynth/banks`
* `/usr/share/amsynth/banks` (read-only access)

Any valid preset banks that are found in these directories will be show in the
drop-down bank selector on the GUI (standalone version only.)

Plug-in versions can load preset banks by right-clicking on the GUI's background.
