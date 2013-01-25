typedef struct _Walle Walle;

struct _Walle
{
    struct {
        char      *engine;
        char      *path;
        char      *theme;
        Evas_Coord width;
        Evas_Coord height;
        int        fps;
        int        lines;
    } config;
    Evas        *evas;
    Evas_Object *o_win;
    Evas_Object *o_edje;
    Evas_Object *o_wall;
    Evas_Object *o_scroll;
    Evas_Object *o_toolbar;
    Eina_List   *pictures;
    Ecore_File_Monitor *monitor;
};

Walle *walle_init(int argc, char **argv);

void walle_shutdown(Walle *walle);

int walle_gui_set(Walle *walle);
