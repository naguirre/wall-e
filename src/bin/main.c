#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <Ecore.h>
#include <Ecore_Evas.h>
#include <Ecore_File.h>
#include <Edje.h>

#include <Elementary.h>

#include "wall-e.h"
#include "wall.h"

static void _walle_wall_changed_size_hints(void *data, Evas *e, Evas_Object *obj, void *einfo)
{
  Walle *walle;
  Evas_Coord w,h;

  walle = data;

  evas_object_geometry_get(walle->o_wall, NULL, NULL, NULL, &h);
  evas_object_size_hint_min_get(walle->o_wall, &w, NULL);
  evas_object_resize(walle->o_wall, w, h);
}

int
main(int argc, char *argv[])
{
    Walle *walle;

    if (!ecore_evas_init())
        return EXIT_FAILURE;

    if (!ecore_file_init())
        goto shutdown_ecore_evas;

    if (!edje_init())
        goto shutdown_ecore_file;

    elm_init(argc, argv);

    walle = walle_init(argc, argv);
    if (!walle)
        goto shutdown_elm;

    if (!walle_gui_set(walle))
        goto shutdown_walle;

    elm_run();

    walle_shutdown(walle);
    elm_shutdown();
    edje_shutdown();
    ecore_file_shutdown();
    ecore_evas_shutdown();

    return EXIT_SUCCESS;

 shutdown_walle:
    walle_shutdown(walle);
 shutdown_elm:
    elm_shutdown();
 shutdown_edje:
    edje_shutdown();
 shutdown_ecore_file:
    ecore_file_shutdown();
 shutdown_ecore_evas:
    ecore_evas_shutdown();

    return EXIT_FAILURE;
}
