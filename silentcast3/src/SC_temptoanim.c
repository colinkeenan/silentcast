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

/* globerr for glob() which requires an error handling function be passed to it
 * and glob() is used in check_for_filepattern
 */
static int globerr(const char *path, int eerrno)
{
  //can't use show_err_message because can't get widget into this function
	fprintf (stderr, "Glob Error, %s: %s", path, strerror(eerrno)); 
	return 0;	/* let glob() keep going */
}

static gboolean get_pngs_glob (GtkWidget *widget, char silentcast_dir[PATH_MAX], glob_t *glob_pngs, gboolean show_errors) 
//should use globfree (glob_pngs) when done with it
{
  char pattern[PATH_MAX];
  strcat (pattern, silentcast_dir);
  strcat (pattern, "ew-???.png"); 
  //glob returns 0 if successful so tests as true when there's an error
  if (glob (pattern, 0, globerr, glob_pngs))  { //not using any flags, that's what the 0 is
    if (show_errors) SC_show_error (widget, "Error trying to expand ew-???.png");
    return FALSE;
  }
  return TRUE;
}

static void delete_pngs (GtkWidget *widget, char silentcast_dir[PATH_MAX], int group)
{
  if (group != 1) { //don't need to do anything if group is 1 because that means keep everything
    glob_t *glob_pngs = NULL;
    get_pngs_glob (widget, silentcast_dir, glob_pngs, FALSE); //FALSE means don't show errors
    for (int i=0; i<glob_pngs->gl_pathc; i++) { //nothing will happen if there's no pngs since gl_pathc will be 0
      char *glib_encoded_filename = SC_get_glib_filename (widget, glob_pngs->gl_pathv[i]);
      if (glib_encoded_filename) {
        //Keep 1 out of group, so 1 = keep all, 2 = keep every other, 3 = keep 1 out of 3 etc.
        //Also, 0 = don't keep any. If group isn't 0, always keep the 2nd one (i = 1) and
        //every +group after that (keep i = 1, i = 1+group, i = 1+2*group etc. In other words,
        //keep when i == 1 or when i % group == 1. Delete when i != 1 && i%group != 1)
        if (group == 0 || (i % group != 1 && i != 1)) g_remove (glib_encoded_filename); 
        g_free (glib_encoded_filename);
      } 
    }
    globfree (glob_pngs);
  }
}

