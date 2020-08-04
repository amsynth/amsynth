/*
 *  Preset.h
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

#ifndef _PRESET_H
#define _PRESET_H

#ifdef __cplusplus

#include "Parameter.h"

#include <sstream>
#include <string>
#include <vector>


class Preset
{
public:
	Preset(const std::string name = "");
					
	Preset&			operator =		(const Preset& p);
	
	bool			isEqual			(const Preset &);

	const std::string& getName		() const { return mName; }
	void			setName			(const std::string name) { mName = name; }
	
	Parameter&		getParameter	(const std::string name);
	Parameter&		getParameter	(const int no) { return mParameters[no]; };
	const Parameter& getParameter	(const int no) const { return mParameters[no]; };
	
	size_t			ParameterCount	() const { return mParameters.size(); }
	
    void			randomise		();
    
    void			AddListenerToAll(UpdateListener*);

    std::string		toString		() { std::stringstream stream; toString(stream); return stream.str(); }
    void			toString		(std::stringstream &);
    bool			fromString		(const std::string &str);

	static bool 	shouldIgnoreParameter(int parameter);
	static void 	setShouldIgnoreParameter(int parameter, bool ignore);

	static std::string getIgnoredParameterNames();
	static void setIgnoredParameterNames(std::string);

private:
    std::string				mName;
	std::vector<Parameter>	mParameters;
	Parameter				nullparam{"null"};
};

#endif

#ifdef __cplusplus
extern "C" {
#endif

void get_parameter_properties(int parameter_index, double *minimum, double *maximum, double *default_value, double *step_size);

#ifdef __cplusplus
}
#endif

#endif

