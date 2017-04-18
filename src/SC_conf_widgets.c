/*
 *  Filename: SC_conf_widgets.c 
 *  App Name: Silentcast <https://github.com/colinkeenan/silentcast>
 *  Copyright © 2017 Colin N Keenan <colinnkeenan@gmail.com>
 * 
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 3 of the License, or
 *  (at your option) any later version.
 *  
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Library General Public License for more details.
 *  
 *  You should have received a copy of the GNU General Public License
 *  along with this program. If not, see <http://www.gnu.org/licenses/>.
 *  
 * 
 *  Description: Everything related to the F1 widget which contains 3
 *  tabs: About Mouse Controls | Configuration | Rectangle Preferences 
 *  The standard "About" button is included on the About Mouse Controls
 *  tab. Configuration allows changing settings that are saved in
 *  silentcast.conf. Rectangle Preferences allows specifying which area
 *  the rectangle should surround on startup (saved in silentcast.conf),
 *  and changing preset sizes for the rectangle that are stored in 
 *  silentcast_presets.
 *
 */
#include "SC_conf_widgets.h"
char* SC_get_glib_filename (GtkWidget *widget, char filename[PATH_MAX]);

static void write_presets (GtkWidget *widget, double presets[PATH_MAX])
{
  char filename[PATH_MAX], contents[PRESET_N * 12], char_preset[11];
  snprintf (char_preset, 11, "%f", presets[0]);
  strcpy (contents, char_preset);
  for (int i=1; i<PRESET_N; i++) {
    strcat (contents, "\n");
    snprintf (char_preset, 11, "%f", presets[i]);
    strcat (contents, char_preset);
  }
  strcpy (filename, g_get_home_dir());
  strcat (filename, "/.config/silentcast_presets");
  char *glib_encoded_filename = SC_get_glib_filename (widget, filename);
  if (glib_encoded_filename) {
    g_file_set_contents (filename, contents, -1, NULL);
    g_free (glib_encoded_filename);
  }
}

static void write_conf (GtkWidget *widget, GtkEntryBuffer *entry_buffer, char area[2], unsigned int fps, gboolean anims_from_temp, 
    gboolean gif, gboolean pngs, gboolean webm, gboolean mp4)
{
  char conf[256+PATH_MAX] = "# Do not put spaces on either side of the =, and do not comment the working_dir line\nworking_dir="; char *l1comment = "\n";
  char *l2 = "area=", *l2comment = " #choices are i e c p for Interior, Entirety, Center, Previous\n";
  char *l3 = "fps=", *l3comment = " \n";
  char fps_char[4]; snprintf (fps_char, 4, "%u", fps); //fps will never be more than 999 and with null-terminator that's 4 digits
  char *l4 = "anims_from=", *l4comment = " #choices are \"temp.mkv\" or ew-???.png\n";
  char *l5 = "gif=", *l5comment = " \n";
  char *l6 = "pngs=", *l6comment = " \n";
  char *l7 = "webm=", *l7comment = " \n";
  char *l8 = "mp4=";
  char directory[PATH_MAX];
  strncpy (directory, gtk_entry_buffer_get_text (entry_buffer), PATH_MAX);

  strcat (conf, directory);
  strcat (conf, l1comment);
  strcat (conf, l2);
  strcat (conf, area);
  strcat (conf, l2comment);
  strcat (conf, l3);
  strcat (conf, fps_char);
  strcat (conf, l3comment);
  strcat (conf, l4);
  strcat (conf, anims_from_temp ? "temp.mkv" : "ew-???.png");
  strcat (conf, l4comment);
  strcat (conf, l5);
  strcat (conf, gif ? "TRUE" : "FALSE");
  strcat (conf, l5comment);
  strcat (conf, l6);
  strcat (conf, pngs ? "TRUE" : "FALSE");
  strcat (conf, l6comment);
  strcat (conf, l7);
  strcat (conf, webm ? "TRUE" : "FALSE");
  strcat (conf, l7comment);
  strcat (conf, l8);
  strcat (conf, mp4 ? "TRUE" : "FALSE");

  char filename[PATH_MAX];
  strcpy (filename, g_get_home_dir());
  strcat (filename, "/.config/silentcast.conf");
  char *glib_encoded_filename = SC_get_glib_filename (widget, filename);
  if (glib_encoded_filename) {
    g_file_set_contents (filename, conf, -1, NULL);
    g_free (glib_encoded_filename);
  }
}

