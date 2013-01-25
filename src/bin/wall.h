#ifndef _WALLE_WALL_H
#define _WALLE_WALL_H

#include <Evas.h>

Evas_Object *walle_wall_add(Evas * evas);
Evas_Object *walle_wall_add_to(Evas_Object *parent);
void walle_wall_append(Evas_Object *o, Evas_Object *child);
void         walle_wall_vertical_set(Evas_Object *obj, Eina_Bool vertival);
void         walle_wall_lines_set(Evas_Object *obj, int nb_lines);

#endif
