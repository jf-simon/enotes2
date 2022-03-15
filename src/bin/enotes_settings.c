#ifdef HAVE_CONFIG_H
#include "config.h"
#else
#define EFL_BETA_API_SUPPORT
#define EFL_EO_API_SUPPORT
#endif

#include "enotes.h"


int settings_on;
double step;
Evas_Object *win_s;
Evas_Object *all_notes_bx;

Evas_Object* settings_win = NULL;
Ecore_Job *update_notes_job;

static void
_close_settings(void* data EINA_UNUSED,
                Evas_Object* obj EINA_UNUSED,
                void* event_info EINA_UNUSED)
{
   settings_on = 0;
   evas_object_hide(settings_win);
}

static double
_step_size_calculate(double min, double max)
{
   double step = 0.0;
   int steps = 0;

   steps = max - min;
   if (steps) step = (1.0 / steps);
   return step;
}

static void
_toggle_border(void* data, Evas_Object* obj, void* event_info EINA_UNUSED)
{

   Eina_Bool state = elm_check_state_get(obj);

   Eina_List* l;
   Eina_List* list_values;
   EINA_LIST_FOREACH(enotes_all_objects_list,
                     l,
                     list_values) // LISTE DER OBJEKTE DURCHGEHEN
   {
      Evas_Object* win = eina_list_nth(list_values, 0);

      if (state == EINA_TRUE)
         elm_win_borderless_set(win, EINA_FALSE);
      else
         elm_win_borderless_set(win, EINA_TRUE);
   }
   ci_border_enabled = state;

}

static void
_systray_callback(void* data, Evas_Object* obj, void* event_info EINA_UNUSED)
{
   Eina_Bool state = elm_check_state_get(obj);

   if (state == EINA_TRUE)
      elm_systray_status_set(item, 0);
   else
      elm_systray_status_set(item, 1);

   ci_systray = state;
}

static void
_quitceck_callback(void* data, Evas_Object* obj, void* event_info EINA_UNUSED)
{
   Eina_Bool state = elm_check_state_get(obj);

   ci_quitpopup_check = state;
}

static void
_config_show_categories(void *data, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   Evas_Object *tb = data;

   Evas_Object *advanced_frame = evas_object_data_get(tb, "advanced_frame");
   Evas_Object *categories_frame = evas_object_data_get(tb, "categories_frame");
   Evas_Object *help_frame = evas_object_data_get(tb, "help_frame");
   Evas_Object *sync_frame = evas_object_data_get(tb, "sync_frame");

   evas_object_show(categories_frame);
   evas_object_hide(advanced_frame);
   evas_object_hide(help_frame);
   evas_object_hide(sync_frame);
}

static void
_config_show_advanced(void *data, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   Evas_Object *tb = data;

   Evas_Object *advanced_frame = evas_object_data_get(tb, "advanced_frame");
   Evas_Object *categories_frame = evas_object_data_get(tb, "categories_frame");
   Evas_Object *help_frame = evas_object_data_get(tb, "help_frame");
   Evas_Object *sync_frame = evas_object_data_get(tb, "sync_frame");

   evas_object_hide(categories_frame);
   evas_object_show(advanced_frame);
   evas_object_hide(help_frame);
   evas_object_hide(sync_frame);
}

static void
_config_show_help(void *data, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   Evas_Object *tb = data;

   Evas_Object *advanced_frame = evas_object_data_get(tb, "advanced_frame");
   Evas_Object *categories_frame = evas_object_data_get(tb, "categories_frame");
   Evas_Object *help_frame = evas_object_data_get(tb, "help_frame");
   Evas_Object *sync_frame = evas_object_data_get(tb, "sync_frame");

   evas_object_hide(categories_frame);
   evas_object_hide(advanced_frame);
   evas_object_show(help_frame);
   evas_object_hide(sync_frame);
}

static void
_config_show_notes(void *data, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   Evas_Object *tb = data;
   Evas_Object *all_notes_frame = evas_object_data_get(tb, "all_notes_frame");

   Evas_Object *advanced_frame = evas_object_data_get(tb, "advanced_frame");
   Evas_Object *categories_frame = evas_object_data_get(tb, "categories_frame");
   Evas_Object *help_frame = evas_object_data_get(tb, "help_frame");
   Evas_Object *sync_frame = evas_object_data_get(tb, "sync_frame");

   evas_object_hide(categories_frame);
   evas_object_hide(advanced_frame);
   evas_object_hide(help_frame);
   evas_object_hide(sync_frame);
   _fill_allnotes_settings(all_notes_bx);
   evas_object_show(all_notes_frame);
}

void update_notes_job_cb(void *data)
{


   update_visible_notes();
   _fill_allnotes_settings(all_notes_bx);
}

static void _hoversel_clicked_delete_single_note_cb(void *data, Evas_Object *obj,
                                                  void *event_info)
{
//    int id = (int)(intptr_t)data;
   _enotes_del_request(data, NULL, NULL);
   update_notes_job = ecore_job_add(update_notes_job_cb, NULL);
}

static void _hoversel_clicked_move_single_note_cb(void *data, Evas_Object *obj,
                                                  void *event_info)
{
   int id = (int)(intptr_t)data;
   Eina_List *l1;
   Note *list_data;

  EINA_LIST_FOREACH(note_list, l1, list_data) // LISTE DER OBJEKTE DURCHGEHEN
  {
    if (id == list_data->id)
      list_data->categories = eina_stringshare_add(elm_object_item_text_get(event_info));
  }
   update_notes_job = ecore_job_add(update_notes_job_cb, NULL);
}

