/* amSynth
 * (c) 2001-2005 Nick Dowell
 */

#ifndef _TUNINGMAP_H
#define _TUNINGMAP_H

#include <string>
#include <vector>

class TuningMap
{
/*
 * A TuningMap consists of two parts.
 * The "key map" maps from MIDI note numbers to logical note numbers for the scale.
 * (This is often the identity mapping, but if your scale has, for example,
 * 11 notes in it, you'll want to skip one per octave so the scale lines up
 * with the pattern of keys on a standard keyboard.)
 * The "scale" maps from those logical note numbers to actual pitches.
 * In terms of member variables, "scale" and "scaleDesc" belong to the scale,
 * and everything else belongs to the mapping.
 * For more information, refer to http://www.huygens-fokker.org/scala/
 */
public:
		TuningMap		();
		// Default is 12-equal, standard mapping

	int	loadScale		(const std::string & filename);
	int	loadKeyMap		(const std::string & filename);
	// Both return 0 on success

	void	defaultScale		();
	void	defaultKeyMap		();

	double	noteToPitch		(int note) const;
private:
	std::string		scaleDesc;

	std::vector<double>	scale;
	// note that logical indices to this begin with 1

	int			zeroNote;
	int			refNote;
	double			refPitch;
	int			mapRepeatInc;

	std::vector<int>	mapping; // -1 for unmapped

	double			basePitch;
	void			updateBasePitch		();
};

#endif

