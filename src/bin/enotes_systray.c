
// FIXME: systray has no proper legacy API and clearly isn't ready for prime
// time with EO either.
#include "enotes.h"




#define WIDTH  320
#define HEIGHT 160

void 
_open_settings_systray(void *data, Evas_Object *obj, void *event_info)
{
   _open_settings(NULL, NULL, NULL, NULL);
}

void 
enotes_win_help_systray(void *data, Evas_Object *obj, void *event_info)
{
   enotes_win_help(NULL, NULL, NULL, NULL);
}

static Eina_Bool
_ev_handler(void *data EINA_UNUSED,
            int type EINA_UNUSED,
            void *event EINA_UNUSED)
{
//    Eina_Bool ret;

//    printf("systray ready event\n");

   elm_systray_register(item);
//    printf("Item Registration: ");
//    if (ret)
//      printf("OK!\n");
//    else
//      printf("Failed!\n");

   return ECORE_CALLBACK_DONE;
}


void
_switch_category_systray(void *data, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   Note* list_data;
   Eina_List *l;
   Eina_List *tmp = NULL;

   activ_cat = data;


   EINA_LIST_FOREACH(note_list, l, list_data)
   {
      if((list_data->categories != NULL) && ((strcmp(list_data->categories, data) == 0) || (strcmp(list_data->categories, "") == 0)))
      {
         tmp = eina_list_append(tmp, (void*)(intptr_t)list_data->id);
      }
   }

   if(eina_list_count(tmp) != 0)
      update_visible_notes();
   else
      _enotes_new();

   eina_list_free(tmp);
   enotes_systray();
}


void
enotes_systray()
{
   Evas_Object *win, *ic;
   Elm_Object_Item *it1 = NULL;
   Elm_Object_Item *it2 = NULL;
   

//    Eina_Bool init_ok = elm_need_systray();
   elm_need_systray();

   ecore_event_handler_add(ELM_EVENT_SYSTRAY_READY,
                           _ev_handler, NULL);

   win = elm_win_add(NULL, "Systray", ELM_WIN_BASIC);
   elm_policy_set(ELM_POLICY_QUIT, ELM_POLICY_QUIT_LAST_WINDOW_CLOSED);
   elm_win_title_set(win, "Systemtray - Status Notifier Item");
   elm_win_autodel_set(win, EINA_TRUE);
 
   
   it = elm_menu_add(win);
       Eina_Strbuf* buf2, *hide_label;
    buf2 = eina_strbuf_new();
    hide_label = eina_strbuf_new();
    
    eina_strbuf_append_printf(buf2, "%s/images/enotes_hideshow.png", elm_app_data_dir_get());
    printf("IMAGE %s\n", eina_strbuf_string_get(buf2));
    
//    char buf2[PATH_MAX];
//    snprintf(buf2, sizeof(buf2), "/usr/local/share/enotes/images/enotes_hideshow.png", elm_app_data_dir_get());
    
   if(all_hidden == EINA_TRUE)
      eina_strbuf_append(hide_label, "Show all Notes");
   else
      eina_strbuf_append(hide_label, "Hide all Notes");
     
   
   it2 = elm_menu_item_add(it, NULL, "enotes", "Hide/Show all Notes", _hide_show_all_notes, it);
//       it1 = elm_menu_item_add(it, NULL, NULL, eina_strbuf_string_get(hide_label), _hide_show_all_notes, it1);
      
//       ic = elm_image_add(win);
//       elm_image_file_set(ic, "/usr/local/share/enotes/images/enotes_hideshow.png", NULL);
//       evas_object_show(ic);
//       elm_object_item_part_content_set(it1, NULL, ic);
//     eina_strbuf_free(buf2);
   elm_menu_item_separator_add(it, NULL);
 
   
   elm_menu_item_separator_add(it, NULL);
   
   elm_menu_item_add(it, NULL, "enotes", "New Note", _enotes_new, NULL); // TODO F??R ALLE DREI: FUNKTIONS VORGABEN BEACHTEN
   elm_menu_item_add(it, NULL, "media-eject", "Settings", _open_settings_systray, NULL);
   elm_menu_item_add(it, NULL, "help-about", "Help", enotes_win_help_systray, NULL);
   
   elm_menu_item_separator_add(it, NULL);
   
   Eina_List *l1;
   My_Conf_Type_Cat *new;
   new = calloc(1, sizeof(My_Conf_Type_Cat));
   char buf_cat[1024];

   EINA_LIST_FOREACH(cat_list_settings, l1, new)
   {
      if (!strcmp(new->cat_name, activ_cat))
      {
         elm_menu_item_add(it, NULL, "enotes", new->cat_name, _switch_category_systray, new->cat_name);
          // FIXME: systray menu needs to be updated on category switch
         // it2 = elm_menu_item_add(it, NULL, "enotes", new->cat_name, NULL, NULL);
         // elm_object_item_disabled_set (it2, EINA_TRUE);
      }
      else
         elm_menu_item_add(it, NULL, NULL, new->cat_name, _switch_category_systray, new->cat_name);
   }

   elm_menu_item_separator_add(it, NULL);
   elm_menu_item_add(it, NULL, "window-close", "Close Enotes", _enotes_exit, NULL);

   // Status Notifier Item Handler   
   item = elm_systray_add(win);

   
   char buf[PATH_MAX];
   snprintf(buf, sizeof(buf), "%s/images/enotes.png", elm_app_data_dir_get());
   elm_systray_icon_name_set(item, buf);
   
   char buf1[PATH_MAX];
   snprintf(buf1, sizeof(buf1), "%s/images/enotes_bw.png", elm_app_data_dir_get());
   elm_systray_att_icon_name_set(item, buf1);
   
   elm_systray_menu_set(item, it);
   
   if(ci_systray == EINA_TRUE)
      elm_systray_status_set(item, 0);
   
   
//    char buf[PATH_MAX];

   
//    snprintf(buf, sizeof(buf), "%s/themes/enotes.edj", elm_app_data_dir_get());
   elm_systray_icon_theme_path_set(item, elm_app_data_dir_get());
   
   printf("THEME: %s\n", elm_systray_icon_theme_path_get(item));
      

//    evas_object_resize(win, WIDTH, HEIGHT);
//    evas_object_show(win);
}
