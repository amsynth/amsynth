% AMSYNTH(1) amsynth 1.6.4 | User Commands
%
% September 2016

NAME
====

amsynth - a two oscillators software synthesizer.

SYNOPSIS
========

**amsynth** \[options\]

DESCRIPTION
===========

amsynth 1.6.4 \[Jan 19 2016\]  \[http://amsynth.github.io/\].
Copyright 2001-2017 Nick Dowell <nick@nickdowell.com>.

amsynth is an easy-to-use software synth with a classic subtractive synthesizer topology.

amsynth comes with ABSOLUTELY NO WARRANTY.
This is free software, and you are welcome to redistribute it
under certain conditions. See the file COPYING for details.

OPTIONS
=======

Any options given here override those in the config file (\$HOME/.amSynthrc).

`-h`

:   show a help message

`-v`

:   show version information

`-x`

:   run in headless mode (without GUI)

`-b` \<filename\>

:   use \<filename\> as the bank to store presets

`-t` \<filename\>

:   use \<filename\> as a tuning file

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

**\$HOME/.amSynthrc**

Configuration for amsynth.

**\$HOME/.amsynth/\***

Banks and others.

**\$HOME/.amSynth.presets**

Presets.

ENVIRONMENT
===========

nothing

BUGS & FEATURES REQUEST
=======================

If you find any bug or if you would like to propose a new feature, please report it to https://github.com/amsynth/amsynth/issues .

AUTHORS
=======

Nick Dowell and contributors. See complete list at amsynth's "Help" -> "About" Dialog.

MISC
====

This manual page was written by Olivier Humbert <trebmuh@tuxfamily.org> on September the 06 2016 as a part of the LibraZiK project (and can be used by others).
