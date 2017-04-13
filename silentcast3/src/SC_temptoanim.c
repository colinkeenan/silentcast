/*
 *  Filename: SC_temptoanim.c 
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
 *  Description: Convert temp.mkv or *.png created by Silentcast into
 *  an animated gif or movie according to configuration settings.
 *
 */
#include "SC_temptoanim.h"
#define P(A) g_object_get_data (G_OBJECT(widget), A)

void SC_show_error (GtkWidget *widget, char *err_message) 
{
  GtkWidget *dialog = 
    gtk_message_dialog_new (GTK_WINDOW(widget), 
        GTK_DIALOG_DESTROY_WITH_PARENT, 
        GTK_MESSAGE_ERROR, GTK_BUTTONS_CLOSE, "%s", err_message);
  gtk_dialog_run (GTK_DIALOG (dialog));
  gtk_widget_destroy (dialog);
}

void SC_show_err_message (GtkWidget *widget, char *message, char *errmessage)
{
  char err_message[1200];
  strcpy (err_message, message);
  strcat (err_message, errmessage);
  fprintf (stderr, "\n%s", err_message);
  SC_show_error (widget, err_message);
}

static void show_file_err (GtkWidget *widget, char filename[PATH_MAX], char *errmessage)
{
  char err_message[PATH_MAX + 200];
  strcpy (err_message, "Error: ");
  strcat (err_message, filename);
  strcat (err_message, errmessage);
  fprintf (stderr, "%s", err_message);
  SC_show_error (widget, err_message);
}

char* SC_get_glib_filename (GtkWidget *widget, char filename[PATH_MAX])
{
  char *glib_filename;
  GError *err = NULL;
  gsize bytes_written = 0;
  glib_filename = g_filename_from_utf8 (filename, -1, NULL, &bytes_written, &err);
  if (err) {
    SC_show_err_message (widget, "Error getting glib encoded filename: ", err->message);
    g_error_free (err);
  }
  return glib_filename;
}

static gboolean is_file (GtkWidget *widget, char filename[PATH_MAX], char *errmessage)
{
  char *glib_encoded_filename = SC_get_glib_filename (widget, filename);
  if (!glib_encoded_filename) return FALSE;
  else if (!g_file_test (glib_encoded_filename, G_FILE_TEST_IS_REGULAR)) {
    show_file_err (widget, filename, errmessage);
    g_free (glib_encoded_filename);
    return FALSE;
  }
  g_free (glib_encoded_filename);
  return TRUE;
}

static gboolean temp_exists (GtkWidget *widget, char silentcast_dir[PATH_MAX])
{
  char filename[PATH_MAX];
  strcpy (filename, silentcast_dir);
  strcat (filename, "/temp.mkv");
  return is_file (widget, filename, "temp.mkv not found, so can't generate anything from it");
}

//kept getting segfault in trying to use qsort, so found this bubblesort here: 
//http://stackoverflow.com/questions/6631355/sorting-alphabetically-in-c-using-strcmp
static void sort_pngs (char **png, int pngc) {
  int limit = pngc - 1;
  gboolean did_swap = TRUE;
  char *temp;

  while (did_swap) {
    did_swap = FALSE;
    for (int i = 0; i < limit; i++) {
      if (strcmp (png[i], png[i+1]) > 0) {
        temp = png[i];
        png[i] = png[i+1];
        png[i+1] = temp;
        did_swap = TRUE;
      }
    }
    limit--;
  }
}