static void show_about_cb (GtkWidget *widget, gpointer  data)
{    
  const gchar *authors[] = { "Colin N Keenan <colinnkeenan@gmail.com>", NULL };
  gtk_show_about_dialog(GTK_WINDOW(gtk_widget_get_toplevel(widget)),
                       "program-name", "Silentcast",
                        "authors", authors, "license-type", GTK_LICENSE_GPL_3_0,
                        "logo-icon-name", "silentcast",
                        "version", "v3.03",
                        "website", "https://github.com/colinkeenan/silentcast", "website-label", "Silentcast README.md on Github",
                        "copyright", "© 2017 Colin N Keenan <colinnkeenan@gmail.com>",
                        "wrap-license", TRUE, 
                        NULL);
}

static void change_w_dir_cb (GtkWidget *change_w_dir, gpointer data) 
{
  GtkWidget *widget = data; //used in the P macro

  GtkFileChooserAction action = GTK_FILE_CHOOSER_ACTION_SELECT_FOLDER;
  GtkWidget *get_working_dir = gtk_file_chooser_dialog_new ("Choose Working Directory for silentcast or silentcast/silentcast",
    GTK_WINDOW(gtk_widget_get_toplevel(change_w_dir)), action, ("Cancel"), GTK_RESPONSE_CANCEL, ("Open"), GTK_RESPONSE_ACCEPT, NULL);
  int res = gtk_dialog_run (GTK_DIALOG (get_working_dir));
  if (res == GTK_RESPONSE_ACCEPT) {
    char *dir = gtk_file_chooser_get_filename (GTK_FILE_CHOOSER (get_working_dir));
    if (dir) {
      gtk_entry_buffer_set_text (P("working_dir"), dir, -1); 
      free (dir);
    }
  } 
  gtk_widget_destroy (get_working_dir);
}

#define T(A) gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON(A))
static void save (GtkWidget *save, gpointer data)
{
  GtkWidget *widget = data; //widget is used in the P macro
  GtkWidget *f1_widget = gtk_widget_get_toplevel(save); //f1_widget is used in the PF1 macro

  int *p_fps = P("p_fps");
  gboolean *p_anims_from_temp = P("p_anims_from_temp"), *p_gif = P("p_gif"), *p_pngs = P("p_pngs"),
           *p_webm = P("p_webm"), *p_mp4 = P("p_mp4");

  // working_dir already defined in change_w_dir_cb

  // fps
  *p_fps = gtk_spin_button_get_value_as_int (GTK_SPIN_BUTTON(PF1("fps_spin_button")));

  // videos made from
  char *choice = gtk_combo_box_text_get_active_text (GTK_COMBO_BOX_TEXT(PF1("from_mkv_combobutt")));
  *p_anims_from_temp = !strcmp (choice, "temp.mkv");
  g_free (choice);
  
  // final outputs
  *p_gif = T(PF1("gif_checkbox"));
  *p_pngs = T(PF1("png_checkbox"));
  *p_webm = T(PF1("webm_checkbox"));
  *p_mp4 = T(PF1("mp4_checkbox"));

  //area
  if (T(PF1("active_win_radiobutt")) && !T(PF1("extents_checkbox"))) strcpy (P("area"), "i");
  else if (T(PF1("center_radiobutt"))) strcpy (P("area"), "c");
  else if (T(PF1("previous_radiobutt"))) strcpy (P("area"), "p");
  else strcpy (P("area"), "e");

  write_conf (widget, P("working_dir"), P("area"), *p_fps, *p_anims_from_temp, *p_gif, *p_pngs, *p_webm, *p_mp4);
  write_presets (widget, P("presets"));
}

