/* amSynth
 * (c) 2001 Nick Dowell
 */

#ifndef _PRESETCONTROLLER_H
#define _PRESETCONTROLLER_H

#define PRESETS 128

#include <string>
#include "Preset.h"
#include "UpdateListener.h"

class PresetController {
  public:
    PresetController();
    ~PresetController();
  /**
   * Selects a Preset and makes it current, updating averything as neccessary.
   * If the requested preset does not exist, then the request is ignored, and
   * an error value is returned.
   * @param preset The preset number to be selected. Preset numbers start at
   * 0, and go up to (PRESETS-1), currently 127 following the MIDI spec.
   * @return 0 on success, -1 on failure.
   */
    int selectPreset(int preset);
  /**
   * Selects a preset by it's name, making it current and updating everything
   * as neccessary. If the requested preset does not exist, then the request 
   * is ignored, and an error value is returned.
   * @param preset The preset name to be selected.
   * @return 0 on success, -1 on failure.
   */
    int selectPreset(string preset);
  /**
   * @return Returns the current Preset object. Note that this is always the 
   * current Preset: it follows changes from selectPreset() invocations.
   */
    Preset & getCurrentPreset();
	Preset & getPreset( int preset );
	Preset & getPreset( string name );
  /**
  	* Commit the current preset to memory
  	*/
    void commitPreset();
	/**
	 * Selects a new, unused preset ready for editing.
	 */
	int newPreset();
	void deletePreset();
  /**
   * Saves the current Preset and Parameter values to disk.
   */
    int savePresets();
	
	/*
	 * Saves the active preset to the filename filename
	 */
	int exportPreset( string filename );
	int importPreset( string filename );
  /**
   * Loads the Preset & Parameter values from disk & restores them in the 
   * current PresetController.
   */
    int loadPresets();
  /**
   * Sets the UpdateListener object for this object.
   */
    void setUpdateListener(UpdateListener & ul);
  /**
   * @returns The number of the current Preset selected
   */
    int getCurrPresetNumber() 
	{ return currentPresetNo; } 
private:
    UpdateListener * updateListener;
    Preset *presets;
    Preset currentPreset;
	Preset blankPreset;
	Preset nullpreset;
    int currentPresetNo;
};

#endif