void _fill_allnotes_settings(Evas_Object *bxp) {

   Eina_List *l1, *l2;
   Evas_Object *lb, *bx1;
   Note *list_data;
   char buf[PATH_MAX];
   elm_box_clear(bxp);

   const Eina_List *l, *items, *items1;
   Elm_Object_Item *list_it;
   Elm_Object_Item *list_it1;
   items = elm_list_items_get(list);
   items1 = eina_list_clone(items);
   Evas_Object *fr, *bx, *hv;

   EINA_LIST_FOREACH(items, l, list_it)
   {
      fr = elm_frame_add(bxp);

      if(!strcmp(elm_object_item_text_get(list_it), activ_cat))
         snprintf(buf, sizeof(buf), "%s - current Category", elm_object_item_text_get(list_it));
      else
         snprintf(buf, sizeof(buf), "%s", elm_object_item_text_get(list_it));

      elm_object_text_set(fr, buf);
      evas_object_size_hint_weight_set(fr, EVAS_HINT_EXPAND, 0.0);
      evas_object_size_hint_align_set(fr, EVAS_HINT_FILL, 0.0);
      evas_object_show(fr);

      bx1 = elm_box_add(fr);
      evas_object_size_hint_weight_set(bx1, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
      evas_object_size_hint_align_set(bx1, EVAS_HINT_FILL, 0.0);
      evas_object_show(bx1);

      EINA_LIST_FOREACH(note_list, l1, list_data) // LISTE DER OBJEKTE DURCHGEHEN
      {
         const char* cat = list_data->categories;

         if(!strcmp(elm_object_item_text_get(list_it), cat))
         {
            bx = elm_box_add(fr);
            elm_box_horizontal_set(bx, EINA_TRUE);
            elm_box_homogeneous_set(bx, EINA_TRUE);
            evas_object_size_hint_weight_set(bx, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
            evas_object_size_hint_align_set(bx, EVAS_HINT_FILL, 0.0);
            evas_object_show(bx);

            lb = elm_label_add(bx);
            evas_object_size_hint_weight_set(lb, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
            evas_object_size_hint_align_set(lb, 0.0, 0.5);
            elm_object_text_set(lb, list_data->note_name);
            evas_object_show(lb);
            elm_box_pack_end(bx, lb);

            hv = elm_hoversel_add(bx);
            evas_object_size_hint_weight_set(hv, EVAS_HINT_EXPAND, 0.0);
            evas_object_size_hint_align_set(hv, EVAS_HINT_FILL, 0.5);
            elm_hoversel_auto_update_set(hv, EINA_TRUE);
            elm_hoversel_hover_parent_set(hv, win_s);
            elm_object_text_set(hv, "Move to");

            EINA_LIST_FOREACH((Eina_List*)items1, l2, list_it1){
               if(strcmp(elm_object_item_text_get(list_it1), cat)){
                  it = elm_hoversel_item_add(hv, elm_object_item_text_get(list_it1), NULL, ELM_ICON_NONE, _hoversel_clicked_move_single_note_cb, (void*)(intptr_t)list_data->id);
               }
            }

            it = elm_hoversel_item_add(hv, "Trash", "edit-delete", ELM_ICON_STANDARD, _hoversel_clicked_delete_single_note_cb, (void*)(intptr_t)list_data->id);

            evas_object_show(hv);
            elm_box_pack_end(bx, hv);
            elm_box_pack_end(bx1, bx);
         }
      }

      elm_object_content_set(fr, bx1);
      elm_box_pack_end(bxp, fr);
   }
}

void
catlist_to_catlisteet()
{
   printf("CATLIST TO CATLIST EET\n");
   Eina_List *l;
   const Eina_List *cat_list_new;
   cat_list_new = elm_list_items_get(list);
   Elm_Object_Item *lit;

   cat_list_settings = NULL;
   EINA_LIST_FOREACH((Eina_List*) cat_list_new, l, lit)
   {
      My_Conf_Type_Cat* new;
      new = calloc(1, sizeof(My_Conf_Type_Cat));
      new->cat_name = elm_object_item_text_get(lit);
      new->cat_selected = elm_list_item_selected_get(lit);

      cat_list_settings = eina_list_append(cat_list_settings, new);
   }
}

void
update_visible_notes()
{
  Eina_List *list_values;
  void* id1;
  Eina_List *l, *l1;
  Eina_List *tmp = NULL;

   Note* list_data;

      EINA_LIST_FOREACH(note_list, l1, list_data)
      {
         if((list_data->categories != NULL) && ((strcmp(list_data->categories, activ_cat) == 0) || (strcmp(list_data->categories, "") == 0)))
            tmp = eina_list_append(tmp, (void*)(intptr_t)list_data->id);
      }

  printf("activ cat %s\n", activ_cat);

  EINA_LIST_FOREACH(enotes_all_objects_list, l, list_values) // LISTE DER OBJEKTE DURCHGEHEN
  {
    Evas_Object *win = eina_list_nth(list_values, 0);
    int id = (int)(intptr_t)eina_list_nth(list_values, 7);
    Evas_Object *en = eina_list_nth(list_values, 3);
    int x = 0, y = 0, w = 0, h = 0;
    evas_object_geometry_get(win, &x, &y, &w, &h);
    int found = 0;


    EINA_LIST_FOREACH(tmp, l1, id1) // LISTE DER OBJEKTE DURCHGEHEN
    {
      if (id == (int)(intptr_t)id1) {
        printf("show id: %i x:%i - y:%i, %s\n", id, x, y, elm_object_text_get(en));
        evas_object_show(win);
        evas_object_move(win, x, y);
        found = 1;
      }
    }
    if (found == 0) {
      printf("hide id: %i x:%i - y:%i, %s\n", id, x, y, elm_object_text_get(en));
      evas_object_hide(win);
    }
  }
}

void
_cat_selected(void *data EINA_UNUSED, Evas_Object *li, void *event_info EINA_UNUSED) // SHOW/HIDE Notes based on aktive cat list items
{
   Note* list_data;
   Eina_List *l;
   Eina_List *tmp = NULL;

   Elm_Object_Item *selected_cat = elm_list_selected_item_get(li);
   activ_cat =  elm_object_item_text_get(selected_cat);
   printf("ACTIV CAT: %s\n", activ_cat);

   EINA_LIST_FOREACH(note_list, l, list_data)
   {
      if((list_data->categories != NULL) && ((strcmp(list_data->categories, activ_cat) == 0) || (strcmp(list_data->categories, "") == 0)))
      {
         tmp = eina_list_append(tmp, (void*)(intptr_t)list_data->id);
      }
   }

   if(eina_list_count(tmp) != 0)
      update_visible_notes();
   else
      _enotes_new();

   eina_list_free(tmp);
}

static void
_cancel_add_notify(void *data, Evas_Object *li EINA_UNUSED,
                   void *event_info EINA_UNUSED)
{
   evas_object_del(data);
}

static void
_add_add_notify(void *data, Evas_Object *li EINA_UNUSED,
                void *event_info EINA_UNUSED)
{
   Evas_Object *en = data;
   Elm_Object_Item *it;
   Note* list_data;
   Eina_List *l;
   Eina_List *tmp = NULL;

   // TODO auf vorhandene gleich einträge checken
   if(strcmp(elm_object_text_get(en), ""))
   {
      it = elm_list_item_append(list, elm_object_text_get(en), NULL, NULL, NULL, list);
      activ_cat = elm_object_text_get(en);
      elm_object_text_set(en, "");  // reset entry
      elm_list_go(list);            // update cat list
      elm_list_item_selected_set(it, EINA_TRUE);

      EINA_LIST_FOREACH(note_list, l, list_data)
      {
         if(strcmp(list_data->categories, activ_cat) == 0)
         {
            tmp = eina_list_append(tmp, (void*)(intptr_t)list_data->id);
         }
      }
      update_visible_notes();
      catlist_to_catlisteet();
      eina_list_free(tmp);
   }
   
   
   enotes_systray();
}

static void
_add_add_keydown_notify(void* data,
                        Evas* e EINA_UNUSED,
                        Evas_Object* obj,
                        void* event_info)
{
   Evas_Event_Key_Down* ev = event_info;
   Evas_Object *en = data;
   const char* k = ev->key;
   Elm_Object_Item *it;
   Note* list_data;
   Eina_List *l;
   Eina_List *tmp = NULL;


   if(strcmp(elm_object_text_get(en), "") && !strcmp(k, "Return")) // check if the new cat isnt in the list
   {
      it = elm_list_item_append(list, elm_object_text_get(en), NULL, NULL, NULL, list);
      activ_cat = elm_object_text_get(en);
      elm_object_text_set(en, ""); // reset entry
      elm_list_go(list); // update cat list
      elm_list_item_selected_set(it, EINA_TRUE);

      EINA_LIST_FOREACH(note_list, l, list_data)
      {
         if(strcmp(list_data->categories, activ_cat) == 0)
         {
            tmp = eina_list_append(tmp, (void*)(intptr_t)list_data->id);
         }
      }
      update_visible_notes();
      catlist_to_catlisteet();
      eina_list_free(tmp);
   }
   if(!strcmp(k, "Escape")) // check if the new cat isnt in the list
   {
      evas_object_del(obj);
   }
   
   enotes_systray();
}

static void
_it_clicked_add_cb(void *data, Evas_Object *li EINA_UNUSED,
                   void *event_info EINA_UNUSED)
{
   Evas_Object *win = data;
   Evas_Object *bxv, *bx, *o, *notify, *en_add = NULL;
   notify = elm_notify_add(win);
   elm_notify_allow_events_set(notify, EINA_FALSE);
   evas_object_smart_callback_add(notify, "block,clicked", _cancel_add_notify, notify);
   elm_notify_align_set(notify, 0.5, 0.5);

   bx = elm_box_add(notify);
   elm_box_horizontal_set(bx, EINA_FALSE);
   elm_object_content_set(notify, bx);

   o = elm_label_add(bx);
   elm_object_text_set(o, gettext("Add new category:"));
   evas_object_show(o);
   elm_box_pack_end(bx, o);

   en_add = elm_entry_add(bx);
   evas_object_size_hint_weight_set(en_add, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(en_add, EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_entry_editable_set(en_add, EINA_TRUE);
   elm_entry_single_line_set(en_add, EINA_TRUE);
   evas_object_show(en_add);
   elm_box_pack_end(bx, en_add);

   bxv = elm_box_add(notify);
   elm_box_horizontal_set(bxv, EINA_TRUE);

   o = elm_button_add(bxv);
   elm_object_text_set(o, gettext("add"));
   evas_object_smart_callback_add(o, "clicked", _add_add_notify, en_add);
   evas_object_show(o);
   elm_box_pack_end(bxv, o);

   o = elm_button_add(bxv);
   elm_object_text_set(o, gettext("close"));
   evas_object_smart_callback_add(o, "clicked", _cancel_add_notify, notify);
   evas_object_show(o);
   elm_box_pack_end(bxv, o);
   evas_object_show(bxv);

   elm_box_pack_end(bx, bxv);

   evas_object_event_callback_add(notify, EVAS_CALLBACK_KEY_DOWN, _add_add_keydown_notify, en_add);
   evas_object_show(notify);
   evas_object_show(bx);

}

static void
_block_clicked_cb(void *data, Evas_Object *obj, void *event_info EINA_UNUSED)
{
   Evas_Object *popup = data;
   if (popup)
      evas_object_del(popup);

   evas_object_del(obj);
   enotes_systray();
}


static void
_hoversel_clicked_move_notes_cb(void *data, Evas_Object *obj,
                                void *event_info EINA_UNUSED)
{
   Eina_List *l, *tmp = NULL;
   Evas_Object *popup = evas_object_data_get(obj, "popup");
   Note* list_data;

   Elm_Object_Item *selected_item;
   Elm_Object_Item *to_it = data;

   selected_item = elm_list_selected_item_get(list);

   EINA_LIST_FOREACH(note_list, l, list_data)
   {
      if(strcmp(list_data->categories, elm_object_item_text_get(selected_item)) == 0)
      {
         printf("move ID: %i to: %s\n", list_data->id, elm_object_item_text_get(to_it));
         list_data->categories = eina_stringshare_add(elm_object_item_text_get(to_it));
         tmp = eina_list_append(tmp, (void*)(intptr_t)list_data->id);

         activ_cat = elm_object_item_text_get(to_it);
      }
   }

   //    save_enotes_all_objects(NULL, NULL, NULL, "0");

   update_visible_notes();

   eina_list_free(tmp);

   selected_item = elm_list_selected_item_get(list);
   elm_object_item_del(selected_item);
   elm_list_item_selected_set(to_it, EINA_TRUE);
   //     elm_list_go(list);
   catlist_to_catlisteet();
   //     fill_list_in_settings(); // Update cat list in settings window

   if (popup)
      evas_object_del(popup);
   
   
   enotes_systray();
}

static void
_hoversel_clicked_delete_notes_cb(void *data, Evas_Object *obj,
                                  void *event_info EINA_UNUSED)
{
   if(eina_list_count(elm_list_items_get(list)) == 1)
      return;

   Evas_Object *popup = data;
   Eina_List *l, *l1, *tmp = NULL;
   Note* list_data;
   void *tmp_id;
   Elm_Object_Item *selected_item = NULL, *it;

   selected_item = elm_list_selected_item_get(list);
   const char *name = elm_object_item_text_get(selected_item);

   char buf[PATH_MAX];
   snprintf(buf, sizeof(buf), "%s", name);

   printf("item to delete 1: %s\n", buf);
   EINA_LIST_FOREACH(note_list, l, list_data)
   {
      if(strcmp(list_data->categories, buf) == 0)
         tmp = eina_list_append(tmp, (void*)(intptr_t)list_data->id);
   }

   EINA_LIST_FOREACH(tmp, l1, tmp_id)
   {
      _del_local_data((int)(intptr_t)tmp_id);
   }
   eina_list_free(tmp);

   //    selected_item = elm_list_selected_item_get(list);
   printf("TEST DELETE\n");
   printf("item to delete 2: %s\n", buf); //WARNING elm_wdg_item_part_text_get

   elm_object_item_del(elm_list_selected_item_get(list));
   //    elm_list_go(list);

   it = elm_list_first_item_get(list);
   elm_list_item_selected_set(it, EINA_TRUE);

   //    elm_list_go(list);
   //     catlist_to_catlisteet();
   //     save_enotes_all_objects(NULL, NULL, NULL, "0");
   //    fill_list_in_settings(); // Update cat list in settings window

   if (popup)
      evas_object_del(popup);
   
   enotes_systray();
}

static void
_cat_clicked_del_cb(void *data, Evas_Object *li EINA_UNUSED,
                    void *event_info EINA_UNUSED)
{
   Evas_Object *popup, *btn_d, *btn_c, *box, *lb, *hoversel, *sep;
//    Elm_Object_Item *it = NULL;

   popup = elm_popup_add(data);
   evas_object_smart_callback_add(popup, "block,clicked", _block_clicked_cb, NULL);

   box = elm_box_add(popup);
   evas_object_show(box);

   elm_object_part_text_set(popup, "title,text", "Choose your option");

   lb = elm_label_add(box);
   elm_object_text_set(lb, "Delete Categorie and delete all notes within?");
   evas_object_show(lb);
   elm_box_pack_end(box, lb);

   btn_d = elm_button_add(box);
   elm_object_text_set(btn_d, "Delete");
   evas_object_show(btn_d);
   elm_box_pack_end(box, btn_d);

   sep = elm_separator_add(box);
   elm_separator_horizontal_set(sep, EINA_TRUE);
   evas_object_show(sep);
   elm_box_pack_end(box, sep);

   lb = elm_label_add(box);
   elm_object_text_set(lb, "Move notes to Categorie");
   evas_object_show(lb);
   elm_box_pack_end(box, lb);

   hoversel = elm_hoversel_add(box);
   elm_hoversel_auto_update_set(hoversel, EINA_TRUE);
   elm_hoversel_hover_parent_set(hoversel, popup);
   elm_object_text_set(hoversel, "Categories");

   const Eina_List *l, *items;
   Elm_Object_Item *list_it, *selected_item;

   items = elm_list_items_get(list);
   selected_item = elm_list_selected_item_get(list);

   EINA_LIST_FOREACH(items, l, list_it){
      evas_object_data_set(hoversel, "popup", popup);
      if(strcmp(elm_object_item_text_get(list_it), elm_object_item_text_get(selected_item)))
         it = elm_hoversel_item_add(hoversel, elm_object_item_text_get(list_it), NULL, ELM_ICON_NONE, _hoversel_clicked_move_notes_cb, list_it);
   }

   evas_object_show(hoversel);
   elm_box_pack_end(box, hoversel);

   sep = elm_separator_add(box);
   elm_separator_horizontal_set(sep, EINA_TRUE);
   evas_object_show(sep);
   elm_box_pack_end(box, sep);

   lb = elm_label_add(box);
   elm_object_text_set(lb, "Do nothing and go back");
   evas_object_show(lb);
   elm_box_pack_end(box, lb);

   btn_c = elm_button_add(box);
   elm_object_text_set(btn_c, "Cancel");
   evas_object_show(btn_c);
   elm_box_pack_end(box, btn_c);

   elm_object_content_set(popup, box);
   evas_object_smart_callback_add(btn_c, "clicked", _block_clicked_cb, popup);

   evas_object_smart_callback_add(btn_d, "clicked", _hoversel_clicked_delete_notes_cb, popup);

   evas_object_show(popup);
}

void
_key_down_settings(void* data,
                   Evas* e EINA_UNUSED,
                   Evas_Object* obj EINA_UNUSED,
                   void* event_info)
{
   Evas_Event_Key_Down* ev = event_info;
   const char* k = ev->keyname;
   Eina_Bool ctrl = evas_key_modifier_is_set(ev->modifiers, "Control");
   printf("keydown win\n");
   if (ctrl) {
      if (!strcmp(k, "q")) // close enotes
      {
         _close_notify(data);
      }
   }
   if (!strcmp(k, "Escape")) {
      ev->event_flags |= EVAS_EVENT_FLAG_ON_HOLD;
   }
   if (!strcmp(k, "F5")) {
      _enotes_new();
   }
}

void
_esc_check(void* data EINA_UNUSED,
           Evas* e EINA_UNUSED,
           Evas_Object* obj EINA_UNUSED,
           void* event_info)
{
   printf("keydown list\n");
   Evas_Event_Key_Down* ev = event_info;
   const char* k = ev->keyname;

   if (!strcmp(k, "Escape")) {
      ev->event_flags |= EVAS_EVENT_FLAG_ON_HOLD;
   }

}

int
count_notes_per_category(const char* cat_name)
{
   int count = 0;
   Eina_List *l;
   Note* list_data;

   EINA_LIST_FOREACH(note_list, l, list_data)
   {
      if(strcmp(list_data->categories, cat_name) == 0)
         count++;
   }
   return count;
}

void
fill_list_in_settings()
{
   Eina_List *l;
   My_Conf_Type_Cat* new;
   new = calloc(1, sizeof(My_Conf_Type_Cat));

      printf("TEST\n");
   //    catlist_to_catlisteet();
   elm_list_clear(list);
   EINA_LIST_FOREACH(cat_list_settings, l, new)
   {
      Elm_Object_Item *it;
      //       int count = 0;
      //       char buf[PATH_MAX];

      //       count = count_notes_per_category(new->cat_name);
      //       snprintf(buf, sizeof(buf), gettext("<color=white>%i Note(s)</color>"), count);

      //       Evas_Object *lb = elm_label_add(list);
      //       elm_object_text_set(lb, buf);
      //       evas_object_color_set(lb, 255, 255, 255, 255);
      //       evas_object_show(lb);

      it = elm_list_item_append(list, new->cat_name, NULL, /*buf*/NULL, NULL, NULL);
      elm_list_item_selected_set(it, new->cat_selected);

   }
   elm_list_go(list);
}

void
fill_list_in_settings1(void* data EINA_UNUSED,
                       Evas_Object* obj EINA_UNUSED,
                       const char* em EINA_UNUSED,
                       const char* src EINA_UNUSED)
{
   fill_list_in_settings();
}


static void
_note_textsize_preview_cb(void *data, Evas_Object *obj, void *event_info EINA_UNUSED)
{
   double val = elm_slider_value_get(obj);

   char buf[PATH_MAX];
   snprintf(buf,
            sizeof(buf),
            "DEFAULT='font=Sans:style=Regular color=white font_size=%1.2f'",val);
   elm_entry_text_style_user_push(data, buf);

   ci_default_notefontsize = (int)val;
   printf("NOTE TEXT SIZE: %1.2f\n", val);
}

static void
_title_textsize_preview_cb(void *data, Evas_Object *obj, void *event_info EINA_UNUSED)
{
   double val = elm_slider_value_get(obj);

   char buf[PATH_MAX];
   snprintf(buf,
            sizeof(buf),
            "DEFAULT='font=Sans:style=Regular color=white font_size=%1.2f'",val);
   elm_entry_text_style_user_push(data, buf);

   ci_default_titlefontsize = (int)val;
   printf("TITLE TEXT SIZE: %1.2f\n", val);
}


void
_open_settings(void* data,
               Evas_Object* obj EINA_UNUSED,
               const char* em EINA_UNUSED,
               const char* src EINA_UNUSED)
{
   Evas_Object *lb, *tb_settings, *hbx, *separator;
   Evas_Object *advanced_frame, *help_frame, *en_help, *systray_check, *check_border_enabled, *check_quitpopup_check, *bt_add, *bt_del, *sl_fontsize, *sl_titlesize, *en_notetextpreview, *en_titletextpreview;
   Evas_Object *all_notes_frame;
   Evas_Object *bx;

   // List Objects
   Elm_Object_Item *list1;
   Evas_Object *it;
//    Eina_List* list_values = data;

//    Eina_List* list_values_check = NULL;

   if (settings_on != 1) {
      win_s = elm_win_util_standard_add("enotes-settings", "Enote SETTINGS");
      elm_win_title_set(win_s, gettext("eNotes Settings"));

      evas_object_smart_callback_add(win_s, "delete,request", _close_settings, win_s);

      tb_settings = elm_table_add(win_s);
      evas_object_size_hint_weight_set(tb_settings, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
      evas_object_show(tb_settings);

      list1 = elm_list_add(tb_settings);
      evas_object_size_hint_weight_set(list1, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
      evas_object_size_hint_align_set(list1, EVAS_HINT_FILL, EVAS_HINT_FILL);
      elm_table_pack(tb_settings, list1, 0, 1, 1, 1);

      elm_list_select_mode_set(list1, ELM_OBJECT_SELECT_MODE_ALWAYS);

      elm_scroller_content_min_limit(list1, 1, 1);

      it = elm_list_item_append(list1, gettext("Categories"), NULL, NULL,
                                _config_show_categories, tb_settings);
      elm_list_item_selected_set(it, 1);

      it = elm_list_item_append(list1, gettext("Notes"), NULL, NULL,
                                   _config_show_notes, tb_settings);

      it = elm_list_item_append(list1, gettext("Advanced"), NULL, NULL,
                                _config_show_advanced, tb_settings);

      it = elm_list_item_append(list1, gettext("Help"), NULL, NULL,
                                _config_show_help, tb_settings);


      elm_list_go(list1);

      evas_object_event_callback_add(list1, EVAS_CALLBACK_KEY_DOWN, _esc_check, NULL);
      evas_object_show(list1);



      /// All Notes FRAME ///
      all_notes_frame = elm_frame_add(win_s);
      elm_object_text_set(all_notes_frame, "All Notes");
      evas_object_size_hint_weight_set(all_notes_frame, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
      evas_object_size_hint_align_set(all_notes_frame, EVAS_HINT_FILL, 0.0);
      elm_table_pack(tb_settings, all_notes_frame, 1, 1, 1, 1);

      all_notes_bx = elm_box_add(all_notes_frame);
      evas_object_data_set(tb_settings, "all_notes_bx", all_notes_bx);
      evas_object_size_hint_weight_set(all_notes_bx, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
      evas_object_size_hint_align_set(all_notes_bx, EVAS_HINT_FILL, 0.0);
      evas_object_show(all_notes_bx);


      _fill_allnotes_settings(all_notes_bx);
   //////////////////// All Notes FRAME  END

   elm_object_content_set(all_notes_frame, all_notes_bx);
   evas_object_data_set(tb_settings, "all_notes_frame", all_notes_frame);
   /// TEST FRAME END ///


      /// CATEGORIES FRAME ///
      categories_frame = elm_frame_add(win_s);
       elm_object_style_set(categories_frame, "outline");
      elm_object_text_set(categories_frame, gettext("Categories"));
//       elm_object_style_set(categories_frame, "pad_huge");
      evas_object_size_hint_weight_set(categories_frame, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
      evas_object_size_hint_align_set(categories_frame, EVAS_HINT_FILL, EVAS_HINT_FILL);
      elm_table_pack(tb_settings, categories_frame, 1, 1, 1, 1);

      bx_c = elm_box_add(categories_frame);
      //                   evas_object_size_hint_weight_set(bx_c, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
      evas_object_size_hint_align_set(bx_c, 0, 0);

      list = elm_list_add(bx_c);
      elm_list_multi_select_set(list, EINA_FALSE);
      elm_list_multi_select_mode_set(list, ELM_OBJECT_MULTI_SELECT_MODE_DEFAULT);
      elm_list_select_mode_set(list, ELM_OBJECT_SELECT_MODE_ALWAYS);
      evas_object_size_hint_weight_set(list, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
      evas_object_size_hint_align_set(list, EVAS_HINT_FILL, 0.0);

      elm_list_mode_set(list, ELM_LIST_EXPAND);

      fill_list_in_settings(); // FILL LIST IN SETTINGS

      evas_object_smart_callback_add(list, "selected", _cat_selected, NULL);

      evas_object_show(list);
      elm_box_pack_end(bx_c, list);

      hbx = elm_box_add(bx_c);
      elm_box_horizontal_set(hbx, EINA_TRUE);
      evas_object_size_hint_weight_set(hbx, EVAS_HINT_EXPAND, 0);
      evas_object_size_hint_align_set(hbx, EVAS_HINT_FILL, 0);

      bt_add = elm_button_add(hbx);
      //                         evas_object_size_hint_weight_set(bt_add, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
      //                         evas_object_size_hint_align_set(bt_add, EVAS_HINT_FILL, EVAS_HINT_FILL);
      elm_object_text_set(bt_add, gettext("Add"));
      evas_object_smart_callback_add(bt_add, "clicked", _it_clicked_add_cb, win_s);
      evas_object_show(bt_add);
      elm_box_pack_end(hbx, bt_add);

      separator = elm_separator_add(hbx);
      elm_separator_horizontal_set(separator, EINA_FALSE);
      evas_object_show(separator);
      elm_box_pack_end(hbx, separator);

      bt_del = elm_button_add(hbx);
      //                         evas_object_size_hint_weight_set(bt_del, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
      //                         evas_object_size_hint_align_set(bt_del, EVAS_HINT_FILL, EVAS_HINT_FILL);
      elm_object_text_set(bt_del, gettext("Delete/Move selected"));
      evas_object_smart_callback_add(bt_del, "clicked", _cat_clicked_del_cb, win_s);
      evas_object_show(bt_del);
      elm_box_pack_end(hbx, bt_del);

      evas_object_show(hbx);

      elm_box_pack_end(bx_c, hbx);
      evas_object_show(bx_c);


      elm_object_content_set(categories_frame, bx_c);
      evas_object_data_set(tb_settings, "categories_frame", categories_frame);
      /// CATEGORIES FRAME END ///


      /// HELP FRAME ///
      help_frame = elm_frame_add(win_s);
       elm_object_style_set(help_frame, "outline");
      elm_object_text_set(help_frame, "Help");
      evas_object_size_hint_weight_set(help_frame, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
      evas_object_size_hint_align_set(help_frame, EVAS_HINT_FILL, EVAS_HINT_FILL);
      elm_table_pack(tb_settings, help_frame, 1, 1, 1, 1);

      bx = elm_box_add(help_frame);
      evas_object_size_hint_weight_set(bx, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
      evas_object_size_hint_align_set(bx, EVAS_HINT_FILL, 0.0);

      en_help = elm_entry_add(bx);
      elm_entry_item_provider_append(en_help, item_provider, NULL);
      elm_config_context_menu_disabled_set(EINA_FALSE);
      elm_entry_scrollable_set(en_help, EINA_TRUE);
      elm_object_text_set(en_help,  gettext("Welcome to enotes! <br><br>"
      "i will save the note for you automaticly <br>"
      "when you leave the note with the mouse! <br><br>"
      "<item relsize=24x24 vsize=full href=done></item> click on "
      "&quot;tape&quot; to move the note<br>"
      "<item relsize=24x24 vsize=full href=done></item> click bottom right "
      "corner to resize<br>"
      "<item relsize=24x24 vsize=full href=done></item> On the left size you "
      "have a little menu, klick on the gray rect to open/close it<br>"
      "<item relsize=24x24 vsize=full href=done></item> Use Mouse Scolling + "
      "CTRL to decrease/increase the font. Or STRG+0 to use standard size<br>"
      "<item relsize=24x24 vsize=full href=done></item> &quot;ESC&quot; "
      "Dismiss dialogs and colorselector<br>"
      "<item relsize=24x24 vsize=full href=done></item> &quot;F1&quot; "
      "Show/hide help<br>"
      "<item relsize=24x24 vsize=full href=done></item> &quot;F2&quot; Blur "
      "text<br>"
      "<item relsize=24x24 vsize=full href=done></item> &quot;F3&quot; adds "
      "<item relsize=24x24 vsize=full href=done></item><br/>"
      "<item relsize=24x24 vsize=full href=done></item> &quot;F4&quot; adds "
      "<item relsize=24x24 vsize=full href=open></item><br/>"
      "<item relsize=24x24 vsize=full href=done></item> &quot;F5&quot; "
      "creates a new note<br/>"
      "<item relsize=24x24 vsize=full href=done></item> &quot;F6&quot; "
      "delete note<br/>"
      "<item relsize=24x24 vsize=full href=done></item> &quot;F7&quot; set "
      "sticky<br/>"
      "<item relsize=24x24 vsize=full href=done></item> &quot;F8&quot; "
      "iconify note<br/>"
      "<item relsize=24x24 vsize=full href=done></item> &quot;F9&quot; "
      "toggle designs<br/>"
      "<item relsize=24x24 vsize=full href=done></item> &quot;F10&quot; "
      "Write backup textfile<br/>"
      "<item relsize=24x24 vsize=full href=done></item> &quot;F12&quot; "
      "Open Settings<br/>"
      "<item relsize=24x24 vsize=full href=done></item> &quot;ctrl + q&quot; "
      "quits enotes<br/>"
      "<item relsize=24x24 vsize=full href=done></item> &quot;crtl + m&quot; "
      "toggle menu<br/><br/>"
      "Have fun! <br><br>"
      "Download and updates:<br><a "
      "href=anc-02>https://github.com/jf-simon/enotes</a><br><br>"
      "Author: Simon Tischer [jf_simon on irc.libera.chat #e.de]"));
      elm_entry_editable_set(en_help, EINA_FALSE);
      evas_object_size_hint_weight_set(
         en_help, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
      evas_object_size_hint_align_set(en_help, EVAS_HINT_FILL, EVAS_HINT_FILL);

      evas_object_show(en_help);
      elm_box_pack_end(bx, en_help);

      evas_object_show(bx);

      elm_object_content_set(help_frame, bx);
      evas_object_data_set(tb_settings, "help_frame", help_frame);
      /// HELP FRAME END ///

      /// ADVANCED FRAME ///
      advanced_frame = elm_frame_add(win_s);
      elm_object_style_set(advanced_frame, "outline");
      elm_object_text_set(advanced_frame, gettext("Advanced"));
      evas_object_size_hint_weight_set(advanced_frame, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
      evas_object_size_hint_align_set(advanced_frame, EVAS_HINT_FILL, 0);
      elm_table_pack(tb_settings, advanced_frame, 1, 1, 1, 1);

      bx = elm_box_add(advanced_frame);
      evas_object_size_hint_weight_set(bx, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
      evas_object_size_hint_align_set(bx, EVAS_HINT_FILL, 0.0);

//       lb = elm_label_add(bx);
//       elm_object_text_set(lb, gettext("Systray Icon:"));
//       //                   evas_object_size_hint_weight_set(lb, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
//       evas_object_size_hint_align_set(lb, 0, 0);
//       evas_object_show(lb);
//       elm_box_pack_end(bx, lb);

      check_quitpopup_check = elm_check_add(bx);
      //                   evas_object_size_hint_weight_set(quitpopup_check, EVAS_HINT_EXPAND, 0);
      evas_object_size_hint_align_set(check_quitpopup_check, 0, 0);
      elm_object_text_set(check_quitpopup_check, gettext("Enable quit Popup"));
      elm_check_state_set(check_quitpopup_check, ci_quitpopup_check);
      evas_object_show(check_quitpopup_check);
      evas_object_smart_callback_add(check_quitpopup_check, "changed", _quitceck_callback, NULL);
//       evas_object_data_set(tb, "quitpopup_check", quitpopup_check);

      evas_object_show(check_quitpopup_check);
      elm_box_pack_end(bx, check_quitpopup_check);

      separator = elm_separator_add(bx);
      evas_object_size_hint_align_set(separator, EVAS_HINT_FILL, EVAS_HINT_FILL);
      evas_object_size_hint_weight_set(separator, EVAS_HINT_EXPAND, 0);
      elm_separator_horizontal_set(separator, EINA_TRUE);
      evas_object_show(separator);
      elm_box_pack_end(bx, separator);

      systray_check = elm_check_add(bx);
      //                   evas_object_size_hint_weight_set(systray_check, EVAS_HINT_EXPAND, 0);
      evas_object_size_hint_align_set(systray_check, 0, 0);
      elm_object_text_set(systray_check, gettext("Disable Systray Icon"));
      elm_check_state_set(systray_check, ci_systray);
      evas_object_show(systray_check);
//       evas_object_data_set(tb, "systray_check", systray_check);
      evas_object_smart_callback_add(systray_check, "changed", _systray_callback, NULL);

      evas_object_show(systray_check);
      elm_box_pack_end(bx, systray_check);

      separator = elm_separator_add(bx);
      evas_object_size_hint_align_set(separator, EVAS_HINT_FILL, EVAS_HINT_FILL);
      evas_object_size_hint_weight_set(separator, EVAS_HINT_EXPAND, 0);
      elm_separator_horizontal_set(separator, EINA_TRUE);
      evas_object_show(separator);
      elm_box_pack_end(bx, separator);
;

      check_border_enabled = elm_check_add(bx);
      //                   evas_object_size_hint_weight_set(m_check, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
      evas_object_size_hint_align_set(check_border_enabled, 0, 0);
      elm_object_text_set(check_border_enabled, gettext("Show Window Border"));
      elm_check_state_set(check_border_enabled, ci_border_enabled);
      evas_object_show(check_border_enabled);
//       evas_object_data_set(tb, "check_border_enabled", check_border_enabled);
      evas_object_smart_callback_add(check_border_enabled, "changed", _toggle_border, NULL);

      evas_object_show(check_border_enabled);
      elm_box_pack_end(bx, check_border_enabled);

      separator = elm_separator_add(bx);
      evas_object_size_hint_align_set(separator, EVAS_HINT_FILL, EVAS_HINT_FILL);
      evas_object_size_hint_weight_set(separator, EVAS_HINT_EXPAND, 0);
      elm_separator_horizontal_set(separator, EINA_TRUE);
      evas_object_show(separator);
      elm_box_pack_end(bx, separator);


      lb = elm_label_add(bx);


      elm_object_text_set(lb, gettext("Textsize:"));
      evas_object_size_hint_align_set(lb, 0, 0);
      evas_object_show(lb);
      elm_box_pack_end(bx, lb);



      lb = elm_label_add(bx);
      char buf[PATH_MAX];
      snprintf(buf, sizeof(buf), "Set default NOTE textsize: current: %i", ci_default_notefontsize);
      elm_object_text_set(lb, gettext(buf));
      evas_object_size_hint_align_set(lb, 0, 0);
      evas_object_show(lb);
      elm_box_pack_end(bx, lb);


      en_notetextpreview = elm_entry_add(bx);
      en_titletextpreview = elm_entry_add(bx);
      printf("SIZE: %i\n", ci_default_notefontsize);


      sl_fontsize = elm_slider_add(bx);
      elm_slider_min_max_set(sl_fontsize, 5, 40);
      evas_object_size_hint_align_set(sl_fontsize, EVAS_HINT_FILL, 0.5);
      evas_object_size_hint_weight_set(sl_fontsize, EVAS_HINT_EXPAND, 0);
      elm_slider_indicator_format_set(sl_fontsize, "%1.0f");
      step = _step_size_calculate(0, 5);
      elm_slider_step_set(sl_fontsize, 0.02);
      elm_slider_value_set(sl_fontsize, ci_default_notefontsize);
      evas_object_smart_callback_add(sl_fontsize, "changed", _note_textsize_preview_cb, en_notetextpreview);

      evas_object_show(sl_fontsize);
      elm_box_pack_end(bx, sl_fontsize);

      lb = elm_label_add(bx);
      snprintf(buf, sizeof(buf), "Set default TITLE textsize: current: %i", ci_default_titlefontsize);
      elm_object_text_set(lb, gettext(buf));
      evas_object_size_hint_align_set(lb, 0, 0);
      evas_object_show(lb);
      elm_box_pack_end(bx, lb);

      sl_titlesize = elm_slider_add(bx);
      elm_slider_min_max_set(sl_titlesize, 5, 40);
      evas_object_size_hint_align_set(sl_titlesize, EVAS_HINT_FILL, 0.5);
      evas_object_size_hint_weight_set(sl_titlesize, EVAS_HINT_EXPAND, 0);
      elm_slider_indicator_format_set(sl_titlesize, "%1.0f");
      step = _step_size_calculate(0, 5);
      elm_slider_step_set(sl_titlesize, 0.02);
      elm_slider_value_set(sl_titlesize, ci_default_titlefontsize);
      evas_object_smart_callback_add(sl_titlesize, "changed", _title_textsize_preview_cb, en_titletextpreview);

      evas_object_show(sl_titlesize);
      elm_box_pack_end(bx, sl_titlesize);


      _note_textsize_preview_cb(en_notetextpreview, sl_fontsize, NULL);
      _title_textsize_preview_cb(en_titletextpreview, sl_titlesize, NULL);

      elm_entry_editable_set(en_notetextpreview, EINA_TRUE);
      elm_entry_single_line_set(en_notetextpreview, EINA_TRUE);
      elm_object_text_set(en_notetextpreview, "Notetext");
      evas_object_show(en_notetextpreview);
      elm_box_pack_end(bx, en_notetextpreview);

      elm_entry_editable_set(en_titletextpreview, EINA_TRUE);
      elm_entry_single_line_set(en_titletextpreview, EINA_TRUE);
      elm_object_text_set(en_titletextpreview, "Titeltext");
      evas_object_show(en_titletextpreview);
      elm_box_pack_end(bx, en_titletextpreview);





      evas_object_show(bx);

      elm_object_content_set(advanced_frame, bx);
      evas_object_data_set(tb_settings, "advanced_frame", advanced_frame);
      /// ADVANCED FRAME END ///

      evas_object_event_callback_add(win_s, EVAS_CALLBACK_MOUSE_OUT, catlist_to_catlisteet, NULL);
      evas_object_event_callback_add(win_s, EVAS_CALLBACK_KEY_DOWN, _key_down_settings, win_s);
//       evas_object_event_callback_add(list, EVAS_CALLBACK_KEY_DOWN, _esc_check, NULL);

      elm_win_resize_object_add(win_s, tb_settings);
      evas_object_resize(win_s, 550, 0);

      _config_show_categories(tb_settings, NULL, NULL);

      elm_win_center(win_s, EINA_TRUE, EINA_TRUE);
      evas_object_show(win_s);
      elm_object_focus_set(win_s, EINA_TRUE);
      settings_win = win_s;
      settings_on = 1;
   } else
   {
      settings_on = 0;
      evas_object_del(settings_win);
      settings_win = NULL;
   }
}