static char** get_array_of_pngs (GtkWidget *widget, char silentcast_dir[PATH_MAX]) {
  char *glib_encoded_silentcast_dir = SC_get_glib_filename (widget, silentcast_dir);
  if (glib_encoded_silentcast_dir) {
    GDir *directory = g_dir_open (glib_encoded_silentcast_dir, 0, NULL);
    if (directory) {
      GPatternSpec *png_pattern = g_pattern_spec_new ("ew-???.png");
      const char *filename = NULL;
      char path_and_file[PATH_MAX];
      int i = 0;
      char **png = malloc (1000 * sizeof (char*)); //there are at most 999 pngs and I'm leaving room for NULL to indicate end of list
      while ( (filename = g_dir_read_name (directory)) ) { //filename should not be modified or freed
        strcpy (path_and_file, silentcast_dir);
        if (g_pattern_match (png_pattern, strlen(filename), filename, NULL)) {
          strcat (path_and_file, "/");
          strcat (path_and_file, filename);
          char *glib_encoded_filename = SC_get_glib_filename (widget, path_and_file);
          if (glib_encoded_filename) {
            png[i] = malloc (strlen (glib_encoded_filename));
            strcpy (png[i], glib_encoded_filename);
            g_free (glib_encoded_filename);
            i++;
          }
        }
      }
      g_pattern_spec_free (png_pattern);
      g_dir_close (directory);
      sort_pngs (png, i);
      png[i] = NULL;
      return png;
    }
    g_free (glib_encoded_silentcast_dir);
  } 
  return NULL;
}

static void delete_pngs (GtkWidget *widget, char silentcast_dir[PATH_MAX], int group)
{
  if (group != 1) { //don't need to do anything if group is 1 because that means keep everything
    char **png_arr = get_array_of_pngs (widget, silentcast_dir); //returns sorted array of pngs in silentcast_dir, ending with NULL
    int i = 0;
    while (png_arr[i]) {
      //Keep 1 out of group, so 1 = keep all, 2 = keep every other, 3 = keep 1 out of 3 etc.
      //Also, 0 = don't keep any. If group isn't 0, always keep the 2nd one (i = 1) and
      //every +group after that (keep i = 1, i = 1+group, i = 1+2*group etc. In other words,
      //keep when i == 1 or when i % group == 1. Delete when i != 1 && i%group != 1)
      if (group == 0 || (i % group != 1 && i != 1)) 
          if (g_remove (png_arr[i]) == -1) fprintf (stderr, "\ncouldn't remove %s", png_arr[i]); 
      g_free (png_arr[i]);
      i++;
    }
    g_free (png_arr);
  }
}

static gboolean animgif_exists (GtkWidget *widget, char silentcast_dir[PATH_MAX])
{
  char filename[PATH_MAX];
  strcpy (filename, silentcast_dir);
  strcat (filename, "/anim.gif");
  char warning_if_no_file[] = 
    "Too many images for the available memory. Try closing other applications, creating a swap file, or removing unecessary images.";

  return is_file (widget, filename, warning_if_no_file);
}

static gboolean on_value_changed_group (GtkSpinButton *group_spnbutt, gpointer data)
{
  int *p_group = data;
  *p_group  = gtk_spin_button_get_value_as_int (group_spnbutt);
  return TRUE;
}

static void get_silentcast_dir (GtkWidget *widget, char silentcast_dir[PATH_MAX])
{
  strcpy (silentcast_dir, gtk_entry_buffer_get_text (P("working_dir")));
  strcat (silentcast_dir, "/silentcast");
}

//executed when done button is clicked from show_edit_pngs
static gboolean make_anim_gif_cb (GtkWidget *done, gpointer data) {
  char convert_com[200], delay[5];
  GtkWidget *widget = GTK_WIDGET(gtk_window_get_transient_for (GTK_WINDOW(gtk_widget_get_toplevel(done))));
  int *p_group = P("p_group"), *p_total_group = P("p_total_group"), *p_fps = P("p_fps");

  char silentcast_dir[PATH_MAX];
  get_silentcast_dir (widget, silentcast_dir);
  delete_pngs (widget, silentcast_dir, *p_group); //delete all but 1 out of every group number of pngs (group set in edit_pngs)

  //construct convert_com: convert -adjoin -delay (total_group * fps) -layers optimize ew-[0-9][0-9][0-9].png anim.gif
  *p_total_group = *p_total_group + *p_group;
  strcpy (convert_com, "/bin/sh -c '/usr/bin/convert -adjoin -delay ");
  snprintf (delay, 5, "%d", *p_total_group * *p_fps);
  strcat (convert_com, delay);
  strcat (convert_com, " -layers optimize ew-[0-9][0-9][0-9].png anim.gif'");
  //spawn it
  GPid convert_com_pid = 0;
  SC_spawn (widget, convert_com, &convert_com_pid, "Making anim.gif from pngs.", "show_edit_pngs"); 
  return TRUE;
}

