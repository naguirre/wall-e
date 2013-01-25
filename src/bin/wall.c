#include <stdio.h>

#include <Edje.h>
#include <Evas.h>

#include <Elementary.h>

typedef struct _Walle_Wall_Data Walle_Wall_Data;
struct _Walle_Wall_Data
{
    Evas_Object_Smart_Clipped_Data  base;
    const Evas_Smart_Class         *api;
    Evas_Object                    *o_smart;
    Evas_Object                    *o_box;
    Eina_List                      *o_boxes;
    Eina_List                      *items;
    unsigned int                    nb_lines;
    unsigned int                    nb_items;
    int                             row_sel;
    unsigned char                   vertical : 1;

};

typedef struct _Walle_Picture_Item Walle_Picture_Item;
struct _Walle_Picture_Item
{
    Evas_Object     *o_edje;
    Evas_Object     *o_pict;
    Walle_Wall_Data *priv;
    int              row;
    unsigned char    selected : 1;
};

#define WALLE_WALL_DATA_GET(o, ptr)				\
    Walle_Wall_Data *ptr = evas_object_smart_data_get(o)

#define WALLE_WALL_DATA_GET_OR_RETURN(o, ptr)                           \
    WALLE_WALL_DATA_GET(o, ptr);					\
    if (!ptr)								\
    {									\
	fprintf(stderr, "CRITICAL: no widget data for object %p (%s)\n", \
	    o, evas_object_type_get(o));				\
	fflush(stderr);							\
	abort();							\
	return;								\
    }

#define WALLE_WALL_DATA_GET_OR_RETURN_VAL(o, ptr, val)                  \
    WALLE_WALL_DATA_GET(o, ptr);					\
    if (!ptr)								\
    {									\
	fprintf(stderr, "CRITICAL: no widget data for object %p (%s)\n", \
	    o, evas_object_type_get(o));				\
	fflush(stderr);							\
	abort();							\
	return val;							\
    }

static Evas_Smart_Class _parent_sc = {NULL};

static void _sizing_eval(Evas_Object *obj);
static void _changed_size_hints(void *data, Evas *e, Evas_Object *obj, void *event_info);
static void _wall_smart_add(Evas_Object *o);

static void
_changed_size_hints(void *data, Evas *e, Evas_Object *obj, void *event_info)
{
    _sizing_eval(data);
}

static void
_sizing_eval(Evas_Object *obj)
{
    Walle_Wall_Data *priv;
    Evas_Coord minw = -1, minh = -1, maxw = -1, maxh = -1;
    Evas_Coord w, h;

    priv = evas_object_smart_data_get(obj);

    if (!priv) return;

    evas_object_size_hint_min_get(priv->o_box, &minw, &minh);
    evas_object_size_hint_max_get(priv->o_box, &maxw, &maxh);
    evas_object_size_hint_min_set(obj, minw, minh);
    evas_object_size_hint_max_set(obj, maxw, maxh);
    evas_object_geometry_get(obj, NULL, NULL, &w, &h);
    if (w < minw) w = minw;
    if (h < minh) h = minh;
    if ((maxw >= 0) && (w > maxw)) w = maxw;
    if ((maxh >= 0) && (h > maxh)) h = maxh;
    evas_object_resize(obj, w, h);
}

static void
_wall_smart_add(Evas_Object *o)
{
    Walle_Wall_Data *priv;
    Evas_Object *o_box;

    priv = evas_object_smart_data_get(o);
    if (!priv)
    {
        const Evas_Smart *smart;
        const Evas_Smart_Class *sc;

	priv = calloc(1, sizeof(*priv));
	if (!priv)
	{
	    fputs("ERROR: could not allocate object private data.\n", stderr);
	    return;
	}

        smart = evas_object_smart_smart_get(o);
        sc = evas_smart_class_get(smart);
        priv->api = (const Evas_Smart_Class *)sc;

	evas_object_smart_data_set(o, priv);
    }
    _parent_sc.add(o);

    priv->nb_lines = 1;
    priv->o_smart = o;

    priv->o_box = elm_box_add(o);
    elm_box_horizontal_set(priv->o_box, 0);

    evas_object_show(priv->o_box);

    o_box = elm_box_add(o);
    elm_box_horizontal_set(o_box, 1);
    evas_object_size_hint_align_set(o_box, 0, 0.5);
    //evas_object_size_hint_weight_set(o_box, 1.0, 1.0);

    elm_box_pack_end(priv->o_box, o_box);
    priv->o_boxes = eina_list_append(priv->o_boxes, o_box);
    evas_object_show(o_box);
    priv->vertical = 0;

    evas_object_event_callback_add(priv->o_box, EVAS_CALLBACK_CHANGED_SIZE_HINTS,
				   _changed_size_hints, o);

    evas_object_smart_member_add(priv->o_box, o);
}

