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

static gboolean animgif_exists (GtkWidget *widget, char silentcast_dir[PATH_MAX], gboolean anims_from_temp, gboolean pngs)
{
  char filename[PATH_MAX];
  strcpy (filename, silentcast_dir);
  strcat (filename, "/anim.gif");
  char warning_if_no_file[] = 
    "Too many images for the available memory. Try closing other applications, creating a swap file, or removing unecessary images.";

  return is_file (widget, filename, warning_if_no_file);
}

static void show_spawn_dialog (GtkWidget *widget, char silentcast_dir[PATH_MAX], GPid pid, char *message) 
{
  //need to rewrite this without using gtk_dialog so I can do the waitpid loop at the same time
  //and update what's shown in the "dialog" with each loop
  //and when done, gtk_dialog_response (dialog, GTK_RESPONSE_CLOSE)
  if (strcmp (message, "")) { //only show a dialog if there's a message to show (strcmp will be 0 when they match)
    int ret = 0;
    GtkWidget *dialog = 
      gtk_message_dialog_new (GTK_WINDOW(widget), 
          GTK_DIALOG_DESTROY_WITH_PARENT, 
          GTK_MESSAGE_INFO, GTK_BUTTONS_CANCEL, "%s", message);
    gtk_window_set_title (GTK_WINDOW(dialog), silentcast_dir);
    ret = gtk_dialog_run (GTK_DIALOG (dialog));
    //if cancel was clicked, send a kill signal to the command
    if (ret == GTK_RESPONSE_CANCEL) {
      if (kill (pid, SIGTERM) == -1) SC_show_error (widget, "Error trying to kill command");
      else g_spawn_close_pid (pid); //doesn't do anything on linux but supposed to use it anyway
    }
    gtk_widget_destroy (dialog);
  }
}

void SC_spawn (GtkWidget *widget, char *working_dir, char *commandstring, GPid *p_pid, char *message)
{
  int argc = 0;
  char **argv = NULL;
  GError *err = NULL;

  char *glib_encoded_working_dir = SC_get_glib_filename (widget, working_dir);
  if (glib_encoded_working_dir) {
    if (!g_shell_parse_argv (commandstring, &argc, &argv, &err)) {
      SC_show_err_message (widget, "Error trying to parse the command: ", err->message);
      g_error_free (err);
    } else if (!g_spawn_async (glib_encoded_working_dir, argv, NULL, G_SPAWN_DEFAULT, NULL, NULL, p_pid, &err)) {
      SC_show_err_message (widget, "Error trying to spawn the command: ", err->message);
      g_error_free (err);
    } else show_spawn_dialog (widget, working_dir, *p_pid, message);
    g_free (glib_encoded_working_dir);
  }
}

static void generate_pngs (GtkWidget *widget, char silentcast_dir[PATH_MAX], int fps)
{
  delete_pngs (widget, silentcast_dir, 0); //before generating new pngs, delete any existing ones (0 means keep none)
  if (temp_exists (widget, silentcast_dir)) {
    char ff_gen_pngs[200];
    char char_fps[5]; snprintf (char_fps, 5, "%d", fps);
    //construct the command to generate pngs: ffmpeg -i -temp.mkv -r fps ew-%03d.png
    strcpy (ff_gen_pngs, "/usr/bin/ffmpeg -i temp.mkv -r ");
    strcat (ff_gen_pngs, char_fps);
    strcat (ff_gen_pngs, " ew-%03d.png");
    //spawn it
    GPid ff_gen_pngs_pid = 0;
    SC_spawn (widget, silentcast_dir, ff_gen_pngs, &ff_gen_pngs_pid, "Generating pngs from anim.temp."); 
  }
}

static gboolean on_value_changed_group (GtkSpinButton *group_spnbutt, gpointer data)
{
  int *p_group = data;
  *p_group  = gtk_spin_button_get_value_as_int (group_spnbutt);
  return TRUE;
}

