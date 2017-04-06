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


/*pointers defined in main.c: P("working_dir") P("area") P("p_fps") P("p_anims_from_temp") P("p_gif") P("p_pngs") P("p_webm") P("p_mp4")
 *
 * Translating variables from old bash version of temptoanim:
 *
 * anims_from: was defined but never actually used in bash temptoanim
 *
 * otype: "png" if *p_pngs && !(*p_gif || *p_webm || *p_mp4), "gif" if *p_gif, "webm" if *p_webm, "mp4" if *p_mp4 
 *        (bash version was run multiple times to get all selected outputs)
 * rm_png: !(*p_pngs)
 * use_pngs: !(*p_anims_from_temp) && (*p_webm || *p_mp4) and was also for using exising pngs without generating them from temp.mkv
 * gen_pngs: *p_pngs || (!(*p_anims_from_temp) && (*p_webm || *p_mp4))
 * fps: *p_fps
 *
 * cut, total_cut, group, count: These were for reducing the number of pngs and adjusting the rate if user chose to do so due to memory limitation
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
  fprintf (stderr, "%s", err_message);
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

gboolean SC_get_glib_filename (GtkWidget *widget, char filename[PATH_MAX], char *glib_filename)
{
  GError *err = NULL;
  gsize bytes_written = 0;
  glib_filename = g_filename_from_utf8 (filename, -1, NULL, &bytes_written, &err);
  if (err) {
    SC_show_err_message (widget, "Error getting glib encoded filename: ", err->message);
    g_error_free (err);
    return FALSE;
  }
  return TRUE;
}

static gboolean is_file (GtkWidget *widget, char filename[PATH_MAX], char *errmessage)
{
  char *glib_encoded_filename = NULL;
  if (!SC_get_glib_filename (widget, filename, glib_encoded_filename)) return FALSE;
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

/* globerr for glob() which requires an error handling function be passed to it
 * and glob() is used in check_for_filepattern
 */
static int globerr(const char *path, int eerrno)
{
  //can't use show_err_message because can't get widget into this function
	fprintf (stderr, "Glob Error, %s: %s", path, strerror(eerrno)); 
	return 0;	/* let glob() keep going */
}

static gboolean get_pngs_glob (GtkWidget *widget, glob_t *p_pngs_glob) //should use globfree (p_pngs_glob) when done with it
{
  //glob returns 0 if successful so tests as true when there's an error
  if (glob ("ew-???.png", 0, globerr, p_pngs_glob))  { //not using any flags, that's what the 0 is
    SC_show_error (widget, "Error: ew-???.png not found");
    return FALSE;
  }
  return TRUE;
}

static void delete_pngs (GtkWidget *widget, glob_t *p_pngs_glob, int group)
{
  if (group == 1) { //don't need to do anything if group is 1 because that means keep everything
    for (int i=0; i<p_pngs_glob->gl_pathc; i++) { 
      char *glib_encoded_filename = NULL;
      if (SC_get_glib_filename (widget, p_pngs_glob->gl_pathv[i], glib_encoded_filename)) {
        //keep 1 out of group, so 1 = keep all, 2 = keep every other, 3 = keep 1 out of 3
        //also, 0 = don't keep any
        if (group == 0 || i % group != 1) g_remove (glib_encoded_filename); 
        g_free (glib_encoded_filename);
      } 
    }
    globfree (p_pngs_glob);
  }
}

static gboolean animgif_exists (GtkWidget *widget, char silentcast_dir[PATH_MAX], gboolean pngs, glob_t *p_pngs_glob)
{
  char filename[PATH_MAX];
  strcpy (filename, silentcast_dir);
  strcat (filename, "/anim.gif");
  if (is_file (widget, filename, 
        "Too many images for the available memory. Try closing other applications, creating a swap file, or removing unecessary images.") 
      && !pngs) {
    //if anim.gif was made and pngs aren't a desired output, delete them
    delete_pngs (widget, p_pngs_glob, 0); //0 means delete them all
    return TRUE;
  } else return FALSE;
}

