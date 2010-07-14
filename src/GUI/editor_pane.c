////////////////////////////////////////////////////////////////////////////////

#include "editor_pane.h"
#include "../controls.h"
#include "bitmap_knob.h"
#include "bitmap_popup.h"

////////////////////////////////////////////////////////////////////////////////

#define SIZEOF_ARRAY( a ) ( sizeof((a)) / sizeof((a)[0]) )

////////////////////////////////////////////////////////////////////////////////
//
// Graphics resources
//
extern const char *background_png;
extern const int   background_png_size;
//
extern const char *knob_png;
extern const int   knob_png_size;
//
extern const char *waveform_png;
extern const int   waveform_png_size;
//
extern const char *slider_boost_1_png;
extern const int   slider_boost_1_png_size;
//
extern const char *slider_boost_2_png;
extern const int   slider_boost_2_png_size;
//
////////////////////////////////////////////////////////////////////////////////

static GdkPixbuf *editor_pane_bg = NULL;

static GtkAdjustment *adjustments[kControls_End];

////////////////////////////////////////////////////////////////////////////////

static GdkPixbuf *
gdk_pixbuf_new_from_file_data (const void *buf, gsize count)
{
	GdkPixbuf *pixbuf = NULL;
	GdkPixbufLoader *loader = gdk_pixbuf_loader_new ();
	gdk_pixbuf_loader_write (loader, buf, count, NULL);
	gdk_pixbuf_loader_close (loader, NULL);
	pixbuf = g_object_ref (gdk_pixbuf_loader_get_pixbuf (loader));
	g_object_unref (G_OBJECT (loader));
	return pixbuf;
}

////////////////////////////////////////////////////////////////////////////////

#define TYPE_KNOB	1000
#define TYPE_POPUP	1001
#define TYPE_SWITCH	1002

typedef struct
{
	gchar bmp_name[32];
	const char *data;
	int   length;
	guint fr_width;
	guint fr_height;
	guint fr_count;
}
resource_info;

typedef struct
{
	guint type;
	guint x;
	guint y;
	gchar bmp_name[32];
	guint paramid;
}
widget_info;

////////////////////////////////////////////////////////////////////////////////

const widget_info widget_layout[] = 
{
	{ TYPE_KNOB,	 10,  10, "knob_boost_cut",		kOsc1Pulsewidth		},	// osc 1 pw
	{ TYPE_POPUP,	 10,  60, "popup_waveforms",	kOsc1Waveform		},	// osc 1 waveform

	{ TYPE_KNOB,	 10, 100, "knob_boost_cut",		kOsc2Pulsewidth		},	// osc 2 pw
	{ TYPE_POPUP,	 10, 160, "popup_waveforms",	kOsc2Waveform		},	// osc 2 waveform
	{ TYPE_POPUP,	 10, 190, "popup_waveforms",	kOsc2Sync			},	// osc 2 sync
	{ TYPE_KNOB,	 70, 100, "knob_boost_cut",		kOsc2Octave			},	// osc 2 octave
	{ TYPE_KNOB,	 70, 160, "knob_boost_cut",		kOsc2Detune			},	// osc 2 detune
	
	{ TYPE_KNOB,	100,  10, "knob_boost_cut",		kOscMix				},	// osc mix
	{ TYPE_POPUP,	100,  60, "popup_waveforms",	kOscMixRingMod		},	// osc mix ring mod

	{ TYPE_KNOB,	195,  10, "knob_boost_cut",		kFilterCutoff		},	// filter cutoff
	{ TYPE_KNOB,	245,  10, "knob_boost_cut",		kFilterResonance	},	// filter resonance
	{ TYPE_KNOB,	295,  10, "knob_boost_cut",		kFilterEnvAmount	},	// filter envelope
	{ TYPE_KNOB,	170,  70, "knob_boost_cut",		kFilterAttack		},	// filter attack
	{ TYPE_KNOB,	220,  70, "knob_boost_cut",		kFilterDecay		},	// filter decay
	{ TYPE_KNOB,	270,  70, "knob_boost_cut",		kFilterSustain		},	// filter sustain
	{ TYPE_KNOB,	320,  70, "knob_boost_cut",		kFilterRelease		},	// filter release

	{ TYPE_KNOB,	460,  10, "knob_boost_cut",		kMasterVol			},	// amp volume
	{ TYPE_KNOB,	510,  10, "knob_boost_cut",		kDistortionCrunch	},	// amp distortion
	{ TYPE_KNOB,	410,  70, "knob_boost_cut",		kAmpAttack			},	// amp attack
	{ TYPE_KNOB,	460,  70, "knob_boost_cut", 	kAmpDecay			},	// amp decay
	{ TYPE_KNOB,	510,  70, "knob_boost_cut", 	kAmpSustain			},	// amp sustain
	{ TYPE_KNOB,	560,  70, "knob_boost_cut", 	kAmpRelease			},	// amp release
	
	{ TYPE_KNOB,	410, 300, "slider_boost_2",		kAmpAttack			},	// amp attack
	{ TYPE_KNOB,	460, 300, "slider_boost_2", 	kAmpDecay			},	// amp decay
	{ TYPE_KNOB,	510, 300, "slider_boost_2", 	kAmpSustain			},	// amp sustain
	{ TYPE_KNOB,	560, 300, "slider_boost_2", 	kAmpRelease			},	// amp release

	{ TYPE_KNOB,	170, 160, "knob_boost_cut",		kLFOFreq			},	// lfo freq
	{ TYPE_POPUP,	170, 210, "popup_waveforms",	kLFOWaveform		},	// lfo waveform
	{ TYPE_KNOB,	220, 160, "knob_boost_cut",		kFreqModAmount		},	// lfo -> vco
	{ TYPE_KNOB,	270, 160, "knob_boost_cut",		kFilterModAmount	},	// lfo -> filt
	{ TYPE_KNOB,	320, 160, "knob_boost_cut",		kAmpModAmount		},	// lfo -> vca

	{ TYPE_KNOB,	410, 160, "knob_boost_cut",		kReverbWet			},	// reverb amount
	{ TYPE_KNOB,	460, 160, "knob_boost_cut",		kReverbRoomsize		},	// reverb room size
	{ TYPE_KNOB,	510, 160, "knob_boost_cut",		kReverbWidth		},	// reverb stereo width
	{ TYPE_KNOB,	560, 160, "knob_boost_cut",		kReverbDamp			},	// reverb damping
};

