/* amSynth
 * (c) 2001-2005 Nick Dowell
 */

#ifndef _PRESET_H
#define _PRESET_H

#include <string>
#include <vector>
#include "Parameter.h"

/**
 * @class Preset
 * the Preset holds all the parameters for a paticular timbre (preset).
 */

class Preset
{
public:
					Preset			();
	Preset&			operator =		(Preset& p);

	const string	getName			() const { return mName; }
	void			setName			(const string name) { mName = name; }
	
	unsigned		ParameterCount	() const { return mParameters.size(); }
	Parameter&		getParameter	(const string name);
	Parameter&		getParameter	(const int no) { return mParameters[no]; };
    void			randomise		();
    void			AddListenerToAll(UpdateListener*);

private:
    std::string				mName;
	std::vector<Parameter>	mParameters;
	Parameter				nullparam;
};

#endif