static void show_edit_pngs (GtkWidget *widget)
{
  GtkWidget *group_spnbutt = NULL;
  group_spnbutt = gtk_spin_button_new_with_range (1, 8, 1); 
  g_signal_connect (group_spnbutt, "value-changed", 
     G_CALLBACK(on_value_changed_group), P("p_group"));
  GtkWidget *group_spinbutt_label = gtk_label_new ("Prune pngs, keeping 1 out of every"); 
  gtk_widget_set_halign (group_spinbutt_label, GTK_ALIGN_END);
  GtkWidget *done= gtk_button_new_with_label ("Done");
  g_signal_connect (done, "clicked", 
     G_CALLBACK(make_anim_gif_cb), NULL);

  GtkWidget *edit_pngs_widget = gtk_application_window_new (gtk_window_get_application (GTK_WINDOW(widget)));
  gtk_window_set_transient_for (GTK_WINDOW(edit_pngs_widget), GTK_WINDOW(widget));
  gtk_window_set_title (GTK_WINDOW(edit_pngs_widget), "Silentcast Edit Pngs");
  gtk_window_set_modal (GTK_WINDOW(edit_pngs_widget), TRUE);
  GtkWidget *edit_pngs = gtk_grid_new ();
  gtk_container_add (GTK_CONTAINER(edit_pngs_widget), edit_pngs);
#define EDPNGS_ATCH(A,C,R,W) gtk_grid_attach (GTK_GRID(edit_pngs), A, C, R, W, 1) 
  EDPNGS_ATCH(group_spinbutt_label, 0, 0, 1); EDPNGS_ATCH(group_spnbutt, 1, 0, 1);
  EDPNGS_ATCH(gtk_label_new ("You can also manipulate the images manually in the file browser or other application."), 0, 1, 4);
  EDPNGS_ATCH(gtk_label_new ("Clicking done will prune the pngs (if indicated) and try to make anim.gif from what remains."), 0, 2, 4);
                                                                                                      EDPNGS_ATCH(done, 4, 3, 1);
  gtk_widget_show_all(edit_pngs_widget);
}

static void make_webm_from_temp (GtkWidget *widget)
{
  gboolean *p_webm = P("p_webm");
  if (*p_webm) {
    char ff_make_movie_com[200], message[35];
    //construct ff_make_movie_com: ffmpeg -i temp.mkv -c:v libvpx -qmin 0 -qmax 50 -crf 5 -b:v 749k anim.webm 
    //or:                          ffmpeg -i temp.mkv -pix_fmt yuv420p -vf "scale=trunc(iw/2)*2:trunc(ih/2)*2" anim.mp4 
    strcpy (ff_make_movie_com, "/bin/sh -c '/usr/bin/ffmpeg -i temp.mkv ");
    strcat (ff_make_movie_com, "-c:v libvpx -qmin 0 -qmax 50 -crf 5 -b:v 749k -y anim.webm'");
    strcpy (message, "Creating anim.webm from temp.mkv");
    //spawn it
    GPid ff_make_movie_com_pid = 0;
    SC_spawn (widget, ff_make_movie_com, &ff_make_movie_com_pid, message, "make_mp4_from_temp"); 
  } else
    SC_spawn (widget, NULL, NULL, "", "make_mp4_from_temp"); 
}

