/* amSynth
 * (c) 2001-2003 Nick Dowell
 */
 
#include "Config.h"

#include <fstream>
#include <iostream>

static string amsynthrc_fname;

Config::Config()
{
	amsynthrc_fname = string(getenv("HOME")) + string("/.amSynthrc");
	realtime = sample_rate = midi_channel = active_voices = polyphony = debug_drivers = load_font = xruns = 0;
	xfontname = "-*-helvetica-medium-r-*-*-*-100-*-*-*-*-*-*";
}

void
Config::Defaults	()
{
	audio_driver = "auto";
	midi_driver = "auto";
	oss_midi_device = "/dev/midi";
	midi_channel = 0;
	oss_audio_device = "/dev/dsp";
	alsa_audio_device = "default";
	sample_rate = 44100;
	channels = 2;
	buffer_size = 128;
	polyphony = 10;
	alsa_seq_client_name = "amSynth";
	current_bank_file = string (getenv ("HOME")) +
		string("/.amSynth.presets");
}

bool
Config::ParseCOpts	(int argc, char* argv[])
{
	int opt;
	while( (opt=getopt(argc, argv, "vhstdm:c:a:r:p:b:"))!= -1 ) {
		switch(opt) {
			case 'm': 
				midi_driver = optarg;
				break;
			case 'b': 
				current_bank_file = optarg;
				break;
			case 'c':
				midi_channel = atoi( optarg ); 
				break;
			case 'a':
				audio_driver = optarg; 
				break;
			case 'd':
				debug_drivers = 1;
				break;
			case 'r':
				sample_rate = atoi( optarg );
				break;
			case 'p':
				polyphony = atoi( optarg );
				break;	
			default:
				break;
		}
	}
	return true;
}

int
Config::load	()
{
	char buffer[100];

	fstream file( amsynthrc_fname.c_str(), ios::in );
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
			*buffer = '!';
		} else {
			file >> buffer;
		}
	}
	file.close();
	
	return 0;
}

int
Config::save	()
{
	fstream ofile ( amsynthrc_fname.c_str(), ios::in | ios::out );
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
