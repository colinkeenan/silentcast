/*
 *  Filename: SC_conf_widgets.h 
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
 *  Description: Do all the includes needed, expose functions for 
 *  getting and setting configuration, and define macros. 
 *
 */
#ifndef SC_CONF_WIDGETS_H
#define SC_CONF_WIDGETS_H

#include <gtk/gtkx.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <errno.h>

#define PF1(A) g_object_get_data (G_OBJECT(f1_widget), A)
#define P(A) g_object_get_data (G_OBJECT(widget), A)
#define PRESET_N 16
#define COL_SPACING 32
#define ROW_SPACING 8
#define BGN_SCTN "<span font_family=\"serif\"><big>"
#define END_SCTN "</big></span>"

void get_presets (GtkWidget *widget, double presets[PRESET_N], double previous[2]);

void get_conf (GtkWidget *widget, GtkEntryBuffer *entry_buffer, char area[2], unsigned int *p_fps, gboolean *p_anims_from_temp, 
    gboolean *p_gif, gboolean *p_pngs, gboolean *p_webm, gboolean *p_mp4); 

double get_w(double a);
double get_h(double a);

void show_f1_widget (GtkApplication *app, GtkWidget *widget);

#endif //SC_CONF_WIDGETS_H