static void on_extents_checkbox_changed (GtkWidget *checkbox, gpointer data)
{
  GtkWidget *widget = data; //widget is used in P macro
  gboolean *p_include_extents = P("p_include_extents");
  *p_include_extents = T(checkbox);
  char *area = P("area");
  if (!strcmp (area, "e") || !strcmp (area, "i")) {
    if (*p_include_extents) *area = 'e';
    else *area = 'i';
  }
}

static void on_active_win_radiobutt_changed (GtkWidget *radiobutt, gpointer data)
{
  GtkWidget *widget = data; //widget is used in P macro
  GtkWidget *f1_widget = gtk_widget_get_toplevel(radiobutt); //f1_widget is used in the PF1 macro
  char *area = P("area");
  if (T(radiobutt)) {
    gtk_widget_set_sensitive (PF1("extents_checkbox"), TRUE);
    if (P("p_include_extents")) *area = 'e';
    else *area = 'i';
  } else gtk_widget_set_sensitive (PF1("extents_checkbox"), FALSE);
}


static void on_center_radiobutt_changed (GtkWidget *radiobutt, gpointer data)
{
  GtkWidget *widget = data; //widget is used in P macro
  char *area = P("area");
  if (T(radiobutt)) *area = 'c';
}

static void on_previous_radiobutt_changed (GtkWidget *radiobutt, gpointer data)
{
  GtkWidget *widget = data; //widget is used in P macro
  char *area = P("area");
  if (T(radiobutt)) *area = 'p';
}

static void on_w_spinbutt_changed (GtkWidget *spin, gpointer data)
{
  int i = *((int *) data);
  GtkWidget *widget = GTK_WIDGET(gtk_window_get_transient_for (GTK_WINDOW(gtk_widget_get_toplevel (spin)))); //widget is used in P macro
  double *presets = P("presets");
  double width = gtk_spin_button_get_value (GTK_SPIN_BUTTON(spin)), w=0;
  double height_fraction = modf (presets[i], &w);
  presets[i] = width + height_fraction;
}

static void on_h_spinbutt_changed (GtkWidget *spin, gpointer data)
{
  int i = *((int *) data);
  GtkWidget *widget = GTK_WIDGET(gtk_window_get_transient_for (GTK_WINDOW(gtk_widget_get_toplevel (spin)))); //widget is used in P macro
  double *presets = P("presets");
  double width = 0;
  modf (presets[i], &width);
  double height_fraction = gtk_spin_button_get_value (GTK_SPIN_BUTTON(spin)) / 100000;
  presets[i] = width + height_fraction;
}

double SC_get_w (double a) 
{
  double w;
  modf (a, &w);
  return w;
}

double SC_get_h (double a) 
{
  double w, h;
  h = 100000 * modf (a, &w);
  return rint (h);
}

#define SET(A, B) gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON(A), B)
static void update_area_prefs (GtkWidget *widget, GtkWidget *f1_widget)
{
  //strcmp is 0 when they match
  SET(PF1("extents_checkbox"), *((gboolean *)P("p_include_extents")));
  if (!strcmp (P("area"), "i")) {
    SET(PF1("active_win_radiobutt"), TRUE);
    SET(PF1("extents_checkbox"), FALSE);
    gtk_widget_set_sensitive (PF1("extents_checkbox"), TRUE);
  } else if (!strcmp (P("area"), "c")) {
    SET(PF1("center_radiobutt"), TRUE);
    //gray out the checkbox because it's only related to active_win_radiobutt
    gtk_widget_set_sensitive (PF1("extents_checkbox"), FALSE); 
  } else if (!strcmp (P("area"), "p")) {
    SET(PF1("previous_radiobutt"), TRUE);
    //gray out the checkbox because it's only related to active_win_radiobutt
    gtk_widget_set_sensitive (PF1("extents_checkbox"), FALSE);
  } else {
    SET(PF1("active_win_radiobutt"), TRUE);
    SET(PF1("extents_checkbox"), TRUE);
    gtk_widget_set_sensitive (PF1("extents_checkbox"), TRUE);
  }
}