static void make_mp4_from_temp (GtkWidget *widget)
{
  gboolean *p_mp4 = P("p_mp4");
  if (*p_mp4) {
    char ff_make_movie_com[200], message[35];
    //construct ff_make_movie_com: ffmpeg -i temp.mkv -c:v libvpx -qmin 0 -qmax 50 -crf 5 -b:v 749k anim.webm 
    //or:                          ffmpeg -i temp.mkv -pix_fmt yuv420p -vf "scale=trunc(iw/2)*2:trunc(ih/2)*2" anim.mp4 
    strcpy (ff_make_movie_com, "/bin/sh -c '/usr/bin/ffmpeg -i temp.mkv ");
    strcat (ff_make_movie_com, "-pix_fmt yuv420p -vf \"scale=trunc(iw/2)*2:trunc(ih/2)*2\" -y anim.mp4'");
    strcpy (message, "Creating anim.mp4 from temp.mkv'");
    //spawn it
    GPid ff_make_movie_com_pid = 0;
    SC_spawn (widget, ff_make_movie_com, &ff_make_movie_com_pid, message, "make_webm_from_pngs"); 
  } else
    SC_spawn (widget, NULL, NULL, "", "make_webm_from_pngs"); 
}

static void make_webm_from_pngs (GtkWidget *widget)
{
  gboolean *p_webm = P("p_webm");
  if (p_webm) {
    char ff_make_movie_com[200], char_fps[4], message[35];
    //construct ff_make_movie_com: ffmpeg -r fps -i ew-[0-9][0-9][0-9].png -c:v libvpx -qmin 0 -qmax 50 -crf 5 -b:v 749k -y anim.webm
    //or:                          ffmpeg -r fps -i ew-[0-9][0-9][0-9].png -pix_fmt yuv420p -vf "scale=trunc(iw/2)*2:trunc(ih/2)*2" -y anim.mp4 
    strcpy (ff_make_movie_com, "/bin/sh -c '/usr/bin/ffmpeg -r ");
    int *p_fps = P("p_fps");
    snprintf (char_fps, 4, "%d", *p_fps);
    strcat (ff_make_movie_com, char_fps);
    strcat (ff_make_movie_com, " -i ew-[0-9][0-9][0-9].png -c:v libvpx -qmin 0 -qmax 50 -crf 5 -b:v 749k -y anim.webm'");
    strcpy (message, "Creating anim.webm from ew-???.png");
    //spawn it
    GPid ff_make_movie_com_pid = 0;
    SC_spawn (widget, ff_make_movie_com, &ff_make_movie_com_pid, message, "make_mp4_from_pngs"); 
  } else
    SC_spawn (widget, NULL, NULL, "", "make_mp4_from_pngs"); 
}

static void make_mp4_from_pngs (GtkWidget *widget)
{
  gboolean *p_mp4 = P("p_mp4");
  if (*p_mp4) {
    char ff_make_movie_com[200], char_fps[4], message[35];
    //construct ff_make_movie_com: ffmpeg -r fps -i ew-[0-9][0-9][0-9].png -c:v libvpx -qmin 0 -qmax 50 -crf 5 -b:v 749k -y anim.webm
    //or:                          ffmpeg -r fps -i ew-[0-9][0-9][0-9].png -pix_fmt yuv420p -vf "scale=trunc(iw/2)*2:trunc(ih/2)*2" -y anim.mp4 
    strcpy (ff_make_movie_com, "/bin/sh -c '/usr/bin/ffmpeg -r ");
    int *p_fps = P("p_fps");
    snprintf (char_fps, 4, "%d", *p_fps);
    strcat (ff_make_movie_com, char_fps);
    strcat (ff_make_movie_com, " -i ew-[0-9][0-9][0-9].png -pix_fmt yuv420p -vf \"scale=trunc(iw/2)*2:trunc(ih/2)*2\" -y anim.mp4'");
    strcpy (message, "Creating anim.mp4 from ew-???.png");
    //spawn it
    GPid ff_make_movie_com_pid = 0;
    SC_spawn (widget, ff_make_movie_com, &ff_make_movie_com_pid, message, "finish"); 
  } else
    SC_spawn (widget, NULL, NULL, "", "finish"); 
}