////////////////////////////////////////////////////////////////////////////////

static void
editor_pane_expose_event_handler (GtkWidget *widget, gpointer data)
{
	gdk_draw_pixbuf(
		widget->window,
		NULL,	// gc
		editor_pane_bg,
		0,	// src_x
		0,	// src_y
		widget->allocation.x,
		widget->allocation.y,
		widget->allocation.width,
		widget->allocation.height,
		GDK_RGB_DITHER_NONE, 0, 0
	);
}

////////////////////////////////////////////////////////////////////////////////

GtkWidget *
editor_pane_new (GtkAdjustment **adjustments)
{
	const resource_info resources[] = 
	{
		{ "knob_boost_cut",		knob_png,			knob_png_size,		 		49, 49, 49 },
		{ "popup_waveforms",	waveform_png,		waveform_png_size,			49, 21,  5 },
		{ "slider_boost_1",		slider_boost_1_png,	slider_boost_1_png_size, 	31, 49, 31 },
		{ "slider_boost_2",		slider_boost_2_png,	slider_boost_2_png_size, 	49, 99, 99 },
	};

	GtkWidget *fixed = gtk_fixed_new ();
	
	editor_pane_bg = gdk_pixbuf_new_from_file_data (background_png, background_png_size);
	
	gtk_widget_set_usize (fixed, gdk_pixbuf_get_width (editor_pane_bg), gdk_pixbuf_get_height (editor_pane_bg));
	g_signal_connect (GTK_OBJECT (fixed), "expose-event", (GtkSignalFunc) editor_pane_expose_event_handler, (gpointer) NULL);
	
	int i;
	
	GData *resdata = NULL;
	GData *resinfo = NULL;
	g_datalist_init (&resdata);
	g_datalist_init (&resinfo);
	
	for (i=0; i<SIZEOF_ARRAY(resources); i++)
	{
		const resource_info *info = resources + i;
		GdkPixbuf *pixbuf = gdk_pixbuf_new_from_file_data (info->data, info->length);
		g_datalist_set_data (&resdata, info->bmp_name, (gpointer)pixbuf);
		g_datalist_set_data (&resinfo, info->bmp_name, (gpointer)info);
	}

	const gchar *waveform_strings[] = { "sine", "square / pulse", "saw / triangle", "white noise", "noise + sample & hold" };
	
	for (i=0; i<SIZEOF_ARRAY(widget_layout); i++)
	{
		const widget_info	*info = widget_layout + i;
		const resource_info	*res = g_datalist_get_data (&resinfo, info->bmp_name);

		GtkWidget *widget = NULL;
		GdkPixbuf *frames = GDK_PIXBUF (g_datalist_get_data (&resdata, info->bmp_name));
		GdkPixbuf *subpixpuf = gdk_pixbuf_new_subpixbuf (editor_pane_bg, info->x, info->y, res->fr_width, res->fr_height);
		GtkAdjustment *adj = adjustments[info->paramid];
		
		switch (info->type)
		{
		case TYPE_KNOB:
			widget = bitmap_knob_new (adj, frames, res->fr_width, res->fr_height, res->fr_count);
			bitmap_knob_set_bg (widget, subpixpuf);
			break;
		
		case TYPE_POPUP:
			widget = bitmap_popup_new (adj, frames, res->fr_width, res->fr_height, res->fr_count);
			bitmap_popup_set_bg (widget, subpixpuf);
			bitmap_popup_set_strings (widget, (const gchar **)waveform_strings);
			break;
		}
		
		g_object_unref (G_OBJECT (subpixpuf));
		gtk_fixed_put (GTK_FIXED (fixed), widget, info->x, info->y);
	}
	
	return fixed;
}

////////////////////////////////////////////////////////////////////////////////

