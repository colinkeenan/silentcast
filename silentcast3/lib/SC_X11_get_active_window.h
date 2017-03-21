/*
 *  Filename: SC_X11_get_active_window.h 
 *  App Name: Silentcast <https://github.com/colinkeenan/silentcast>
 *  Copyright © 2016, 2017 Colin N Keenan <colinnkeenan@gmail.com>
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
 *  Description: defines some custom X11 error messags and exposes 3 functions:
 *  SC_get_active_gdkwindow (...), SC_get_geomeotry_for (...), 
 *  and SC_get_active_windows_and_geometry (...)
 */

#include <gtk/gtkx.h>

#define SC_X11_ERROR0 "                                      \n"
#define SC_X11_ERROR1 "Failed to connect to X server.\n"
#define SC_X11_ERROR2 "x11 error trying to get focused window\n"
#define SC_X11_ERROR3 "X11 reports no focused window\n"
#define SC_X11_ERROR4 "X11 error trying to get top window\n"

#define D_ERR 1
#define FOCUS_ERR1 2
#define FOCUS_ERR2 3
#define TOP_ERR 4
#define UKN_ERR 5

#define SC_X11_E1 D_ERR
#define SC_X11_E2 FOCUS_ERR1
#define SC_X11_E3 FOCUS_ERR2
#define SC_X11_E4 TOP_ERR
#define SC_X11_E0 UKN_ERR

unsigned int SC_get_active_X11window (Window *w, Window* *w_children, ssize_t *n);
	
gboolean SC_get_active_gdkwindow (Window aw, Window *aw_children, ssize_t n, GdkWindow* *gdkwindow);

void SC_get_geometry_for (Window aw, Window *aw_children, ssize_t n, GdkRectangle *win_rect, GdkRectangle *extents, GdkWindow* *gdkwindow); 

gboolean SC_get_active_windows_and_geometry (Window *aw, Window* *aw_children, ssize_t *n, GdkRectangle *win_rect, GdkRectangle *extents, GdkWindow* *gdkwindow); 

