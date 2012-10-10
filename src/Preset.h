/* amSynth
 * (c) 2001-2005 Nick Dowell
 */

#ifndef _PRESET_H
#define _PRESET_H

#ifdef __cplusplus

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
					Preset			(const string name = "");
					
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

#ifdef __cplusplus
extern "C" {
#endif

void get_parameter_properties(int parameter_index, double *minimum, double *maximum, double *default_value, double *step_size);

#ifdef __cplusplus
}
#endif

#endif