struct SC_anim_data {
  char *silentcast_dir;
  int *p_group;
  int *p_total_group;
  int *p_fps;
  GMainLoop *loop;
};

//executed when done button is clicked from edit_pngs
static gboolean make_anim_gif_cb (GtkWidget *done, gpointer data) {
  struct SC_anim_data *p_SC_anim_data = data; 
  char *silentcast_dir = p_SC_anim_data->silentcast_dir;
  int *p_group = p_SC_anim_data->p_group, group = *p_group;
  int *p_total_group = p_SC_anim_data->p_total_group;
  int *p_fps = p_SC_anim_data->p_fps, fps = *p_fps;
  GMainLoop *loop = p_SC_anim_data->loop;
  if (g_main_loop_is_running (loop)) g_main_loop_quit (loop); 

  char convert_com[128], delay[5];
  GtkWidget *widget = GTK_WIDGET(gtk_window_get_transient_for (GTK_WINDOW(gtk_widget_get_toplevel(done))));

  delete_pngs (widget, silentcast_dir, group); //delete all but 1 out of every group number of pngs (group set in edit_pngs)

  //construct convert_com: convert -adjoin -delay (total_group * fps) -layers optimize ew-[0-9][0-9][0-9].png anim.gif
  *p_total_group = *p_total_group + group;
  strcpy (convert_com, "/usr/bin/convert -adjoin -delay ");
  snprintf (delay, 5, "%d", *p_total_group * fps);
  strcat (convert_com, delay);
  strcat (convert_com, " -layers optimize ew-[0-9][0-9][0-9].png anim.gif");
  //spawn it
  GPid convert_com_pid = 0;
  SC_spawn (widget, silentcast_dir, convert_com, &convert_com_pid, "Making anim.gif from pngs."); 
  return TRUE;
}

static void show_edit_pngs (GtkWidget *widget, char silentcast_dir[PATH_MAX], int *p_fps)
{
  static int group = 1; //in delete_pngs, keep 1 out of group, so 1 = keep all, 2 = keep every other, 3 = keep 1 out of 3
  int *p_group = &group;
  static int total_group = 0; //user may edit pngs many times, choosing many groups in a row. need total for calculating delay in make_anim_gif
  int *p_total_group = &total_group;
  struct SC_anim_data SC_anim_data = {
    .silentcast_dir = silentcast_dir,
    .p_group = p_group,
    .p_total_group = p_total_group,
    .p_fps = p_fps,
    .loop = g_main_loop_new (NULL, FALSE)
  };
  struct SC_anim_data *p_SC_anim_data = &SC_anim_data;
  GtkWidget *group_spnbutt = NULL;
  group_spnbutt = gtk_spin_button_new_with_range (1, 8, 1); 
  g_signal_connect (group_spnbutt, "value-changed", 
     G_CALLBACK(on_value_changed_group), p_group);
  GtkWidget *group_spinbutt_label = gtk_label_new ("Prune pngs, keeping 1 out of every"); 
  gtk_widget_set_halign (group_spinbutt_label, GTK_ALIGN_END);
  GtkWidget *done= gtk_button_new_with_label ("Done");
  g_signal_connect (done, "clicked", 
     G_CALLBACK(make_anim_gif_cb), p_SC_anim_data);

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

  //trying to block the application until the user clicks done. Got this method from source code for gtk_dialog_run, but it's deprecated
  gdk_threads_leave ();
  g_main_loop_run (SC_anim_data.loop);
  gdk_threads_enter ();
  g_main_loop_unref (SC_anim_data.loop);
  SC_anim_data.loop = NULL;
  gtk_widget_destroy (edit_pngs_widget);
}

