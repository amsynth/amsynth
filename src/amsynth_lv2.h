/*
 *  amsynth_lv2.h
 *
 *  Copyright (c) 2001-2017 Nick Dowell
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

#ifndef AMSYNTH_LV2_H
#define AMSYNTH_LV2_H

#define AMSYNTH_LV2_URI             "http://code.google.com/p/amsynth/amsynth"
#define AMSYNTH__tuning_kbm_file    AMSYNTH_LV2_URI "#tuning_kbm_file"
#define AMSYNTH__tuning_scl_file    AMSYNTH_LV2_URI "#tuning_scl_file"

enum {
    PORT_CONTROL            = 0,
    PORT_NOTIFY             = 1,
    PORT_AUDIO_L            = 2,
    PORT_AUDIO_R            = 3,
    PORT_FIRST_PARAMETER    = 4,
};

#endif //AMSYNTH_LV2_H
