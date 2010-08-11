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
					Preset			(const string name = "New Preset");
					
	Preset&			operator =		(Preset& p);
	
	bool			isEqual			(Preset &);

	const string	getName			() const { return mName; }
	void			setName			(const string name) { mName = name; }
	
	Parameter&		getParameter	(const string name);
	Parameter&		getParameter	(const int no) { return mParameters[no]; };
	
	unsigned		ParameterCount	() const { return mParameters.size(); }
	
    void			randomise		();
    
    void			AddListenerToAll(UpdateListener*);
    
    string			toString		();
    bool			fromString		(string str);

private:
    std::string				mName;
	std::vector<Parameter>	mParameters;
	Parameter				nullparam;
};

#endif