void SC_spawn (GtkWidget *widget, char *glib_encoded_working_dir, char *commandstring, int *p_pid)
{
  int argc = 0;
  char **argv = NULL;
  GError *err = NULL;

  if (!g_shell_parse_argv (commandstring, &argc, &argv, &err)) {
    SC_show_err_message (widget, "Error trying to parse the command: ", err->message);
    g_error_free (err);
  } else if (!g_spawn_async (glib_encoded_working_dir, argv, NULL, G_SPAWN_DEFAULT, NULL, NULL, p_pid, &err)) {
    SC_show_err_message (widget, "Error trying to spawn the command: ", err->message);
    g_error_free (err);
  }
}

void SC_kill_pid (GtkWidget *widget, int pid) 
{
  GError *err = NULL;
  char kill_com[13], char_pid[8]; //pid will be at most 7 digits long and /0 to terminate the string makes 8
  snprintf (char_pid, 8, "%d", pid);
  strcpy (kill_com, "kill ");
  strcat (kill_com, char_pid);
  if (!g_spawn_command_line_sync (kill_com, NULL, NULL, NULL, &err)) {
    SC_show_err_message (widget, "Error trying to kill command: ", err->message);
    g_error_free (err);
  }
}

static void show_genpngs_dialog (GtkWidget *widget, char silentcast_dir[PATH_MAX], int ff_gen_pngs_pid) 
{
  int ret = 0;
  GtkWidget *dialog = 
    gtk_message_dialog_new (GTK_WINDOW(widget), 
        GTK_DIALOG_DESTROY_WITH_PARENT, 
        GTK_MESSAGE_INFO, GTK_BUTTONS_CANCEL, "Generating pngs from temp.mkv");
  gtk_window_set_title (GTK_WINDOW(dialog), silentcast_dir);
  ret = gtk_dialog_run (GTK_DIALOG (dialog));
  if (ret == GTK_RESPONSE_CANCEL) SC_kill_pid (widget, ff_gen_pngs_pid);
  gtk_widget_destroy (dialog);
}

static void generate_pngs (GtkWidget *widget, char silentcast_dir[PATH_MAX], glob_t *p_pngs_glob, int fps)
{
  delete_pngs (widget, p_pngs_glob, 0); //before generating new pngs, delete any existing ones (0 means keep none)
  if (temp_exists (widget, silentcast_dir)) {
    char *glib_encoded_silentcast_dir = NULL;
    if (SC_get_glib_filename (widget, silentcast_dir, glib_encoded_silentcast_dir)) {
      char ff_gen_pngs[200];
      char char_fps[5]; snprintf (char_fps, 5, "%d", fps);
      //construct the command to generate pngs: ffmpeg -i -temp.mkv -r fps ew-%03d.png
      strcpy (ff_gen_pngs, "/usr/bin/ffmpeg -i temp.mkv -r ");
      strcat (ff_gen_pngs, char_fps);
      strcat (ff_gen_pngs, " ew-%03d.png");
      //spawn it
      int ff_gen_pngs_pid = 0;
      SC_spawn (widget, glib_encoded_silentcast_dir, ff_gen_pngs, &ff_gen_pngs_pid); 
      show_genpngs_dialog (widget, silentcast_dir, ff_gen_pngs_pid);
      g_free (glib_encoded_silentcast_dir);
    }
  }
}

static gboolean on_value_changed_group (GtkSpinButton *group_spnbutt, gpointer data)
{
  int *p_group = data;
  *p_group  = gtk_spin_button_get_value_as_int (group_spnbutt);
  return TRUE;
}

static void show_make_anim_dialog (GtkWidget *widget, char silentcast_dir[PATH_MAX], int convert_com_pid) 
{
  int ret = 0;
  GtkWidget *dialog = 
    gtk_message_dialog_new (GTK_WINDOW(widget), 
        GTK_DIALOG_DESTROY_WITH_PARENT, 
        GTK_MESSAGE_INFO, GTK_BUTTONS_CANCEL, "Generating anim.gif from pngs");
  gtk_window_set_title (GTK_WINDOW(dialog), silentcast_dir);
  ret = gtk_dialog_run (GTK_DIALOG (dialog));
  if (ret == GTK_RESPONSE_CANCEL) SC_kill_pid (widget, convert_com_pid);
  gtk_widget_destroy (dialog);
}

