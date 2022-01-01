/*
 *  amsynth_lv2.h
 *
 *  Copyright (c) 2001-2021 Nick Dowell
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

#include <lv2/lv2plug.in/ns/ext/atom/atom.h>
#include <lv2/lv2plug.in/ns/ext/atom/forge.h>
#include <lv2/lv2plug.in/ns/ext/midi/midi.h>
#include <lv2/lv2plug.in/ns/ext/patch/patch.h>
#include <lv2/lv2plug.in/ns/ext/state/state.h>
#include <lv2/lv2plug.in/ns/ext/worker/worker.h>
#include <lv2/lv2plug.in/ns/extensions/ui/ui.h>

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

//
// Copied from lv2_util.h which is missing from lv2-dev in ubuntu < 20.04
//
// https://github.com/lv2/lv2/issues/16
//

/*
  Copyright 2016 David Robillard <http://drobilla.net>

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

#include <stdarg.h>
#include <stdbool.h>
#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
   Return the data for a feature in a features array.

   If the feature is not found, NULL is returned.  Note that this function is
   only useful for features with data, and can not detect features that are
   present but have NULL data.
*/
static inline void*
lv2_features_data(const LV2_Feature*const* features,
                  const char* const        uri)
{
	if (features) {
		for (const LV2_Feature*const* f = features; *f; ++f) {
			if (!strcmp(uri, (*f)->URI)) {
				return (*f)->data;
			}
		}
	}
	return NULL;
}

/**
   Query a features array.

   This function allows getting several features in one call, and detect
   missing required features, with the same caveat of lv2_features_data().

   The arguments should be a series of const char* uri, void** data, bool
   required, terminated by a NULL URI.  The data pointers MUST be initialized
   to NULL.  For example:

   @code
   LV2_URID_Log* log = NULL;
   LV2_URID_Map* map = NULL;
   const char* missing = lv2_features_query(
        features,
        LV2_LOG__log,  &log, false,
        LV2_URID__map, &map, true,
        NULL);
   @endcode

   @return NULL on success, otherwise the URI of this missing feature.
*/
static inline const char*
lv2_features_query(const LV2_Feature* const* features, ...)
{
	va_list args;
	va_start(args, features);

	const char* uri = NULL;
	while ((uri = va_arg(args, const char*))) {
		void** data     = va_arg(args, void**);
		bool   required = va_arg(args, int);

		*data = lv2_features_data(features, uri);
		if (required && !*data) {
			return uri;
		}
	}

	return NULL;
}

#ifdef __cplusplus
}  /* extern "C" */
#endif


#endif //AMSYNTH_LV2_H
