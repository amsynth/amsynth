////////////////////////////////////////////////////////////////////////////////

#include "editor_pane.h"
#include "../controls.h"
#include "bitmap_knob.h"
#include "bitmap_popup.h"

////////////////////////////////////////////////////////////////////////////////

#define SIZEOF_ARRAY( a ) ( sizeof((a)) / sizeof((a)[0]) )

////////////////////////////////////////////////////////////////////////////////

static GdkPixbuf *editor_pane_bg = NULL;

static GtkAdjustment *adjustments[kControls_End];

////////////////////////////////////////////////////////////////////////////////

typedef struct
{
	const char *data;
	int   length;
	guint fr_width;
	guint fr_height;
	guint fr_count;
}
resource_info;

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

#define HANDLE_GERROR( gerror ) \
	if (gerror) { \
		fprintf (stderr, "GError: %s\n", gerror->message); \
		g_error_free (gerror); \
		gerror = NULL; \
		g_assert (FALSE); \
	}

#define KEY_CONTROL_TYPE		"type"
#define KEY_CONTROL_TYPE_KNOB	"knob"
#define KEY_CONTROL_TYPE_POPUP	"popup"
#define KEY_CONTROL_POS_X		"pos_x"
#define KEY_CONTROL_POS_Y		"pos_y"
#define KEY_CONTROL_RESOURCE	"resource"
#define KEY_CONTROL_PARAM_NAME	"param_name"
#define KEY_CONTROL_PARAM_NUM	"param_num"

int deldir (const char *dir_path)
{
	g_assert (dir_path);
	gchar *command = g_strdup_printf("rm -r \"%s\"", dir_path);
	printf ("%s\n", command); return 0;
	int result = system (command);
	g_free (command);
	return result;
}

gchar *extract_skin (char *skin_file)
{
	gchar *tempdir = g_strconcat(g_get_tmp_dir(), "/amsynth.skin.XXXXXXXX", NULL);
	if (!mkdtemp(tempdir)) {
		g_message("Failed to create temporary directory. Unable to load skin.");
		g_free(tempdir);
		return NULL;
	}
	
	gchar *command = g_strdup_printf("/usr/bin/unzip -qq -o -j \"%s\" -d %s", skin_file, tempdir);
	int result = system (command);
	g_free (command);
	command = NULL;
	
	if (result != 0) {
		g_message("Failed to extract archive. Unable to load skin.");
		deldir (tempdir);
		g_free (tempdir);
		tempdir = NULL;
	}
	return tempdir;
}

////////////////////////////////////////////////////////////////////////////////

