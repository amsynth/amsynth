/* amSynth
 * (c) 2001-2003 Nick Dowell
 */
 
#include "Config.h"

#include <fstream>
#include <iostream>

Config::Config()
{
	realtime = sample_rate = midi_channel = active_voices = polyphony = debug_drivers = load_font = 0;
	xfontname = "";
}

int
Config::load	( string filename )
{
	char buffer[100];

	fstream file( filename.c_str(), ios::in );
	while( file.good() ) {
		file >> buffer;
		if( string(buffer)=="#" ){
			// ignore lines beginning with '#' (comments)
			// this next line is needed to deal with a line with 
			// just a '#'
			file.unget();
			// this moves file on by a whole line, so we ignore it
			file.get(buffer,100);
		} else if (string(buffer)=="audio_driver"){
			file >> buffer;
			audio_driver = string(buffer);
		} else if (string(buffer)=="midi_driver"){
			file >> buffer;
			midi_driver = buffer;
		} else if (string(buffer)=="oss_midi_device"){
			file >> buffer;
			oss_midi_device = string(buffer);
		} else if (string(buffer)=="midi_channel"){
			file >> buffer;
			midi_channel = atoi(buffer);
		} else if (string(buffer)=="oss_audio_device"){
			file >> buffer;
			oss_audio_device = string(buffer);
		} else if (string(buffer)=="alsa_audio_device"){
			file >> buffer;
			alsa_audio_device = string(buffer);
		} else if (string(buffer)=="sample_rate"){
			file >> buffer;
			sample_rate = atoi(buffer);
		} else if (string(buffer)=="polyphony"){
			file >> buffer;
			polyphony = atoi(buffer);
		} else if (string(buffer)=="gui_font"){
			char tmp;
			char str[256];
			char *strpt = str;
			int whitespace = 1;
			
			for (int i=0; i<256; i++)
			{
				file.get( tmp );

				if (!whitespace || tmp != ' ')
				{
					if (tmp == '\n')
					{
						*strpt++ = '\0';
						break;
					}
					whitespace = 0;
					*strpt++ = tmp;
				}
			}
			
			xfontname = str;
			load_font = 1;
		} else {
			file >> buffer;
		}
	}
	file.close();
	
	return 0;
}

int
Config::save	( string filename )
{
	fstream ofile ( filename.c_str(), ios::in | ios::out );
	if (load_font)
	{	
		// replace fontname in .amSynthrc with new fontname
		
		// seek until we find the gui_font key
		char chdata[200];
		int fileidx;
		while (ofile.good())
		{
			// get offset for line we are about to read;
			fileidx = ofile.tellg ( );
			ofile.seekp ( ofile.tellg() );
			ofile.getline ( chdata, 200 );
			if (strncmp(chdata,"gui_font",8)==0)
			{
				ofile.seekp ( fileidx );
				ofile << "gui_font " << xfontname << endl;
			}
		}
	}
	else
	{
		// create fontname entry in .amSynthrc
		ofile.seekp ( 0, ios::end );
		ofile << "gui_font " << xfontname << endl;
	}
	ofile.close ( );
	return 0;
}
