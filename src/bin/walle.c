#ifdef HAVE_CONFIG_H
# include <config.h>
#endif


#include <stdlib.h>

#include <Ecore_Getopt.h>
#include <Ecore_File.h>

#include <Elementary.h>

#include "wall-e.h"
#include "wall.h"

static const Ecore_Getopt options = {
    "wall-e",
    "%prog [options]",
    "0.1.0",
    "(C) 2008 Enlightenment",
    "BSD\nThis is a 3 clause bsd bla bla",
    "a simple image viewer using enlightenment foundation libraries.\n"
    "Here is the text of the licence",
    1,
    {
        ECORE_GETOPT_STORE_STR('e', "engine", "ecore-evas engine to use"),
        ECORE_GETOPT_CALLBACK_NOARGS('E', "list-engines", "list ecore-evas engines",
                                     ecore_getopt_callback_ecore_evas_list_engines, NULL),
        ECORE_GETOPT_STORE_DEF_BOOL('F', "fullscreen", "fullscreen mode", 0),
        ECORE_GETOPT_CALLBACK_ARGS('g', "geometry", "geometry to use in x:y:w:h form.", "X:Y:W:H",
                                   ecore_getopt_callback_geometry_parse, NULL),
        ECORE_GETOPT_STORE_STR('p', "path", "path to read the image files from"),
        ECORE_GETOPT_STORE_STR('t', "theme", "path to read the theme file from"),
        ECORE_GETOPT_STORE_INT('f', "fps", "frame per second"),
        ECORE_GETOPT_STORE_INT('l', "lines", "Number of lines to display"),
        ECORE_GETOPT_COUNT('v', "verbose", "be more verbose"),
        ECORE_GETOPT_VERSION('V', "version"),
        ECORE_GETOPT_COPYRIGHT('R', "copyright"),
        ECORE_GETOPT_LICENSE('L', "license"),
        ECORE_GETOPT_HELP('h', "help"),
        ECORE_GETOPT_SENTINEL
    }
};

static void _walle_win_del_cb(void *data, Evas_Object *obj, void *event_info)
{
   elm_exit();
}

static void _walle_picture_cb_resize(void *data, Evas *e, Evas_Object *obj, void *einfo)
{
    Evas_Object *o_pict;
    Evas_Coord   width;
    Evas_Coord   height;

    o_pict = (Evas_Object *)data;
    width = 0;
    height = 0;
    /* callback when a picture is resized */
    evas_object_geometry_get(o_pict, NULL, NULL, &width, &height);
    evas_object_image_fill_set(o_pict, 0, 0, width, height);
}

static void _walle_wall_cb_resize(void *data, Evas *e, Evas_Object *obj, void *einfo)
{
    Walle *walle = data;
    Evas_Coord ow, oh;

    evas_object_geometry_get(walle->o_scroll, NULL, NULL, &ow, &oh);
    evas_object_size_hint_min_set(walle->o_wall, ow, oh);
}

static void _walle_wall_populate(Walle *walle, const char *directory, const char *theme)
{
    Evas_Object *o;
    Evas_Coord ow, oh, w, h;
    Eina_List *files = NULL;
    char *file = NULL;
    char filename[4096];

    if (!ecore_file_is_dir(directory) && ecore_file_dir_is_empty(directory))
        return;
    files = ecore_file_ls(directory);
    if (!files)
        return;
    EINA_LIST_FREE(files, file)
    {
    	sprintf(filename, "%s/%s", directory, file);
    	o = evas_object_image_add(walle->evas);
    	evas_object_image_file_set(o, filename, NULL);
    	switch(evas_object_image_load_error_get(o))
    	{
    	case EVAS_LOAD_ERROR_NONE:
    	{
    	    evas_object_event_callback_add(o, EVAS_CALLBACK_RESIZE, _walle_picture_cb_resize, o);
    	    evas_object_image_size_get(o, &w, &h);
    	    oh = 200;
    	    ow = oh * (float)w/(float)h;
    	    evas_object_image_load_size_set(o, ow, oh);
    	    evas_object_image_fill_set(o, 0, 0, ow, oh);
    	    evas_object_resize(o, ow, oh);
    	    evas_object_size_hint_min_set(o, ow, oh);
    	    walle_wall_append(walle->o_wall, o);
    	    walle->pictures = eina_list_append(walle->pictures, o);
    	}
    	break;
    	default:
    	    evas_object_del(o);
    	    break;
    	}
    }
}