GtkWidget *
editor_pane_new (GtkAdjustment **adjustments)
{
	GtkWidget *fixed = gtk_fixed_new ();
	gtk_widget_set_usize (fixed, 400, 300);
	
	g_signal_connect (GTK_OBJECT (fixed), "expose-event", (GtkSignalFunc) editor_pane_expose_event_handler, (gpointer) NULL);
	
	gsize i;
	
//	const gchar *skin_path = "/home/nixx/Code/amsynthe-new-gui-1/src/GUI";
	
	gchar *skin_path = extract_skin ("/home/nixx/Code/amsynthe-new-gui-1/src/GUI/default.zip");
	if (skin_path == NULL) {
		g_message ("Could not load skin :-(");
		return fixed;
	}
	
	{
		GData *resdata = NULL;
		GData *resinfo = NULL;
		g_datalist_init (&resdata);
		g_datalist_init (&resinfo);
	
		////////
		
		GError *gerror = NULL;
		GKeyFile *gkey_file = g_key_file_new ();
		gchar *ini_path = g_strconcat (skin_path, "/layout.ini", NULL);
		gboolean ok = g_key_file_load_from_file (gkey_file, ini_path, G_KEY_FILE_NONE, &gerror);
		g_key_file_set_list_separator (gkey_file, ',');
		g_free (ini_path); ini_path = NULL;
		HANDLE_GERROR (gerror);
		g_assert (ok);
		
		////////
		
		{
			gchar *bg_name = g_key_file_get_string  (gkey_file, "layout", "background", &gerror); HANDLE_GERROR (gerror); g_strstrip (bg_name);
			gchar *path = g_strconcat (skin_path, "/", bg_name, NULL);
			editor_pane_bg = gdk_pixbuf_new_from_file (path, &gerror); HANDLE_GERROR (gerror);
			g_assert (editor_pane_bg);
			g_free (bg_name);
			g_free (path);
			
			gtk_widget_set_usize (fixed, gdk_pixbuf_get_width (editor_pane_bg), gdk_pixbuf_get_height (editor_pane_bg));
		}
		
		//// Load resources
		
		gsize num_resources = 0;
		gchar **resource_list = g_key_file_get_string_list (gkey_file, "layout", "resources", &num_resources, &gerror); HANDLE_GERROR (gerror);
		if (resource_list)
		{
			for (i=0; i<num_resources; i++)
			{
				gchar *resource_name = resource_list[i]; g_strstrip (resource_name);
				
				gchar *file = g_key_file_get_string  (gkey_file, resource_name, "file",   &gerror); HANDLE_GERROR (gerror); g_strstrip (file);
				gint width  = g_key_file_get_integer (gkey_file, resource_name, "width",  &gerror); HANDLE_GERROR (gerror);
				gint height = g_key_file_get_integer (gkey_file, resource_name, "height", &gerror); HANDLE_GERROR (gerror);
				gint frames = g_key_file_get_integer (gkey_file, resource_name, "frames", &gerror); HANDLE_GERROR (gerror);
				
				gchar *path = g_strconcat (skin_path, "/", file, NULL);
				printf ("%s\n", path);
				
				GdkPixbuf *pixbuf = gdk_pixbuf_new_from_file (path, &gerror); HANDLE_GERROR (gerror); g_assert (pixbuf);
				
				g_assert (gdk_pixbuf_get_width (pixbuf) == width || gdk_pixbuf_get_height (pixbuf) == height);
				g_assert (gdk_pixbuf_get_width (pixbuf) == (width * frames) || gdk_pixbuf_get_height (pixbuf) == (height * frames));
				
				resource_info *info = g_malloc0 (sizeof (resource_info));
				info->fr_width  = width;
				info->fr_height = height;
				info->fr_count  = frames;
				
				printf ("adding resource '%s'\n", resource_name);
				g_datalist_set_data (&resdata, resource_name, (gpointer)pixbuf);
				g_datalist_set_data (&resinfo, resource_name, (gpointer)info);
				
				g_free (file);
				g_free (path);
			}
			g_strfreev (resource_list);
			resource_list = NULL;
		}
		
		//// Create controls
	
		gsize num_controls = 0;
		gchar **control_list = g_key_file_get_string_list (gkey_file, "layout", "controls", &num_controls, &gerror); HANDLE_GERROR (gerror);
		if (control_list)
		{
			for (i=0; i<num_controls; i++)
			{
				gchar *control_name = control_list[i]; g_strstrip (control_name);
				printf ("adding control '%s'\n", control_name);
				
				gint pos_x = g_key_file_get_integer (gkey_file, control_name, KEY_CONTROL_POS_X, &gerror); HANDLE_GERROR (gerror);
				gint pos_y = g_key_file_get_integer (gkey_file, control_name, KEY_CONTROL_POS_Y, &gerror); HANDLE_GERROR (gerror);
				gchar *resn = g_key_file_get_string (gkey_file, control_name, KEY_CONTROL_RESOURCE, &gerror); HANDLE_GERROR (gerror); g_strstrip (resn);
				gchar *type = g_key_file_get_string (gkey_file, control_name, KEY_CONTROL_TYPE, &gerror); HANDLE_GERROR (gerror); g_strstrip (type);
				gchar *param_name = g_key_file_get_string (gkey_file, control_name, KEY_CONTROL_PARAM_NAME, &gerror); HANDLE_GERROR (gerror);
				const gint param_num = parameter_index_from_name (param_name);
				g_strstrip (param_name);
				
				/////////////////////////
								
				GtkWidget *widget = NULL;
				printf ("using resource '%s'\n", resn);
				resource_info *res = g_datalist_get_data (&resinfo, resn); g_assert (res);
				GdkPixbuf *frames = GDK_PIXBUF (g_datalist_get_data (&resdata, resn)); g_assert (frames);
				GdkPixbuf *subpixpuf = gdk_pixbuf_new_subpixbuf (editor_pane_bg, pos_x, pos_y, res->fr_width, res->fr_height);
				GtkAdjustment *adj = adjustments[param_num];
				
				if (g_strcmp0 (KEY_CONTROL_TYPE_KNOB, type) == 0)
				{
					widget = bitmap_knob_new (adj, frames, res->fr_width, res->fr_height, res->fr_count);
					bitmap_knob_set_bg (widget, subpixpuf);
				}
				else if (g_strcmp0 (KEY_CONTROL_TYPE_POPUP, type) == 0)
				{
					gsize nstrings = 0;
					gchar **strings = g_key_file_get_string_list (gkey_file, control_name, "strings", &nstrings, &gerror); HANDLE_GERROR (gerror);
					widget = bitmap_popup_new (adj, frames, res->fr_width, res->fr_height, res->fr_count);
					bitmap_popup_set_strings (widget, (const char **)strings);	
					bitmap_popup_set_bg (widget, subpixpuf);
					g_strfreev (strings);
				}
				
				g_object_unref (G_OBJECT (subpixpuf));
				gtk_fixed_put (GTK_FIXED (fixed), widget, pos_x, pos_y);
				
				g_free (resn);
				g_free (type);
				g_free (param_name);
			}
			g_strfreev (control_list);
			control_list = NULL;
		}
	
		g_key_file_free (gkey_file);
	}
	
	deldir (skin_path);
	g_free (skin_path);

	return fixed;
}

////////////////////////////////////////////////////////////////////////////////

