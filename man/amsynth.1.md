% AMSYNTH(1) amsynth VERSION | User Commands
%
% January 2017

NAME
====

amsynth - a two oscillator subtractive software synthesizer

SYNOPSIS
========

**amsynth** \[options\]

DESCRIPTION
===========

amsynth is an easy-to-use software synth with a classic subtractive synthesizer topology.

OPTIONS
=======

The following options override those in the config file (\$HOME/.amSynthrc).

`-h`

:   show a help message

`-v`

:   show version information

`-x`

:   run in headless mode (without GUI)

`-b` \<file\>

:   use \<file\> as the bank to store presets

`-t` \<file\>

:   use \<file\> as a tuning file

`-a` \<string\>

:   set the sound output driver to use \[alsa/oss/auto(default)\]

`-r` \<int\>

:   set the sampling rate to use

`-m` \<string\>

:   set the MIDI driver to use \[alsa/oss/auto(default)\]

`-c` \<int\>

:   set the MIDI channel to respond to (default=all)

`-p` \<int\>

:   set the polyphony (maximum active voices)

`-n` \<name\>

:   specify the JACK client name to use

`--jack_autoconnect[=<true|false>]`

:   automatically connect jack audio ports to hardware I/O ports. (Default: true)

FILES
=====

`$HOME/.amSynthrc`

:   Configuration for amsynth.

`$HOME/.amsynth/*`

:   Banks and others.

`$HOME/.amSynth.presets`

:   Presets.

ENVIRONMENT
===========

`AMSYNTH_NO_GUI`

:   If AMSYNTH\_NO\_GUI is set, amsynth is started in headless mode (without GUI).

`AMSYNTH_SKIN`

:   Specifies the directory from which amsynth should load its skin.

BUGS & FEATURE REQUESTS
=======================

If you find a bug or if you would like to propose a new feature, please report it at https://github.com/amsynth/amsynth/issues .

AUTHORS
=======

Nick Dowell and contributors. See complete list in amsynth's "Help" -> "About" dialog.

The initial version of this manual page was written by Olivier Humbert <trebmuh@tuxfamily.org> on September 06, 2016 as a part of the LibraZiK project (and can be used by others).