static void
_walle_monitor_cb(void *data,
                  Ecore_File_Monitor *em,
                  Ecore_File_Event event,
                  const char *path)
{
    Walle *walle;

    walle = (Walle *)data;

    if (event == ECORE_FILE_EVENT_CREATED_FILE)
    {
        Evas_Object *o;
        Evas_Coord ow, oh, w, h;

	o = evas_object_image_add(walle->evas);
	evas_object_image_file_set(o, path, NULL);
	switch(evas_object_image_load_error_get(o))
	{
	case EVAS_LOAD_ERROR_NONE:
	{
	    evas_object_event_callback_add(o, EVAS_CALLBACK_RESIZE, _walle_picture_cb_resize, o);
	    evas_object_image_size_get(o, &w, &h);
	    oh = 200;
	    ow = oh * (float)w/(float)h;
	    evas_object_image_load_size_set(o, ow, oh);
	    evas_object_image_fill_set(o, 0, 0, ow, oh);
	    evas_object_resize(o, ow, oh);
	    evas_object_size_hint_min_set(o, ow, oh);
	    walle_wall_append(walle->o_wall, o);
	    walle->pictures = eina_list_append(walle->pictures, o);
	}
	break;
	default:
	    evas_object_del(o);
	    break;

	}
    }
}


Walle *
walle_init(int argc, char **argv)
{
    Eina_Rectangle     geometry = {0, 0, 0, 0};
    char              *engine = NULL;
    char              *path = NULL;
    char              *theme = NULL;
    int                fps = 0;
    int                lines = 2;
    int                verbose = 0;
    int                args;
    unsigned char      help = 0;
    unsigned char      engines_listed = 0;
    unsigned char      is_fullscreen = 0;
    Ecore_Getopt_Value values[] = {
        ECORE_GETOPT_VALUE_STR(engine),
        ECORE_GETOPT_VALUE_BOOL(engines_listed),
        ECORE_GETOPT_VALUE_BOOL(is_fullscreen),
        ECORE_GETOPT_VALUE_PTR_CAST(geometry),
        ECORE_GETOPT_VALUE_STR(path),
        ECORE_GETOPT_VALUE_STR(theme),
        ECORE_GETOPT_VALUE_INT(fps),
        ECORE_GETOPT_VALUE_INT(lines),
        ECORE_GETOPT_VALUE_INT(verbose),
        ECORE_GETOPT_VALUE_NONE,
        ECORE_GETOPT_VALUE_NONE,
        ECORE_GETOPT_VALUE_NONE,
        ECORE_GETOPT_VALUE_BOOL(help),
        ECORE_GETOPT_VALUE_NONE
    };

    Walle             *walle;

    ecore_app_args_set(argc, (const char **)argv);

    args = ecore_getopt_parse(&options, values, argc, argv);
    if (args < 0)
    {
        fprintf(stderr, "[Wall-e] ERROR: could not parse options.\n");
        return NULL;
    }

    /* parse the options */

    if (help)
        return NULL;

    if (engines_listed)
        return NULL;

    if (!engine)
    {
        fprintf(stderr, "[Wall-e] ERROR: could not set engine.\n");
        return NULL;
    }

    if (!path)
    {
        fprintf(stderr, "[Wall-e] ERROR: could not set path.\n");
        return NULL;
    }

    if (!theme)
    {
        theme = getenv("WALLE_THEME");
        if (!theme)
            theme = PACKAGE_DATA_DIR "/data/default.edj";
    }

    if (is_fullscreen &&
        ((geometry.x != 0) ||
         (geometry.y != 0) ||
         (geometry.w != 0) ||
         (geometry.h != 0)))
    {
        fprintf(stderr, "[Wall-e] ERROR: could not set fulllscreen and geometry at same time.\n");
        return NULL;
    }

    if (!is_fullscreen &&
        (geometry.x == 0) &&
        (geometry.y == 0) &&
        (geometry.w == 0) &&
        (geometry.h == 0))
    {
        geometry.w = 640;
        geometry.h = 480;
    }

    if (fps != 0)
        ecore_animator_frametime_set(1.0 / (double)fps);

    /* we set our Walle object */

    walle = (Walle *)calloc(1, sizeof(Walle));
    if (!walle)
	return NULL;

    /* config */
    walle->config.engine = strdup(engine);
    walle->config.path = strdup(path);
    walle->config.theme = strdup(theme);
    walle->config.width = geometry.w;
    walle->config.height = geometry.h;
    walle->config.fps = fps;
    walle->config.lines = lines;

    return walle;
}

