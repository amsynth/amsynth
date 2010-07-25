////////////////////////////////////////////////////////////////////////////////

#include "editor_pane.h"
#include "../controls.h"
#include "bitmap_button.h"
#include "bitmap_knob.h"
#include "bitmap_popup.h"

#define ENABLE_LAYOUT_EDIT 1

////////////////////////////////////////////////////////////////////////////////

#define SIZEOF_ARRAY( a ) ( sizeof((a)) / sizeof((a)[0]) )

////////////////////////////////////////////////////////////////////////////////

static GdkPixbuf *editor_pane_bg = NULL;

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
		gdk_pixbuf_get_width (editor_pane_bg),
		gdk_pixbuf_get_height (editor_pane_bg),
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
#define KEY_CONTROL_TYPE_BUTTON	"button"
#define KEY_CONTROL_TYPE_KNOB	"knob"
#define KEY_CONTROL_TYPE_POPUP	"popup"
#define KEY_CONTROL_POS_X		"pos_x"
#define KEY_CONTROL_POS_Y		"pos_y"
#define KEY_CONTROL_RESOURCE	"resource"
#define KEY_CONTROL_PARAM_NAME	"param_name"
#define KEY_CONTROL_PARAM_NUM	"param_num"

int deldir (const char *dir_path)
{
//	g_assert (dir_path);
//	gchar *command = g_strdup_printf("rm -r \"%s\"", dir_path);
//	int result = system (command);
//	g_free (command);
//	return result;
	return 0;
}