void
walle_wall_vertical_set(Evas_Object *obj, Eina_Bool vertical)
{
    WALLE_WALL_DATA_GET_OR_RETURN(obj, priv);

    if (vertical == priv->vertical)
    {
	return;
    }
    else if (vertical)
    {
	evas_object_box_layout_set(priv->o_box, evas_object_box_layout_flow_vertical, NULL, NULL);
	priv->vertical = 1;
    }
    else
    {
	evas_object_box_layout_set(priv->o_box, evas_object_box_layout_flow_horizontal, NULL, NULL);
	priv->vertical = 0;
    }

}

void
walle_wall_lines_set(Evas_Object *obj, int nb_lines)
{
    Eina_List   *l;
    Evas_Object *o_box;
    int          i;

    WALLE_WALL_DATA_GET_OR_RETURN(obj, priv);

    if (priv->nb_lines == nb_lines)
	return;

    priv->nb_lines = nb_lines;

    while (priv->o_boxes)
    {
        o_box = (Evas_Object *)eina_list_data_get(priv->o_boxes);
	evas_object_del(o_box);
	priv->o_boxes = eina_list_remove_list(priv->o_boxes, priv->o_boxes);
    }

    /* FIXME: should we destroy the list too ? */
    priv->o_boxes = NULL;

    for (i = 0; i < nb_lines; i++)
    {
	o_box = elm_box_add(obj);
	elm_box_horizontal_set(o_box, 1);
	evas_object_size_hint_align_set(o_box, 0, 0.5);
	evas_object_size_hint_weight_set(o_box, 1.0, 1.0);
	elm_box_pack_end(priv->o_box, o_box);
	priv->o_boxes = eina_list_append(priv->o_boxes, o_box);
	evas_object_show(o_box);
    }
}


static Walle_Picture_Item *_wall_selected_item_get( Walle_Wall_Data *priv, int *row, int *col)
{
    Eina_List *l1;
    Eina_List *items1;
    int i;
    int j;

    i = 0;
    EINA_LIST_FOREACH(priv->items, l1, items1)
    {
        Eina_List *l2;
        Eina_List *items2;

        j = 0;
        EINA_LIST_FOREACH(items1, l2, items2)
        {
	    Walle_Picture_Item *pi;

            pi = (Walle_Picture_Item *)items2;
            if (pi->selected)
            {
                if (row)
                    *row = i;
                if (col)
                    *col = j;
                return pi;
            }
            j++;
        }
        i++;
    }
    if (row)
        *row = -1;
    if (col)
        *col = -1;

    return NULL;
}

static void _wall_item_unselect(Walle_Wall_Data *priv, Walle_Picture_Item *pi)
{
    if (!pi || !priv)
        return;
    pi->selected = 0;
    edje_object_signal_emit(pi->o_edje, "unselect", "walle");
}

static void _wall_item_select(Walle_Wall_Data *priv, Walle_Picture_Item *pi)
{
    if (!pi)
        return;

    pi->selected = 1;
    evas_object_raise(pi->o_edje);
    evas_object_raise(eina_list_nth(pi->priv->o_boxes, pi->row));

    edje_object_signal_emit(pi->o_edje, "select", "walle");

    priv->row_sel = pi->row;
}

static void
_wall_event_mouse_down_cb(void *data, Evas *evas, Evas_Object *obj, void *event_info)
{
    Walle_Picture_Item *pi, *ppi;
    int row, col;

    pi = data;

    ppi = _wall_selected_item_get(pi->priv, &row, &col);
    if (ppi && ppi != pi)
    {
        ppi->selected = 0;
	_wall_item_unselect(pi->priv, ppi);
    }
    else if (ppi == pi)
    {
	/* Item already selected send activate callback */
	evas_object_smart_callback_call (pi->priv->o_smart, "activate", pi);
        return;
    }

    evas_object_raise(eina_list_nth(pi->priv->o_boxes,pi->row));
    evas_object_raise(pi->o_edje);
    pi->selected = 1;
    pi->priv->row_sel = pi->row;
    _wall_item_select(pi->priv, pi);
}