void
walle_shutdown(Walle *walle)
{
    if (!walle)
        return;

    free(walle->config.engine);
    free(walle->config.path);
    free(walle->config.theme);
    ecore_file_monitor_del(walle->monitor);
    free(walle);
}

int
walle_gui_set(Walle *walle)
{
    Evas_Object       *o;

    /* monitoring */
    walle->monitor = ecore_file_monitor_add(walle->config.path,
                                            _walle_monitor_cb,
                                            walle);
    if (!walle->monitor)
        return 0;

    /* main window */
    o = elm_win_add(NULL, "Wall-E", ELM_WIN_BASIC);
    if (!o)
    {
        ecore_file_monitor_del(walle->monitor);
        return 0;
    }

    elm_win_title_set(o, "Wall-E");
    evas_object_smart_callback_add(o, "delete-request", _walle_win_del_cb, NULL);
    walle->o_win = o;

    walle->evas = evas_object_evas_get(walle->o_win);

    /* main edje object */
    o = edje_object_add(walle->evas);
    edje_object_file_set(o, walle->config.theme, "wall-e");
    evas_object_size_hint_weight_set(o, 1.0, 1.0);
    elm_win_resize_object_add(walle->o_win, o);
    evas_object_show(o);
    walle->o_edje = o;

    /* scroller */
    o = elm_scroller_add(walle->o_win);
    edje_object_part_swallow(walle->o_edje, "walle.swallow.content", o);
    evas_object_show(o);
    walle->o_scroll = o;

    /* toolbar */
    o = elm_toolbar_add(walle->o_win);
    evas_object_size_hint_weight_set(o, 0.0, 0.0);
    evas_object_size_hint_align_set(o, -1.0, 0.0);
    evas_object_size_hint_min_set(o, -1, -1);
    edje_object_part_swallow(walle->o_edje, "walle.swallow.toolbar", o);
    walle->o_toolbar = o;

    elm_toolbar_item_append(walle->o_toolbar, "walle/icon/resize", "Resize", NULL, NULL);
    elm_toolbar_item_append(walle->o_toolbar, "walle/icon/rotate", "Rotate", NULL, NULL);
    elm_toolbar_item_append(walle->o_toolbar, "walle/icon/config", "Configuration", NULL, NULL);

    /* another brick in the wall */
    o = walle_wall_add(walle->evas);
    walle_wall_lines_set(o, walle->config.lines);
    elm_object_content_set(walle->o_scroll, o);
    evas_object_event_callback_add(walle->o_scroll, EVAS_CALLBACK_RESIZE, _walle_wall_cb_resize, walle);
    walle->o_wall = o;

    _walle_wall_populate(walle, walle->config.path, walle->config.theme);

    /* resize and show window */
    evas_object_resize(walle->o_win, walle->config.width, walle->config.height);
    evas_object_show(walle->o_win);

    return 1;
}
