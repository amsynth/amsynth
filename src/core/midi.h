/*
 *  midi.h
 *
 *  Copyright (c) 2001-2012 Nick Dowell
 *
 *  This file is part of amsynth.
 *
 *  amsynth is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  amsynth is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with amsynth.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef _midi_h
#define _midi_h

/* See https://midi.org/summary-of-midi-1-0-messages for further reading  */

enum {
    MIDI_STATUS_NOTE_OFF                = 0x80,
    MIDI_STATUS_NOTE_ON                 = 0x90,
    MIDI_STATUS_NOTE_PRESSURE           = 0xA0,
    MIDI_STATUS_CONTROLLER              = 0xB0,
    MIDI_STATUS_PROGRAM_CHANGE          = 0xC0,
    MIDI_STATUS_CHANNEL_PRESSURE        = 0xD0,
    MIDI_STATUS_PITCH_WHEEL             = 0xE0,
};

/* https://midi.org/midi-1-0-control-change-messages
 */
enum {
    MIDI_CC_BANK_SELECT_MSB             = 0x00,
    MIDI_CC_MODULATION_WHEEL_MSB        = 0x01,
    MIDI_CC_PORTAMENTO_TIME             = 0x05,
    MIDI_CC_DATA_ENTRY_MSB              = 0x06,
    MIDI_CC_VOLUME                      = 0x07,
    MIDI_CC_PAN_MSB                     = 0x0A,

    MIDI_CC_BANK_SELECT_LSB             = 0x20,
    MIDI_CC_DATA_ENTRY_LSB              = 0x26,

    MIDI_CC_SUSTAIN_PEDAL               = 0x40,
    MIDI_CC_PORTAMENTO                  = 0x41,
    MIDI_CC_SOSTENUTO                   = 0x42,
    MIDI_CC_SOUND_CONTROLLER_2          = 0x47, // default: Timbre/Harmonic Intensity
    MIDI_CC_SOUND_CONTROLLER_3          = 0x48, // default: Release Time
    MIDI_CC_SOUND_CONTROLLER_4          = 0x49, // default: Attack Time
    MIDI_CC_SOUND_CONTROLLER_5          = 0x4A, // default: Brightness
    MIDI_CC_EFFECTS_1_DEPTH             = 0x5B,
    MIDI_CC_NRPN_LSB                    = 0x62,
    MIDI_CC_NRPN_MSB                    = 0x63,
    MIDI_CC_RPN_LSB                     = 0x64,
    MIDI_CC_RPN_MSB                     = 0x65,

    /* ------- Channel Mode Messages ------- */
    MIDI_CC_ALL_SOUND_OFF               = 0x78,
    MIDI_CC_RESET_ALL_CONTROLLERS       = 0x79,
    MIDI_CC_LOCAL_CONTROL               = 0x7A,
    MIDI_CC_ALL_NOTES_OFF               = 0x7B,
    MIDI_CC_OMNI_MODE_OFF               = 0x7C, /* + all notes off            */
    MIDI_CC_OMNI_MODE_ON                = 0x7D, /* + all notes off            */
    MIDI_CC_MONO_MODE_ON                = 0x7E, /* + poly off + all notes off */
    MIDI_CC_POLY_MODE_ON                = 0x7F, /* + mono off + all notes off */
};

#endif