/* 
 * The F1 Widget
 *
 * f1_widget is a window with a tabbed notbook (f1) with 3 tabs: 
 * About Mouse Controls|Configuration|Rectangle Preferences
 *
 */
#define PF1_SET(A) g_object_set_data (G_OBJECT(f1_widget), #A, A); // preprocessor changes #A to "A"

void SC_show_f1_widget (GtkApplication *app, GtkWidget *widget)
{
  GtkWidget *f1_widget = gtk_application_window_new (app);
  gtk_window_set_transient_for (GTK_WINDOW(f1_widget), GTK_WINDOW(widget));
  GtkWidget *f1 = gtk_notebook_new ();
  /* initialize About Mouse Controls widgets */
  GtkWidget *about_controls = gtk_grid_new ();
  gtk_grid_set_row_spacing (GTK_GRID(about_controls), ROW_SPACING);
  GtkWidget *about_label = gtk_label_new ("About Mouse Controls");
  GtkWidget *about_button = gtk_button_new_with_label ("About Silentcast");
  g_signal_connect (about_button, "clicked",
      G_CALLBACK(show_about_cb), NULL);

  /* initialize Configuration widgets */
  GtkWidget *config = gtk_grid_new ();
  gtk_grid_set_column_spacing (GTK_GRID(config), COL_SPACING);
  gtk_grid_set_row_spacing (GTK_GRID(config), ROW_SPACING);
  GtkWidget *config_label = gtk_label_new ("Configuration");
  GtkWidget *save_config = gtk_button_new_with_label ("Save");
  g_signal_connect (save_config, "clicked", 
     G_CALLBACK(save), widget);
  //working directory
  GtkWidget *view_w_dir_entry = gtk_entry_new_with_buffer (P("working_dir"));
  gtk_editable_set_editable (GTK_EDITABLE(view_w_dir_entry), FALSE);
  GtkWidget *change_w_dir = gtk_button_new_from_icon_name ("folder-open", GTK_ICON_SIZE_BUTTON);
  g_signal_connect (change_w_dir, "clicked", 
     G_CALLBACK(change_w_dir_cb), widget);
  //Configuration button widgets
  static GtkWidget *fps_spin_button = NULL, *gif_checkbox = NULL, *png_checkbox = NULL, 
                   *webm_checkbox = NULL, *mp4_checkbox = NULL, *from_mkv_combobutt = NULL;
  //fps_spin_button
  int *p_fps = P("p_fps");
  GtkAdjustment *adjustment = gtk_adjustment_new (*p_fps, 0.0, 120.0, 1.0, 5.0, 0.0);
  fps_spin_button = gtk_spin_button_new (adjustment, 1.0, 0);
  //final outputs checkboxes
  gboolean *p_gif = P("p_gif"), *p_pngs = P("p_pngs"), *p_webm = P("p_webm"), *p_mp4 = P("p_mp4");
  gif_checkbox = gtk_check_button_new_with_label ("anim.gif");
  png_checkbox = gtk_check_button_new_with_label ("ew-???.png images");
  webm_checkbox = gtk_check_button_new_with_label ("anim.webm");
  mp4_checkbox = gtk_check_button_new_with_label ("anim.mp4");
  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON(gif_checkbox), *p_gif);
  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON(png_checkbox), *p_pngs);
  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON(webm_checkbox), *p_webm);
  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON(mp4_checkbox), *p_mp4);
  //from_mkv_combobutt
  gboolean *p_anims_from_temp = P("p_anims_from_temp");
  from_mkv_combobutt = gtk_combo_box_text_new ();
  gtk_combo_box_text_append (GTK_COMBO_BOX_TEXT(from_mkv_combobutt), NULL, "temp.mkv"); 
  gtk_combo_box_text_append (GTK_COMBO_BOX_TEXT(from_mkv_combobutt), NULL, "ew-???.png"); 
  gtk_combo_box_set_active (GTK_COMBO_BOX(from_mkv_combobutt), !*p_anims_from_temp); //want index 0 when want temp... i.e. !anims.., 1 for ew...
  //PF1_SET the Configuration Widgets to be recovered in other functions using PF1
  PF1_SET(fps_spin_button); PF1_SET(gif_checkbox); PF1_SET(png_checkbox); PF1_SET(webm_checkbox); PF1_SET(mp4_checkbox); PF1_SET(from_mkv_combobutt);

  /* initialize Rectangle Preferences widgets */
  GtkWidget *prefs = gtk_grid_new ();
  gtk_grid_set_column_spacing (GTK_GRID(prefs), COL_SPACING);
  gtk_grid_set_row_spacing (GTK_GRID(prefs), ROW_SPACING);
  GtkWidget *prefs_label = gtk_label_new ("Rectangle Preferences");
  //radiobuttons
  static GtkWidget *active_win_radiobutt = NULL, *center_radiobutt = NULL, *previous_radiobutt = NULL;
  active_win_radiobutt = gtk_radio_button_new_with_label (NULL, "Active window");
  g_signal_connect (active_win_radiobutt, "toggled", 
     G_CALLBACK(on_active_win_radiobutt_changed), widget);
  center_radiobutt = gtk_radio_button_new_with_label (
      gtk_radio_button_get_group (GTK_RADIO_BUTTON(active_win_radiobutt)),
      "Center fourth of screen");
  g_signal_connect (center_radiobutt, "toggled", 
     G_CALLBACK(on_center_radiobutt_changed), widget);
  previous_radiobutt = gtk_radio_button_new_with_label (
      gtk_radio_button_get_group (GTK_RADIO_BUTTON(center_radiobutt)),
      "Previously recorded area");
  g_signal_connect (previous_radiobutt, "toggled", 
     G_CALLBACK(on_previous_radiobutt_changed), widget);
  PF1_SET(active_win_radiobutt); PF1_SET(center_radiobutt); PF1_SET(previous_radiobutt);
  //arrays of spinbuttons displaying presests
  static GtkWidget *w_spinbutt[PRESET_N], *h_spinbutt[PRESET_N];
  double *presets = P("presets");
  GtkAdjustment *adjstmnt_w[PRESET_N];
  static int j[PRESET_N];
  for (int i = 0; i < PRESET_N; i++) j[i] = i; //array of indexes to pass i as gpointer data
  for (int i = 0; i < PRESET_N; i++) {
    adjstmnt_w[i] = gtk_adjustment_new (SC_get_w (presets[i]), 0.0, 9999.0, 1.0, 5.0, 0.0);
    w_spinbutt[i] = gtk_spin_button_new (adjstmnt_w[i], 1.0, 0);
    g_signal_connect (w_spinbutt[i], "value-changed", 
       G_CALLBACK(on_w_spinbutt_changed), &j[i]);
  }
  GtkAdjustment *adjstmnt_h[PRESET_N];
  for (int i = 0; i < PRESET_N; i++) {
    adjstmnt_h[i] = gtk_adjustment_new (SC_get_h (presets[i]), 0.0, 9999.0, 1.0, 5.0, 0.0);
    h_spinbutt[i] = gtk_spin_button_new (adjstmnt_h[i], 1.0, 0);
    g_signal_connect (h_spinbutt[i], "value-changed", 
       G_CALLBACK(on_h_spinbutt_changed), &j[i]);
  }
  PF1_SET(w_spinbutt); PF1_SET(h_spinbutt);
  //extents_checkbox
  GtkWidget *extents_checkbox = NULL;
  extents_checkbox = gtk_check_button_new_with_label ("with title and borders");
  g_signal_connect (extents_checkbox, "toggled", 
     G_CALLBACK(on_extents_checkbox_changed), widget);
  PF1_SET(extents_checkbox);

  GtkWidget *save_prefs = gtk_button_new_with_label ("Save");
  g_signal_connect (save_prefs, "clicked", 
     G_CALLBACK(save), widget);

  /* title f1_widget and put the notebook, f1, into it */
  gtk_window_set_title (GTK_WINDOW(f1_widget), "Silentcast F1");
  gtk_container_add (GTK_CONTAINER(f1_widget), f1);

  /* insert the 3 tabs */
  gtk_notebook_insert_page (GTK_NOTEBOOK(f1), about_controls, about_label, -1);
  gtk_notebook_insert_page (GTK_NOTEBOOK(f1), config, config_label, -1);
  gtk_notebook_insert_page (GTK_NOTEBOOK(f1), prefs, prefs_label, -1);

  /* About tab layout */
  GtkWidget *lclabel = gtk_label_new (NULL); gtk_widget_set_halign (lclabel, GTK_ALIGN_END);
  gtk_widget_set_valign (lclabel, GTK_ALIGN_START);
  gtk_label_set_markup (GTK_LABEL(lclabel), BGN_SCTN"left-click/drag: "END_SCTN);
  GtkWidget *lcexpln = gtk_label_new ("Move rectangle by the top-left corner");
  gtk_widget_set_halign (lcexpln, GTK_ALIGN_START);

  GtkWidget *rclabel = gtk_label_new (NULL); gtk_widget_set_halign (rclabel, GTK_ALIGN_END);
  gtk_widget_set_valign (rclabel, GTK_ALIGN_START);
  gtk_label_set_markup (GTK_LABEL(rclabel), BGN_SCTN"right-click/drag: "END_SCTN);
  GtkWidget *rcexpln = gtk_label_new ("Resize the rectangle by the bottom-right corner");
  gtk_widget_set_halign (rcexpln, GTK_ALIGN_START);

  GtkWidget *mclabel = gtk_label_new (NULL); gtk_widget_set_halign (mclabel, GTK_ALIGN_END);
  gtk_widget_set_valign (mclabel, GTK_ALIGN_START);
  gtk_label_set_markup (GTK_LABEL(mclabel), BGN_SCTN"middle-click: "END_SCTN);
  GtkWidget *mcexpln = gtk_label_new ("Resize rectangle to include/not include title and \nborders of the original active window");
  gtk_widget_set_halign (mcexpln, GTK_ALIGN_START);

  GtkWidget *mdlabel = gtk_label_new (NULL); gtk_widget_set_halign (mdlabel, GTK_ALIGN_END);
  gtk_widget_set_valign (mdlabel, GTK_ALIGN_START);
  gtk_label_set_markup (GTK_LABEL(mdlabel), BGN_SCTN"middle-drag: "END_SCTN);
  GtkWidget *mdexpln = gtk_label_new ("Resize rectangle to the largest fitting preset \nbetween top left corner and mouse pointer");
  gtk_widget_set_halign (mdexpln, GTK_ALIGN_START);

  GtkWidget *swlabel = gtk_label_new (NULL); gtk_widget_set_halign (swlabel, GTK_ALIGN_END);
  gtk_widget_set_valign (swlabel, GTK_ALIGN_START);
  gtk_label_set_markup (GTK_LABEL(swlabel), BGN_SCTN"scroll wheel: "END_SCTN);
  GtkWidget *swexpln = gtk_label_new ("Resize rectangle to the next preset size");
  gtk_widget_set_halign (swexpln, GTK_ALIGN_START);

  GtkWidget * f2expln = gtk_label_new ("        If the F2 \"Resize active window to match\" checkbox is active, the active window will be resized\n\
        along with the rectangle");

