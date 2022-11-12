/*
 *  UpdateListener.h
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

#ifndef _UPDATELISTENER_H
#define _UPDATELISTENER_H

#include "controls.h"

/**
 * an interface for classes which can be update() ed, eg the GUI objects.
 **/
class UpdateListener
{
public:
    virtual ~UpdateListener() = default;

    // The user has started to change this parameter
    virtual void parameterWillChange(Param) {}

    virtual void update			()		{;}
    virtual void UpdateParameter(Param, float controlValue) { update (); }
};

#endif