static void child_watch_cb (GPid pid, int status, gpointer data) //called when spawned process ends
{
  g_spawn_close_pid (pid); //supposed to use this even though pid is just an int on linux and closing it does nothing

  //call nextfunction
  GtkWidget *dialog = data;
  GtkWidget *widget = GTK_WIDGET(gtk_window_get_transient_for (GTK_WINDOW(dialog)));
  char silentcast_dir[PATH_MAX];
  get_silentcast_dir (widget, silentcast_dir);
  char *nextfunc = P("nextfunc");
 SC_show_error (widget, nextfunc); 

  if (!strcmp (nextfunc, "show_edit_pngs")) { 
    if (!animgif_exists (widget, silentcast_dir)) show_edit_pngs (widget);
    else make_webm_from_temp (widget);
  } else if (!strcmp (nextfunc, "make_webm_from_temp")) make_webm_from_temp (widget);
  else if (!strcmp (nextfunc, "make_mp4_from_temp")) make_mp4_from_temp (widget);
  else if (!strcmp (nextfunc, "make_webm_from_pngs")) make_webm_from_pngs (widget);
  else if (!strcmp (nextfunc, "make_mp4_from_pngs")) make_mp4_from_pngs (widget);
  else if (!strcmp (nextfunc, "finish")) {
    gboolean *p_pngs = P("p_pngs");
    if (!*p_pngs) delete_pngs (widget, silentcast_dir, 0); //0 means delete them all
    //try to open silentcast_dir in the default file manager to show the final outputs
    char *glib_encoded_silentcast_dir = SC_get_glib_filename (widget, silentcast_dir);
    if (glib_encoded_silentcast_dir) {
      //trying to open the default filemanager and don't care about errors
      char *silentcast_dir_uri = g_filename_to_uri (glib_encoded_silentcast_dir, NULL, NULL);
      g_free (glib_encoded_silentcast_dir);
      if (silentcast_dir_uri) {
        g_app_info_launch_default_for_uri (silentcast_dir_uri, NULL, NULL);
        g_free (silentcast_dir_uri);
      }
    } else SC_show_err_message (widget, "Silentcast Internal Error. Function not found: ", nextfunc);
    //the surface widget was hidden just before calling SC_generate_outputs
    //should probably destroy the widget now, but that will close the filebrowser just opened
    //so, showing it again
    gtk_widget_show_all (widget); 
  }

  gtk_widget_destroy (dialog); //close the spawn dialog when the spawned process ends
}

static void cancel_cb (GtkWidget *cancel_button, gpointer data)
{
  GtkWidget *widget = GTK_WIDGET(gtk_window_get_transient_for (GTK_WINDOW(gtk_widget_get_toplevel(cancel_button))));
  GPid *p_pid = data;
  if (kill (*p_pid, SIGTERM) == -1) SC_show_error (widget, "Error trying to kill command");
}

static void show_spawn_dialog (GtkWidget *widget, char silentcast_dir[PATH_MAX], GPid pid, char *message) 
{
  if (strcmp (message, "")) { //don't show dialog if there's no message to show (strcmp will be 0 when they match)
    GtkWidget *dialog_widget = gtk_application_window_new (gtk_window_get_application (GTK_WINDOW(widget)));
    gtk_window_set_transient_for (GTK_WINDOW(dialog_widget), GTK_WINDOW(widget));
    gtk_window_set_title (GTK_WINDOW(dialog_widget), silentcast_dir);
    gtk_window_set_modal (GTK_WINDOW(dialog_widget), TRUE);
    g_child_watch_add (pid, (GChildWatchFunc) child_watch_cb, dialog_widget);

    GtkWidget *message_label = gtk_label_new (message);
    GtkWidget *cancel_button = gtk_button_new_with_label ("Cancel");
    g_signal_connect (cancel_button, "clicked", 
        G_CALLBACK(cancel_cb), &pid);

    GtkWidget *dialog = gtk_grid_new ();
    gtk_container_add (GTK_CONTAINER(dialog_widget), dialog);
#define DIALOG_ATCH(A,C,R,W) gtk_grid_attach (GTK_GRID(dialog), A, C, R, W, 1) 
    DIALOG_ATCH(message_label, 0, 0, 4);
                                        DIALOG_ATCH(cancel_button, 4, 1, 1);

    gtk_widget_show_all (dialog_widget);
  }
}