#define ABOUT_ATTACH(A,C,R,W) gtk_grid_attach (GTK_GRID(about_controls), A, C, R, W, 1) 
                                                                               ABOUT_ATTACH(about_button, 6, 0, 1);
                ABOUT_ATTACH(lclabel, 1, 1, 1); ABOUT_ATTACH(lcexpln, 2, 1, 4);
                ABOUT_ATTACH(rclabel, 1, 2, 1); ABOUT_ATTACH(rcexpln, 2, 2, 4);
                ABOUT_ATTACH(mclabel, 1, 3, 1); ABOUT_ATTACH(mcexpln, 2, 3, 4);
                ABOUT_ATTACH(mdlabel, 1, 4, 1); ABOUT_ATTACH(mdexpln, 2, 4, 4);
                ABOUT_ATTACH(swlabel, 1, 5, 1); ABOUT_ATTACH(swexpln, 2, 5, 4);
                ABOUT_ATTACH(f2expln, 1, 6, 5);

  /* Configuration tab layout */
  GtkWidget *fps_label = gtk_label_new ("Frames per second"); gtk_widget_set_halign (fps_label, GTK_ALIGN_END);
  GtkWidget *from_label = gtk_label_new ("Videos made from:"); gtk_widget_set_halign (from_label, GTK_ALIGN_END);
  GtkWidget *out_label = gtk_label_new ("Final Outputs"); gtk_widget_set_halign (out_label, GTK_ALIGN_START);
  GtkWidget *dir_label = gtk_label_new ("Working Directory"); gtk_widget_set_halign (dir_label, GTK_ALIGN_END);
  
