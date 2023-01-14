/*
 *  PresetControllerView.h
 *
 *  Copyright (c) 2001-2019 Nick Dowell
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

#ifndef _PRESETCONTROLLERVIEW_H
#define _PRESETCONTROLLERVIEW_H

#include <gtk/gtk.h>

class PresetController;

class PresetControllerView { public:
	static PresetControllerView * instantiate(PresetController *presetController);
    virtual void update() = 0;
    virtual unsigned char getAuditionNote() = 0;
	virtual GtkWidget * getWidget() = 0;
    virtual ~PresetControllerView() {}
};

#endif