gchar *extract_skin (char *skin_file)
{
	gchar *tempdir = g_strconcat(g_get_tmp_dir(), "/amsynth.skin.XXXXXXXX", NULL);
	if (!mkdtemp(tempdir)) {
		g_message("Failed to create temporary directory. Unable to load skin.");
		g_free(tempdir);
		return NULL;
	}
	
	gchar *unzip_bin = "/usr/bin/unzip";
	gchar *command = g_strdup_printf("%s -qq -o -j \"%s\" -d %s", unzip_bin, skin_file, tempdir);
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

#if ENABLE_LAYOUT_EDIT /////////////////////////////////////////////////////////

static gboolean
control_move_handler (GtkWidget *widget, GdkEventMotion *event)
{
	static GtkWidget *current_widget = NULL;
	static gint drag_offset_x = 0;
	static gint drag_offset_y = 0;
	
	if (!widget || !event)
		return FALSE;
	
	if (event->type == GDK_MOTION_NOTIFY && event->state & GDK_BUTTON2_MASK)
	{
		if (current_widget != widget) {
			// remember where the drag operation began
			current_widget = widget;
			drag_offset_x = event->x;
			drag_offset_y = event->y;
			return TRUE;
		}
		GtkWidget *parent = gtk_widget_get_parent (widget);
		gtk_fixed_move (GTK_FIXED (parent), widget,
			widget->allocation.x - parent->allocation.x + event->x - drag_offset_x,
			widget->allocation.y - parent->allocation.y + event->y - drag_offset_y);
		return TRUE;
	}
    return FALSE;
}

static void
foreach_containter_widget (GtkWidget *widget, gpointer data)
{
	GtkWidget *parent = GTK_WIDGET (data);
	
	const gchar *name = gtk_buildable_get_name (GTK_BUILDABLE (widget));

	gchar *type = NULL, *resname = NULL;
	g_object_get (G_OBJECT (widget),
		"tooltip-text", &type,
		"name", &resname,
		NULL);
	
	printf (
		"[%s]\n"
		"resource=%s\n"
		"type=%s\n"
		"pos_x=%d\n"
		"pos_y=%d\n",
		name, resname, type,
		widget->allocation.x - parent->allocation.x,
		widget->allocation.y - parent->allocation.y);
	
	printf ("\n");
}

static gboolean
on_unrealize (GtkWidget *widget, gpointer user_data)
{
	GtkContainer *container = GTK_CONTAINER (widget);
	gtk_container_foreach (container, foreach_containter_widget, container);
	return FALSE;
}

#endif /////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////

GtkWidget *
editor_pane_new (GtkAdjustment **adjustments)
{
	GtkWidget *fixed = gtk_fixed_new ();
	gtk_widget_set_usize (fixed, 400, 300);
	
	g_signal_connect (GTK_OBJECT (fixed), "expose-event", (GtkSignalFunc) editor_pane_expose_event_handler, (gpointer) NULL);
	
#if ENABLE_LAYOUT_EDIT
	g_signal_connect (GTK_OBJECT (fixed), "unrealize", (GtkSignalFunc) on_unrealize, (gpointer) NULL);
#endif
	
	gsize i;
	gchar *skin_dir = NULL;
	gchar *skin_path = (gchar *)g_getenv ("AMSYNTH_SKIN");
	if (skin_path == NULL) {
		skin_path = "amsynth-skin.zip";
	}
	
	if (!g_file_test (skin_path, G_FILE_TEST_EXISTS)) {
		g_error ("cannot find skin '%s'", skin_path);
		return fixed;
	}
	
	if (g_file_test (skin_path, G_FILE_TEST_IS_DIR)) {
		skin_dir = g_strdup (skin_path);
	}
	
	if (g_file_test (skin_path, G_FILE_TEST_IS_REGULAR)) {
		skin_dir = extract_skin (skin_path);
		if (skin_dir == NULL) {
			g_message ("Could not unpack skin file '%s'", skin_path);
			return fixed;
		}
	}
	
	{
		GData *resdata = NULL;
		GData *resinfo = NULL;
		g_datalist_init (&resdata);
		g_datalist_init (&resinfo);
	
		////////
		
		GError *gerror = NULL;
		GKeyFile *gkey_file = g_key_file_new ();
		gchar *ini_path = g_strconcat (skin_dir, "/layout.ini", NULL);
		gboolean ok = g_key_file_load_from_file (gkey_file, ini_path, G_KEY_FILE_NONE, &gerror);
		g_key_file_set_list_separator (gkey_file, ',');
		g_free (ini_path); ini_path = NULL;
		HANDLE_GERROR (gerror);
		g_assert (ok);
		
		////////
		
		{
			gchar *bg_name = g_key_file_get_string  (gkey_file, "layout", "background", &gerror); HANDLE_GERROR (gerror); g_strstrip (bg_name);
			gchar *path = g_strconcat (skin_dir, "/", bg_name, NULL);
			editor_pane_bg = gdk_pixbuf_new_from_file (path, &gerror); HANDLE_GERROR (gerror);
			g_assert (editor_pane_bg);
			g_free (bg_name);
			g_free (path);
			
			gtk_widget_set_size_request (fixed, gdk_pixbuf_get_width (editor_pane_bg), gdk_pixbuf_get_height (editor_pane_bg));
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
				
				gchar *path = g_strconcat (skin_dir, "/", file, NULL);
				
				GdkPixbuf *pixbuf = gdk_pixbuf_new_from_file (path, &gerror); HANDLE_GERROR (gerror); g_assert (pixbuf);
				
				g_assert (gdk_pixbuf_get_width (pixbuf) == width || gdk_pixbuf_get_height (pixbuf) == height);
				g_assert (gdk_pixbuf_get_width (pixbuf) == (width * frames) || gdk_pixbuf_get_height (pixbuf) == (height * frames));
				
				resource_info *info = g_malloc0 (sizeof (resource_info));
				info->fr_width  = width;
				info->fr_height = height;
				info->fr_count  = frames;
				
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
				
				gint pos_x = g_key_file_get_integer (gkey_file, control_name, KEY_CONTROL_POS_X, &gerror); HANDLE_GERROR (gerror);
				gint pos_y = g_key_file_get_integer (gkey_file, control_name, KEY_CONTROL_POS_Y, &gerror); HANDLE_GERROR (gerror);
				gchar *resn = g_key_file_get_string (gkey_file, control_name, KEY_CONTROL_RESOURCE, &gerror); HANDLE_GERROR (gerror); g_strstrip (resn);
				gchar *type = g_key_file_get_string (gkey_file, control_name, KEY_CONTROL_TYPE, &gerror); HANDLE_GERROR (gerror); g_strstrip (type);
				gchar *param_name = g_key_file_get_string (gkey_file, control_name, KEY_CONTROL_PARAM_NAME, &gerror); HANDLE_GERROR (gerror);
				const gint param_num = parameter_index_from_name (param_name);
				g_strstrip (param_name);
				
				/////////////////////////
								
				GtkWidget *widget = NULL;
				resource_info *res = g_datalist_get_data (&resinfo, resn); g_assert (res);
				GdkPixbuf *frames = GDK_PIXBUF (g_datalist_get_data (&resdata, resn)); g_assert (frames);
				GdkPixbuf *subpixpuf = gdk_pixbuf_new_subpixbuf (editor_pane_bg, pos_x, pos_y, res->fr_width, res->fr_height);
				GtkAdjustment *adj = adjustments[param_num];
				
				if (g_strcmp0 (KEY_CONTROL_TYPE_KNOB, type) == 0)
				{
					widget = bitmap_knob_new (adj, frames, res->fr_width, res->fr_height, res->fr_count);
					bitmap_knob_set_bg (widget, subpixpuf);
				}
				else if (g_strcmp0 (KEY_CONTROL_TYPE_BUTTON, type) == 0)
				{
					widget = bitmap_button_new (adj, frames, res->fr_width, res->fr_height, res->fr_count);
					bitmap_button_set_bg (widget, subpixpuf);
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
				
#if ENABLE_LAYOUT_EDIT
				gtk_buildable_set_name (GTK_BUILDABLE (widget), control_name);
				g_object_set (G_OBJECT (widget), "name", resn, "tooltip-text", type, NULL);
			    g_signal_connect(widget, "motion-notify-event", G_CALLBACK(control_move_handler), NULL);
				gtk_widget_add_events(widget, GDK_BUTTON2_MOTION_MASK);
#endif
				
				g_free (resn);
				g_free (type);
				g_free (param_name);
			}
			g_strfreev (control_list);
			control_list = NULL;
		}
	
		g_key_file_free (gkey_file);
	}
	
	//deldir (skin_dir);
	g_free (skin_dir);

	return fixed;
}

////////////////////////////////////////////////////////////////////////////////