struct data_struct {
  char *silentcast_dir;
  glob_t *p_pngs_glob;
  int *p_group;
  int *p_fps;
};

static gboolean make_anim_gif_cb (GtkWidget *done, gpointer data) {
  struct data_struct *p_data_struct = data; 
  char *silentcast_dir = p_data_struct->silentcast_dir;
  glob_t *p_pngs_glob = p_data_struct->p_pngs_glob;
  int *p_group = p_data_struct->p_group, group = *p_group;
  int *p_fps = p_data_struct->p_fps, fps = *p_fps;
  char convert_com[100], delay[5];
  GtkWidget *widget = GTK_WIDGET(gtk_window_get_transient_for (GTK_WINDOW(gtk_widget_get_toplevel(done))));

  delete_pngs (widget, p_pngs_glob, group); //delete all but 1 out of every group number of pngs

  char *glib_encoded_silentcast_dir = NULL;
  if (SC_get_glib_filename (widget, silentcast_dir, glib_encoded_silentcast_dir)) {
    //construct convert_com: convert -adjoin -delay (group * fps) -layers optimize ew-[0-9][0-9][0-9].png anim.gif
    strcpy (convert_com, "convert -adjoin -delay ");
    snprintf (delay, 5, "%d", group * fps);
    strcat (convert_com, delay);
    strcat (convert_com, " -layers optimize ew-[0-9][0-9][0-9].png anim.gif");
    //spawn it
    int convert_com_pid = 0;
    SC_spawn (widget, glib_encoded_silentcast_dir, convert_com, &convert_com_pid); 
    show_make_anim_dialog (widget, silentcast_dir, convert_com_pid);
    g_free (glib_encoded_silentcast_dir);
  }
  return TRUE;
}

static void show_edit_pngs (GtkWidget *widget, char silentcast_dir[PATH_MAX], glob_t *p_pngs_glob, int *p_fps)
{
  static int group = 1; //in delete_pngs, keep 1 out of group, so 1 = keep all, 2 = keep every other, 3 = keep 1 out of 3
  int *p_group = &group;
  struct data_struct data_struct = {
    .silentcast_dir = silentcast_dir,
    .p_pngs_glob = p_pngs_glob,
    .p_group = p_group,
    .p_fps = p_fps
  };
  struct data_struct *p_data_struct = &data_struct;
  GtkWidget *group_spnbutt = NULL;
  group_spnbutt = gtk_spin_button_new_with_range (1, 8, 1); 
  g_signal_connect (group_spnbutt, "value-changed", 
     G_CALLBACK(on_value_changed_group), p_group);
  GtkWidget *group_spinbutt_label = gtk_label_new ("Delete pngs, keeping 1 out of every"); 
  gtk_widget_set_halign (group_spinbutt_label, GTK_ALIGN_END);
  GtkWidget *done= gtk_button_new_with_label ("Done");
  g_signal_connect (done, "clicked", 
     G_CALLBACK(make_anim_gif_cb), p_data_struct);

  GtkWidget *edit_pngs_widget = gtk_application_window_new (gtk_window_get_application (GTK_WINDOW(widget)));
  gtk_window_set_transient_for (GTK_WINDOW(edit_pngs_widget), GTK_WINDOW(widget));
  gtk_window_set_title (GTK_WINDOW(edit_pngs_widget), "Silentcast Edit Pngs");
  GtkWidget *edit_pngs = gtk_grid_new ();
  gtk_container_add (GTK_CONTAINER(edit_pngs_widget), edit_pngs);
#define EDPNGS_ATCH(A,C,R,W) gtk_grid_attach (GTK_GRID(edit_pngs), A, C, R, W, 1) 
  EDPNGS_ATCH(group_spinbutt_label, 0, 0, 1); EDPNGS_ATCH(group_spnbutt, 1, 0, 1);
  EDPNGS_ATCH(gtk_label_new ("You can also manipulate the images manually in the file browser or other application."), 0, 1, 4);
  EDPNGS_ATCH(gtk_label_new ("Clicking done will try to make anim.gif from the remaining pngs."), 0, 2, 4);
                                                                                                      EDPNGS_ATCH(done, 4, 3, 1);
  gtk_widget_show_all(edit_pngs_widget);
}