static gboolean animgif_exists (GtkWidget *widget, char silentcast_dir[PATH_MAX], gboolean anims_from_temp)
{
  char filename[PATH_MAX];
  strcpy (filename, silentcast_dir);
  strcat (filename, "/anim.gif");
  char warning[] = 
    "Too many images for the available memory. Try closing other applications, creating a swap file, or removing unecessary images.";

  if (is_file (widget, filename, warning) ) {
    //if anim.gif was made and pngs aren't needed to make a movie, delete pngs
    if (anims_from_temp) delete_pngs (widget, silentcast_dir, 0); //0 means delete them all
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

static void generate_pngs (GtkWidget *widget, char silentcast_dir[PATH_MAX], int fps)
{
  delete_pngs (widget, silentcast_dir, 0); //before generating new pngs, delete any existing ones (0 means keep none)
  if (temp_exists (widget, silentcast_dir)) {
    char *glib_encoded_silentcast_dir = SC_get_glib_filename (widget, silentcast_dir);
    if (glib_encoded_silentcast_dir) {
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

static void show_make_anim_dialog (GtkWidget *widget, char silentcast_dir[PATH_MAX], int pid, char *message) 
{
  int ret = 0;
  GtkWidget *dialog = 
    gtk_message_dialog_new (GTK_WINDOW(widget), 
        GTK_DIALOG_DESTROY_WITH_PARENT, 
        GTK_MESSAGE_INFO, GTK_BUTTONS_CANCEL, "%s", message);
  gtk_window_set_title (GTK_WINDOW(dialog), silentcast_dir);
  ret = gtk_dialog_run (GTK_DIALOG (dialog));
  //if cancel was clicked, send a kill signal to the command
  if (ret == GTK_RESPONSE_CANCEL) SC_kill_pid (widget, pid);
  gtk_widget_destroy (dialog);
}

struct SC_anim_data {
  char *silentcast_dir;
  int *p_group;
  int *p_total_group;
  int *p_fps;
};

//executed when done button is clicked from edit_pngs
static gboolean make_anim_gif_cb (GtkWidget *done, gpointer data) {
  struct SC_anim_data *p_SC_anim_data = data; 
  char *silentcast_dir = p_SC_anim_data->silentcast_dir;
  int *p_group = p_SC_anim_data->p_group, group = *p_group;
  int *p_total_group = p_SC_anim_data->p_total_group;
  int *p_fps = p_SC_anim_data->p_fps, fps = *p_fps;
  char convert_com[100], delay[5];
  GtkWidget *widget = GTK_WIDGET(gtk_window_get_transient_for (GTK_WINDOW(gtk_widget_get_toplevel(done))));

  delete_pngs (widget, silentcast_dir, group); //delete all but 1 out of every group number of pngs (group set in edit_pngs)

  char *glib_encoded_silentcast_dir = SC_get_glib_filename (widget, silentcast_dir);
  if (glib_encoded_silentcast_dir) {
    //construct convert_com: convert -adjoin -delay (total_group * fps) -layers optimize ew-[0-9][0-9][0-9].png anim.gif
    *p_total_group = *p_total_group + group;
    strcpy (convert_com, "convert -adjoin -delay ");
    snprintf (delay, 5, "%d", *p_total_group * fps);
    strcat (convert_com, delay);
    strcat (convert_com, " -layers optimize ew-[0-9][0-9][0-9].png anim.gif");
    //spawn it
    int convert_com_pid = 0;
    SC_spawn (widget, glib_encoded_silentcast_dir, convert_com, &convert_com_pid); 
    show_make_anim_dialog (widget, silentcast_dir, convert_com_pid, "making anim.gif from pngs");
    g_free (glib_encoded_silentcast_dir);
  }
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
    .p_fps = p_fps
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
  GtkWidget *edit_pngs = gtk_grid_new ();
  gtk_container_add (GTK_CONTAINER(edit_pngs_widget), edit_pngs);
#define EDPNGS_ATCH(A,C,R,W) gtk_grid_attach (GTK_GRID(edit_pngs), A, C, R, W, 1) 
  EDPNGS_ATCH(group_spinbutt_label, 0, 0, 1); EDPNGS_ATCH(group_spnbutt, 1, 0, 1);
  EDPNGS_ATCH(gtk_label_new ("You can also manipulate the images manually in the file browser or other application."), 0, 1, 4);
  EDPNGS_ATCH(gtk_label_new ("Clicking done will prune the pngs (if indicated) and try to make anim.gif from what remains."), 0, 2, 4);
                                                                                                      EDPNGS_ATCH(done, 4, 3, 1);
  gtk_widget_show_all(edit_pngs_widget);
}

static void make_movie_from_pngs (GtkWidget *widget, char silentcast_dir[PATH_MAX], int fps, gboolean webm)
{
  char ff_make_movie_com[100], char_fps[4], message[35];
  char *glib_encoded_silentcast_dir = SC_get_glib_filename (widget, silentcast_dir);
  if (glib_encoded_silentcast_dir) {
    //construct ff_make_movie_com: ffmpeg -r fps -i ew-[0-9][0-9][0-9].png -c:v libvpx -qmin 0 -qmax 50 -crf 5 -b:v 749k anim.webm
    //or:                          ffmpeg -r fps -i ew-[0-9][0-9][0-9].png -pix_fmt yuv420p -vf "scale=trunc(iw/2)*2:trunc(ih/2)*2" anim.mp4 
    strcpy (ff_make_movie_com, "ffmpeg -r ");
    snprintf (char_fps, 4, "%d", fps);
    strcat (ff_make_movie_com, char_fps);
    if (webm) {
      strcat (ff_make_movie_com, "ew-[0-9][0-9][0-9].png -c:v libvpx -qmin 0 -qmax 50 -crf 5 -b:v 749k anim.webm");
      strcpy (message, "Creating anim.webm from ew-???.png");
    } else {
      strcat (ff_make_movie_com, "ew-[0-9][0-9][0-9].png -pix_fmt yuv420p -vf \"scale=trunc(iw/2)*2:trunc(ih/2)*2\" anim.mp4");
      strcpy (message, "Creating anim.mp4 from ew-???.png");
    }
    //spawn it
    int ff_make_movie_com_pid = 0;
    SC_spawn (widget, glib_encoded_silentcast_dir, ff_make_movie_com, &ff_make_movie_com_pid); 
    show_make_anim_dialog (widget, silentcast_dir, ff_make_movie_com_pid, message);
    g_free (glib_encoded_silentcast_dir);
  }
}

static void make_movie_from_temp (GtkWidget *widget, char silentcast_dir[PATH_MAX], gboolean webm)
{
  char ff_make_movie_com[100], message[35];
  char *glib_encoded_silentcast_dir = SC_get_glib_filename (widget, silentcast_dir);
  if (glib_encoded_silentcast_dir) {
    //construct ff_make_movie_com: ffmpeg -i temp.mkv -c:v libvpx -qmin 0 -qmax 50 -crf 5 -b:v 749k anim.webm 
    //or:                          ffmpeg -i temp.mkv -pix_fmt yuv420p -vf "scale=trunc(iw/2)*2:trunc(ih/2)*2" anim.mp4 
    strcpy (ff_make_movie_com, "ffmpeg -i temp.mkv ");
    if (webm) {
      strcat (ff_make_movie_com, "-c:v libvpx -qmin 0 -qmax 50 -crf 5 -b:v 749k anim.webm");
      strcpy (message, "Creating anim.webm from temp.mkv");
    } else {
      strcat (ff_make_movie_com, "-pix_fmt yuv420p -vf \"scale=trunc(iw/2)*2:trunc(ih/2)*2\" anim.mp4");
      strcpy (message, "Creating anim.mp4 from temp.mkv");
    }
    //spawn it
    int ff_make_movie_com_pid = 0;
    SC_spawn (widget, glib_encoded_silentcast_dir, ff_make_movie_com, &ff_make_movie_com_pid); 
    show_make_anim_dialog (widget, silentcast_dir, ff_make_movie_com_pid, message);
    g_free (glib_encoded_silentcast_dir);
  }
}

void SC_generate_outputs (GtkWidget *widget, char silentcast_dir[PATH_MAX], int *p_fps,
    gboolean anims_from_temp, gboolean gif, gboolean webm, gboolean mp4)
{
  if (gif || !anims_from_temp) generate_pngs (widget, silentcast_dir, *p_fps); //existing pngs are deleted in generate_pngs
  if (gif) 
    while (!animgif_exists (widget, silentcast_dir, anims_from_temp)) show_edit_pngs (widget, silentcast_dir, p_fps);
  if (anims_from_temp) {
    if (webm) make_movie_from_temp (widget, silentcast_dir, TRUE); //TRUE means make webm
    if (mp4) make_movie_from_temp (widget, silentcast_dir, FALSE); //FALSE means make mp4
  } else {
    if (webm) make_movie_from_pngs (widget, silentcast_dir, *p_fps, TRUE);
    if (mp4) make_movie_from_pngs (widget, silentcast_dir, *p_fps, FALSE);
  }
}