#define CONFIG_ATTACH(A,C,R,W) gtk_grid_attach (GTK_GRID(config), A, C, R, W, 1) 
                                                
  CONFIG_ATTACH(gtk_label_new ("        "), 0, 0, 1);                                                        CONFIG_ATTACH(save_config, 8, 0, 1); 
                   CONFIG_ATTACH(out_label, 1, 1, 2);                                                                                 
                CONFIG_ATTACH(gif_checkbox, 1, 2, 1);
                CONFIG_ATTACH(png_checkbox, 1, 3, 1);
               CONFIG_ATTACH(webm_checkbox, 1, 4, 1);       CONFIG_ATTACH(fps_label, 2, 4, 1);    CONFIG_ATTACH(fps_spin_button, 3, 4, 1); 
                CONFIG_ATTACH(mp4_checkbox, 1, 5, 1);      CONFIG_ATTACH(from_label, 2, 5, 1); CONFIG_ATTACH(from_mkv_combobutt, 3, 5, 1);         
                   CONFIG_ATTACH(dir_label, 1, 6, 1);CONFIG_ATTACH(view_w_dir_entry, 2, 6, 5);       CONFIG_ATTACH(change_w_dir, 7, 6, 1);  

  /* Preferences tab layout */
  GtkWidget *sel_area_label = gtk_label_new (NULL);
  gtk_label_set_markup (GTK_LABEL(sel_area_label), BGN_SCTN"What area should the rectangle surround on startup?"END_SCTN);
  gtk_widget_set_halign (sel_area_label, GTK_ALIGN_START);
