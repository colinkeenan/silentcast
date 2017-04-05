/*
 *  Filename: SC_temptoanim.h 
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
 *  Description: definitions of the global funtions in SC_temptoanim.c 
 *
 */

#ifndef SC_TEMPTOANIM_H
#define SC_TEMPTOANIM_H

#include <gtk/gtkx.h>
#include <string.h>
#include <errno.h>
#include <glob.h>
#include <glib.h>
#include <glib/gstdio.h>

void SC_show_error (GtkWidget *widget, char *err_message);
void SC_show_err_message (GtkWidget *widget, char *message, char *errmessage);
gboolean SC_get_glib_filename (GtkWidget *widget, char filename[PATH_MAX], char *glib_filename);
void SC_spawn (GtkWidget *widget, char *glib_encoded_working_dir, char *commandstring, int *p_pid);

#endif //SC_TEMPTOANIM_H
