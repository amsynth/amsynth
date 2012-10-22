/*
  Copyright 2012 David Robillard <http://drobilla.net>

  Permission to use, copy, modify, and/or distribute this software for any
  purpose with or without fee is hereby granted, provided that the above
  copyright notice and this permission notice appear in all copies.

  THIS SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
  WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
  MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
  ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
  WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
  ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
  OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
*/

/**
   @file midi.h
   C definitions for the LV2 MIDI extension <http://lv2plug.in/ns/ext/midi>.
*/

#ifndef LV2_MIDI_H
#define LV2_MIDI_H

#define LV2_MIDI_URI    "http://lv2plug.in/ns/ext/midi"
#define LV2_MIDI_PREFIX LV2_MIDI_URI "#"

#define LV2_MIDI__ActiveSense      LV2_MIDI_PREFIX "ActiveSense"
#define LV2_MIDI__Aftertouch       LV2_MIDI_PREFIX "Aftertouch"
#define LV2_MIDI__Bender           LV2_MIDI_PREFIX "Bender"
#define LV2_MIDI__ChannelPressure  LV2_MIDI_PREFIX "ChannelPressure"
#define LV2_MIDI__Chunk            LV2_MIDI_PREFIX "Chunk"
#define LV2_MIDI__Clock            LV2_MIDI_PREFIX "Clock"
#define LV2_MIDI__Continue         LV2_MIDI_PREFIX "Continue"
#define LV2_MIDI__Controller       LV2_MIDI_PREFIX "Controller"
#define LV2_MIDI__MidiEvent        LV2_MIDI_PREFIX "MidiEvent"
#define LV2_MIDI__NoteOff          LV2_MIDI_PREFIX "NoteOff"
#define LV2_MIDI__NoteOn           LV2_MIDI_PREFIX "NoteOn"
#define LV2_MIDI__ProgramChange    LV2_MIDI_PREFIX "ProgramChange"
#define LV2_MIDI__QuarterFrame     LV2_MIDI_PREFIX "QuarterFrame"
#define LV2_MIDI__Reset            LV2_MIDI_PREFIX "Reset"
#define LV2_MIDI__SongPosition     LV2_MIDI_PREFIX "SongPosition"
#define LV2_MIDI__SongSelect       LV2_MIDI_PREFIX "SongSelect"
#define LV2_MIDI__Start            LV2_MIDI_PREFIX "Start"
#define LV2_MIDI__Stop             LV2_MIDI_PREFIX "Stop"
#define LV2_MIDI__SystemCommon     LV2_MIDI_PREFIX "SystemCommon"
#define LV2_MIDI__SystemExclusive  LV2_MIDI_PREFIX "SystemExclusive"
#define LV2_MIDI__SystemMessage    LV2_MIDI_PREFIX "SystemMessage"
#define LV2_MIDI__SystemRealtime   LV2_MIDI_PREFIX "SystemRealtime"
#define LV2_MIDI__Tick             LV2_MIDI_PREFIX "Tick"
#define LV2_MIDI__TuneRequest      LV2_MIDI_PREFIX "TuneRequest"
#define LV2_MIDI__VoiceMessage     LV2_MIDI_PREFIX "VoiceMessage"
#define LV2_MIDI__benderValue      LV2_MIDI_PREFIX "benderValue"
#define LV2_MIDI__byteNumber       LV2_MIDI_PREFIX "byteNumber"
#define LV2_MIDI__chunk            LV2_MIDI_PREFIX "chunk"
#define LV2_MIDI__controllerNumber LV2_MIDI_PREFIX "controllerNumber"
#define LV2_MIDI__controllerValue  LV2_MIDI_PREFIX "controllerValue"
#define LV2_MIDI__noteNumber       LV2_MIDI_PREFIX "noteNumber"
#define LV2_MIDI__pressure         LV2_MIDI_PREFIX "pressure"
#define LV2_MIDI__programNumber    LV2_MIDI_PREFIX "programNumber"
#define LV2_MIDI__property         LV2_MIDI_PREFIX "property"
#define LV2_MIDI__songNumber       LV2_MIDI_PREFIX "songNumber"
#define LV2_MIDI__songPosition     LV2_MIDI_PREFIX "songPosition"
#define LV2_MIDI__status           LV2_MIDI_PREFIX "status"
#define LV2_MIDI__statusMask       LV2_MIDI_PREFIX "statusMask"
#define LV2_MIDI__velocity         LV2_MIDI_PREFIX "velocity"

#endif  /* LV2_MIDI_H */