void SC_spawn (GtkWidget *widget, char *commandstring, GPid *p_pid, char *message, char *nextfunc)
{
  int argc = 0;
  char **argv = NULL;
  GError *err = NULL;
  char silentcast_dir[PATH_MAX];
  get_silentcast_dir (widget, silentcast_dir);

  if (commandstring && strcmp (commandstring,"")) { //don't spawn the command if there is none (strcmp is false when match)
    GSpawnFlags flag = G_SPAWN_DEFAULT;
    //if there's a message, a dialog will be shown until the spawned command ends and so need child not to be reaped automatically
    if (strcmp (message, "")) flag = G_SPAWN_DO_NOT_REAP_CHILD; //strcmp is false when they match (i.e. 0)
    char *glib_encoded_working_dir = SC_get_glib_filename (widget, silentcast_dir);
    if (glib_encoded_working_dir) {
      if (!g_shell_parse_argv (commandstring, &argc, &argv, &err)) {
        SC_show_err_message (widget, "Error trying to parse the command: ", err->message);
        g_error_free (err);
      } else if (!g_spawn_async (glib_encoded_working_dir, argv, NULL, flag, NULL, NULL, p_pid, &err)) {
        SC_show_err_message (widget, "Error trying to spawn the command: ", err->message);
        g_error_free (err);
      } else {
        g_strfreev (argv);
        strcpy (P("nextfunc"), nextfunc);
        show_spawn_dialog (widget, silentcast_dir, *p_pid, message);
      }
      g_free (glib_encoded_working_dir);
    }
  } else strcpy (P("nextfunc"), nextfunc);
}

//Starts a chain of spawned commands. Each call to SC_spawn specifies a command to spawn and a nextfunc to call next.
//The nextfunc gets called in a watch child callback function when the spawned command ends.
void SC_generate_outputs (GtkWidget *widget)
{
  char silentcast_dir[PATH_MAX];
  get_silentcast_dir (widget, silentcast_dir);
  gboolean *p_pngs = P("p_pngs"), *p_gif = P("p_gif"), *p_anims_from_temp = P("p_anims_from_temp");
  
  if (*p_pngs || *p_gif || !*p_anims_from_temp) {
    delete_pngs (widget, silentcast_dir, 0); //before generating new pngs, delete any existing ones (0 means keep none)
    if (temp_exists (widget, silentcast_dir)) {
      char ff_gen_pngs[200];
      int *p_fps = P("p_fps");
      char char_fps[5]; snprintf (char_fps, 5, "%d", *p_fps);
      //construct the command to generate pngs: ffmpeg -i -temp.mkv -r fps ew-%03d.png
      strcpy (ff_gen_pngs, "/bin/sh -c '/usr/bin/ffmpeg -i temp.mkv -r ");
      strcat (ff_gen_pngs, char_fps);
      strcat (ff_gen_pngs, " ew-%03d.png'");
      //spawn it
      GPid ff_gen_pngs_pid = 0;

      //spawn command to generate the pngs then call show_edit_pngs
      SC_spawn (widget, ff_gen_pngs, &ff_gen_pngs_pid, "Generating pngs from anim.temp.", "show_edit_pngs"); 
    }
  }
}