#define PRFS_ATCH(A,C,R,W) gtk_grid_attach (GTK_GRID(prefs), A, C, R, W, 1) 
  PRFS_ATCH(gtk_label_new ("        "), 0, 0, 1);                                           PRFS_ATCH(save_prefs, 6, 0, 1);  
  PRFS_ATCH(sel_area_label, 1, 1, 4);
  PRFS_ATCH(active_win_radiobutt, 2, 2, 2);   PRFS_ATCH(extents_checkbox, 4, 2, 1);
  PRFS_ATCH(center_radiobutt, 2, 3, 3);
  PRFS_ATCH(previous_radiobutt, 2, 4, 3);

  GtkWidget *edit_presets_label = gtk_label_new (NULL);
  gtk_label_set_markup (GTK_LABEL(edit_presets_label), BGN_SCTN"What preset sizes should be used with the scroll wheel and middle drag?"END_SCTN);
  gtk_widget_set_halign (edit_presets_label, GTK_ALIGN_START);
  GtkWidget *w1_label = gtk_label_new ("Width"); gtk_widget_set_halign (w1_label, GTK_ALIGN_START);
  GtkWidget *w2_label = gtk_label_new ("Width"); gtk_widget_set_halign (w2_label, GTK_ALIGN_START);
  GtkWidget *h1_label = gtk_label_new ("Height"); gtk_widget_set_halign (h1_label, GTK_ALIGN_START);
  GtkWidget *h2_label = gtk_label_new ("Height"); gtk_widget_set_halign (h2_label, GTK_ALIGN_START);

  PRFS_ATCH(gtk_label_new (" "), 1, 5, 1); //blank row before Edit preset sizes
  PRFS_ATCH(edit_presets_label, 1, 6, 5);
  //verical separator at column 3
  gtk_grid_attach (GTK_GRID(prefs), gtk_separator_new (GTK_ORIENTATION_VERTICAL), 3, 7, 1, 9);
       PRFS_ATCH(w1_label,1,7 ,1);     PRFS_ATCH(h1_label,2,7 ,1);      PRFS_ATCH(w2_label,4,7 ,1);      PRFS_ATCH(h2_label,5,7 ,1);
  PRFS_ATCH(w_spinbutt[0],1,8 ,1);PRFS_ATCH(h_spinbutt[0],2,8 ,1);PRFS_ATCH(w_spinbutt[8 ],4,8 ,1);PRFS_ATCH(h_spinbutt[8 ],5,8 ,1);  
  PRFS_ATCH(w_spinbutt[1],1,9 ,1);PRFS_ATCH(h_spinbutt[1],2,9 ,1);PRFS_ATCH(w_spinbutt[9 ],4,9 ,1);PRFS_ATCH(h_spinbutt[9 ],5,9 ,1);  
  PRFS_ATCH(w_spinbutt[2],1,10,1);PRFS_ATCH(h_spinbutt[2],2,10,1);PRFS_ATCH(w_spinbutt[10],4,10,1);PRFS_ATCH(h_spinbutt[10],5,10,1);  
  PRFS_ATCH(w_spinbutt[3],1,11,1);PRFS_ATCH(h_spinbutt[3],2,11,1);PRFS_ATCH(w_spinbutt[11],4,11,1);PRFS_ATCH(h_spinbutt[11],5,11,1);  
  PRFS_ATCH(w_spinbutt[4],1,12,1);PRFS_ATCH(h_spinbutt[4],2,12,1);PRFS_ATCH(w_spinbutt[12],4,12,1);PRFS_ATCH(h_spinbutt[12],5,12,1);  
  PRFS_ATCH(w_spinbutt[5],1,13,1);PRFS_ATCH(h_spinbutt[5],2,13,1);PRFS_ATCH(w_spinbutt[13],4,13,1);PRFS_ATCH(h_spinbutt[13],5,13,1);  
  PRFS_ATCH(w_spinbutt[6],1,14,1);PRFS_ATCH(h_spinbutt[6],2,14,1);PRFS_ATCH(w_spinbutt[14],4,14,1);PRFS_ATCH(h_spinbutt[14],5,14,1);  
  PRFS_ATCH(w_spinbutt[7],1,15,1);PRFS_ATCH(h_spinbutt[7],2,15,1);PRFS_ATCH(w_spinbutt[15],4,15,1);PRFS_ATCH(h_spinbutt[15],5,15,1);  
  PRFS_ATCH(gtk_label_new (" "),1,16,1); //blank row at bottom
  
  gtk_widget_show_all(f1_widget);
  update_area_prefs (widget, f1_widget); //have to do this after show_all or else get error that extents_checkbox 
                                         //isn't a widget when set sensitive TRUE or FALSE
}

