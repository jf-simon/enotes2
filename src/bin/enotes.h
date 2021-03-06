#ifndef ENOTES_H
#define ENOTES_H

#endif

#include "config.h"
#include <Ecore_Getopt.h>
#include <Elementary.h>
#include <Elementary_Cursor.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#define EFL_BETA_API_SUPPORT
#include <elm_systray.h>
#undef EFL_BETA_API_SUPPORT

#include "gettext.h"
#include <Ecore_X.h>

typedef struct {
  const char *cat_name;
  Eina_Bool cat_selected;

} My_Conf_Type_Cat;


typedef struct {
  const char *note_name;
  int id;
  int x;
  int y;
  int h;
  int w;
  int h_m;
  int w_m;
  int h_mtmp;
  int w_mtmp;
  int h_cs;
  int w_cs;
  int h_cstmp;
  int w_cstmp;
  int color_r, color_g, color_b, color_a;
  int text_color;
  const char *tcolor;
  int notetext_size;
  int titletext_size;
  Eina_Bool iconify;
  Eina_Bool sticky;
  const char *menu;
  const char *blur;
  const char *theme;
  const char *note_text;
  const char *categories;
} Note;


extern Eina_List *note_list;                  // list with all informations about a note, this will be store to eet and filled in starting enotes
extern Eina_List *enotes_all_objects_list;    // list which holds a struct with the Evas_Objects of every single notes created from *note_list
extern Eina_List *cat_list;                   // used to save the categories list from the settings


extern Eina_Bool ci_systray;
extern Eina_Bool ci_border_enabled;
extern Eina_Bool ci_quitpopup_check;
extern int ci_default_notefontsize;
extern int ci_default_titlefontsize;
extern const char *cat_settings;
extern const char *activ_cat;
extern Eina_List *cat_list_settings;

extern char enotes_running[PATH_MAX];
extern Eina_Bool all_hidden;
extern Evas_Object *categories_frame;
extern Evas_Object *bx_c;
extern Evas_Object *test;
extern Evas_Object *text;
extern Evas_Object *list;
extern Eo *item;
extern Elm_Object_Item *menu_it3;
extern Evas_Object *it;
extern Evas_Object *help_win;
extern Evas_Object *all_notes_bx;


void _del_local_data(int del_id);
void _open_settings(void *data EINA_UNUSED, Evas_Object *obj EINA_UNUSED,
                    const char *em EINA_UNUSED, const char *src EINA_UNUSED);
void _save_and_check_user_data(void *data, Evas_Object *obj EINA_UNUSED,
                               void *event_info EINA_UNUSED);
void fill_cat_list();
char *stringReplace(char *search, char *replace, char *string);
char *find_data(char *string, char *start1, char *end1);
void _enotes_del_local(int del_id);
void _hide_show_all_notes(void *data, Evas_Object *obj,
                          void *event_info EINA_UNUSED);
void _enotes_exit(void *data EINA_UNUSED, Evas_Object *obj EINA_UNUSED,
                  void *event_info EINA_UNUSED);
void enotes_systray();
void _enotes_new();
void enotes_win_help(void *data, Evas_Object *obj EINA_UNUSED,
                     const char *em EINA_UNUSED, const char *src EINA_UNUSED);

Evas_Object *cat_list_fill_note();

void catlist_to_catlisteet();

void _fill_list_to_notes();

void _it_clicked_cb(void *data, Evas_Object *li, void *event_info EINA_UNUSED);

void _esc_check(void *data EINA_UNUSED, Evas *e EINA_UNUSED,
                Evas_Object *obj EINA_UNUSED, void *event_info);

void enotes_win_help(void *data, Evas_Object *obj EINA_UNUSED,
                     const char *em EINA_UNUSED, const char *src EINA_UNUSED);

void enotes_win_help_close(void *data, Evas *e EINA_UNUSED,
                           Evas_Object *obj EINA_UNUSED,
                           void *event_info EINA_UNUSED);

void _close_notify(void *data);

void _delete_dialogs_cs(void *data, Evas *e EINA_UNUSED,
                        Evas_Object *obj EINA_UNUSED,
                        void *event_info EINA_UNUSED);

void _close_all2(void *data EINA_UNUSED, Evas *e EINA_UNUSED,
                 Evas_Object *obj EINA_UNUSED, void *event_info);

Evas_Object *item_provider(void *images EINA_UNUSED,
                           Evas_Object *entry_notecontent, const char *item);
void fill_list_in_settings();

void fill_list_in_settings1(void *data EINA_UNUSED,
                            Evas_Object *obj EINA_UNUSED,
                            const char *em EINA_UNUSED,
                            const char *src EINA_UNUSED);

void save_enotes_all_objects(void *data, Evas *e EINA_UNUSED,
                             Evas_Object *obj EINA_UNUSED, void *event_info);

int count_notes_per_category(const char *cat_name);

void update_visible_notes();

void _fill_allnotes_settings(Evas_Object *bxp);

void _enotes_del_request(void *data,
                    Evas_Object* obj EINA_UNUSED,
                    void* event_info EINA_UNUSED);