static void make_movie_from_pngs (GtkWidget *widget, char silentcast_dir[PATH_MAX], int fps, gboolean webm)
{
  char ff_make_movie_com[128], char_fps[4], message[35];
  //construct ff_make_movie_com: ffmpeg -r fps -i ew-[0-9][0-9][0-9].png -c:v libvpx -qmin 0 -qmax 50 -crf 5 -b:v 749k -y anim.webm
  //or:                          ffmpeg -r fps -i ew-[0-9][0-9][0-9].png -pix_fmt yuv420p -vf "scale=trunc(iw/2)*2:trunc(ih/2)*2" -y anim.mp4 
  strcpy (ff_make_movie_com, "/usr/bin/ffmpeg -r ");
  snprintf (char_fps, 4, "%d", fps);
  strcat (ff_make_movie_com, char_fps);
  if (webm) {
    strcat (ff_make_movie_com, "ew-[0-9][0-9][0-9].png -c:v libvpx -qmin 0 -qmax 50 -crf 5 -b:v 749k -y anim.webm");
    strcpy (message, "Creating anim.webm from ew-???.png");
  } else {
    strcat (ff_make_movie_com, "ew-[0-9][0-9][0-9].png -pix_fmt yuv420p -vf \"scale=trunc(iw/2)*2:trunc(ih/2)*2\" -y anim.mp4");
    strcpy (message, "Creating anim.mp4 from ew-???.png");
  }
  //spawn it
  GPid ff_make_movie_com_pid = 0;
  SC_spawn (widget, silentcast_dir, ff_make_movie_com, &ff_make_movie_com_pid, message); 
}

static void make_movie_from_temp (GtkWidget *widget, char silentcast_dir[PATH_MAX], gboolean webm)
{
  char ff_make_movie_com[128], message[35];
  //construct ff_make_movie_com: ffmpeg -i temp.mkv -c:v libvpx -qmin 0 -qmax 50 -crf 5 -b:v 749k anim.webm 
  //or:                          ffmpeg -i temp.mkv -pix_fmt yuv420p -vf "scale=trunc(iw/2)*2:trunc(ih/2)*2" anim.mp4 
  strcpy (ff_make_movie_com, "/usr/bin/ffmpeg -i temp.mkv ");
  if (webm) {
    strcat (ff_make_movie_com, "-c:v libvpx -qmin 0 -qmax 50 -crf 5 -b:v 749k -y anim.webm");
    strcpy (message, "Creating anim.webm from temp.mkv");
  } else {
    strcat (ff_make_movie_com, "-pix_fmt yuv420p -vf \"scale=trunc(iw/2)*2:trunc(ih/2)*2\" -y anim.mp4");
    strcpy (message, "Creating anim.mp4 from temp.mkv");
  }
  //spawn it
  GPid ff_make_movie_com_pid = 0;
  SC_spawn (widget, silentcast_dir, ff_make_movie_com, &ff_make_movie_com_pid, message); 
}

void SC_generate_outputs (GtkWidget *widget, char silentcast_dir[PATH_MAX], int *p_fps,
    gboolean anims_from_temp, gboolean gif, gboolean pngs, gboolean webm, gboolean mp4)
{
  if (pngs || gif || !anims_from_temp) generate_pngs (widget, silentcast_dir, *p_fps); //previous pngs (if any) are deleted in generate_pngs
  if (gif) {
    do show_edit_pngs (widget, silentcast_dir, p_fps); 
    while (!animgif_exists (widget, silentcast_dir, anims_from_temp, pngs));
  }
  if (anims_from_temp) {
    if (webm) make_movie_from_temp (widget, silentcast_dir, TRUE); //TRUE means make webm
    if (mp4) make_movie_from_temp (widget, silentcast_dir, FALSE); //FALSE means make mp4
  } else {
    if (webm) make_movie_from_pngs (widget, silentcast_dir, *p_fps, TRUE);
    if (mp4) make_movie_from_pngs (widget, silentcast_dir, *p_fps, FALSE);
  }
  if (!pngs) delete_pngs (widget, silentcast_dir, 0); //0 means delete them all
  gtk_widget_show (widget); //the surface widget was hidden just before calling SC_generate_outputs

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
  }
}