static void
_walle_event_image_preload_cb(void *data, Evas *evas, Evas_Object *obj, void *event_info)
{
    Walle_Picture_Item *pi = data;

    if (!pi) return;

    edje_object_part_swallow(pi->o_edje, "walle.swallow.content", pi->o_pict);
    evas_object_show(pi->o_pict);
    edje_object_signal_emit(pi->o_edje, "walle,loading,stop", "walle");
}

static void
_wall_smart_resize(Evas_Object *o, Evas_Coord w, Evas_Coord h)
{
    Eina_List *l;
    Evas_Object *o_box;

    WALLE_WALL_DATA_GET(o, priv);

    EINA_LIST_FOREACH(priv->o_boxes, l, o_box)
    {
	evas_object_resize(o_box, w, h/priv->nb_lines);
    }
    evas_object_resize(priv->o_box, w, h);
    evas_object_size_hint_min_get(priv->o_box, &w, &h);
    evas_object_size_hint_min_set(o, w, h);
}

static void
_wall_smart_move(Evas_Object *o, Evas_Coord x, Evas_Coord y)
{
    WALLE_WALL_DATA_GET(o, priv);
    evas_object_move(priv->o_box, x, y);
}

static void
_wall_smart_del(Evas_Object *o)
{
    Evas_Object *box;
    Eina_List *l;

    WALLE_WALL_DATA_GET(o, priv);

    evas_object_del(priv->o_box);
    EINA_LIST_FOREACH(priv->o_boxes, l, box)
    {
        evas_object_del(box);
    }

    while(priv->items)
    {
        Walle_Picture_Item *pi;
        Eina_List *items;

        items = priv->items->data;
        priv->items = eina_list_remove_list(priv->items, priv->items);
        while(items)
        {
            pi = items->data;
            items = eina_list_remove_list(items, items);
            evas_object_del(pi->o_edje);
            evas_object_image_preload(pi->o_pict, 1);
            evas_object_del(pi->o_pict);
            free(pi);
        }
        eina_list_free(items);
    }
    eina_list_free(priv->items);
    free(priv);
}

static Evas_Smart *
_wall_smart_class_new(void)
{
    static Evas_Smart_Class api = {"Walle_Wall", EVAS_SMART_CLASS_VERSION};

    if (!_parent_sc.name)
    {
	evas_object_smart_clipped_smart_set(&_parent_sc);
	api.add = _wall_smart_add;
	api.resize = _wall_smart_resize;
	api.move = _wall_smart_move;
	api.del = _wall_smart_del;
    }

    return evas_smart_class_new(&api);
}

Evas_Object *
walle_wall_add(Evas *evas)
{
    static Evas_Smart *smart = NULL;
    Evas_Object *o;

    if (!smart)
	smart = _wall_smart_class_new();

    o = evas_object_smart_add(evas, smart);
    return o;
}



void
walle_wall_append(Evas_Object *o, Evas_Object *child)
{

    int n;
    Evas_Object *o_box;
    Walle_Picture_Item *pi;
    Evas_Coord ow, oh;
    Eina_List *items = NULL;

    WALLE_WALL_DATA_GET_OR_RETURN(o, priv);

    pi = calloc(1, sizeof(Walle_Picture_Item));
    pi->priv = priv;

    pi->o_edje = edje_object_add(evas_object_evas_get(o));

    /* FIXME change this to retrieve theme filename from config option */
    edje_object_file_set(pi->o_edje, PACKAGE_DATA_DIR"/data/default.edj", "walle/picture/item");
    evas_object_show(pi->o_edje);
    edje_object_signal_emit(pi->o_edje, "walle,loading,start", "walle");

    pi->o_pict = child;
    evas_object_image_preload(pi->o_pict, 0);
    evas_object_geometry_get(child, NULL, NULL, &ow, &oh);
    edje_extern_object_min_size_set(pi->o_edje, ow, oh);
    evas_object_size_hint_min_set(child, 0, 0);

    n = priv->nb_items % priv->nb_lines;
    pi->row = n;
    o_box = eina_list_nth(priv->o_boxes, n);
    elm_box_pack_end(o_box, pi->o_edje);
    items = eina_list_nth(priv->items, n);
    if (!items)
    {
	items = eina_list_append(items, pi);
	priv->items = eina_list_append(priv->items, items);
    }
    else
    {
	items = eina_list_append(items, pi);
    }

    priv->nb_items ++;
    evas_object_event_callback_add(pi->o_edje, EVAS_CALLBACK_MOUSE_DOWN,
				   _wall_event_mouse_down_cb, pi);
    evas_object_event_callback_add(pi->o_pict, EVAS_CALLBACK_IMAGE_PRELOADED,
				   _walle_event_image_preload_cb, pi);
}
