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

	string		getName			() { return mName; }
	void		setName			(string name) { mName = name; }
	
	unsigned	ParameterCount	() { return mParameters.size(); }
	Parameter&	getParameter	(string name);
	Parameter&	getParameter	(int no) { return mParameters[no]; };
    void		clone			(Preset & preset);
    void		randomise		();
    void		AddListenerToAll(UpdateListener*);
private:
    std::string				mName;
	std::vector<Parameter>	mParameters;
	Parameter				nullparam;
};

#endif
