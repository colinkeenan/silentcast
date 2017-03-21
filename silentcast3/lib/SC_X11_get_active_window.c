/*
 *  Filename: SC_X11_get_active_window.c 
 *  App Name: Silentcast <https://github.com/colinkeenan/silentcast>
 *  Copyright © 2016 Colin N Keenan <colinnkeenan@gmail.com>
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
 *  Description: adapted from "get the active window on X window system" 
 *               https://gist.github.com/kui/2622504
 *               to get Gdk geometry of the active window, both the
 *               inner window and the extents
 */


#include "SC_X11_get_active_window.h"

Bool xerror = False;

static int handle_error (Display* display, XErrorEvent* error) {
  xerror = True;
  return 1;
}

static int get_focus_window (Display* d, Window *w) {
  int revert_to;

  XGetInputFocus (d, w, &revert_to);
  if (xerror) return FOCUS_ERR1; //X error trying to get focused window
  else if (w == None) return FOCUS_ERR2; //no focused window
  else return 0;
}

static int get_top_window (Display* d, Window start, Window *w, Window* *w_children, ssize_t *n) {
  Window parent = start, root = None, *children = NULL;
  *w = start; 
  unsigned int nchildren;
  Status s = XQueryTree (d, *w, &root, &parent, &children, &nchildren), s_prev;

  /* ultimately trying to get *w and *w_children */
  while (parent != root && !xerror) {
    
    *w = parent; //previous parent
    s_prev = s; //previous status of XQueryTree
    if (s_prev) {
      *w_children = children; //previous children
      *n = nchildren; //previous number of children
    }
    
    s = XQueryTree (d, *w, &root, &parent, &children, &nchildren);
    /* When parent == root, the previous "parent" is the top window.
     * Save the children of the top window too, but XFree all other
     * children.
     */
    if (parent != root) {
    // parent is not root, so previous parent wasn't top window, so don't need it's children
      if (s_prev) XFree (*w_children); 
    } else 
      if (s) XFree (children); // don't keep the children of root either
  }
  if (xerror) return TOP_ERR;
  else return 0;
}

unsigned int 
SC_get_active_X11window (Window *w, Window* *w_children, ssize_t *n)
{
  Display* d = NULL;
  unsigned int e = 0;

  XSetErrorHandler (handle_error);
  d = XOpenDisplay (NULL); 
  if (d == NULL) { 
    return D_ERR; 
  } else {
    /* set w to the focused window */
    e = get_focus_window (d, w); 
    if (e) { //if error
      XCloseDisplay (d);
      return e;
    }
    /* get_top_window will set w to the top focused window (active window) */
    e = get_top_window (d, *w, w, w_children, n); 
    if (e) { //if error
      XCloseDisplay (d);
      return e;
    }
    XCloseDisplay(d);
  } 

  return 0; //no error
}

/* SC_get_active_gdkwindow (...) tries to match a GdkWindow to one of the passed X11
 * windows (supposed to be the active X11 window and it's n children), and returns
 * TRUE if such a match is found, FALSE if not
 */
gboolean
SC_get_active_gdkwindow (Window aw, Window *aw_children, ssize_t n, GdkWindow* *gdkwindow) {
  ssize_t i = 0;
  GdkWindow *dwindow = NULL;
  GdkScreen *screen = NULL;
  GList *gl_item = NULL, *gl = NULL;
  gboolean active_window_found = FALSE;
  

  screen = gdk_screen_get_default ();
  if (screen != NULL) { 
    /* Go through all windows known to Gtk and check XID against active X11 window, aw. */
    gl = gdk_screen_get_window_stack (screen);
    for (gl_item = g_list_first (gl); !active_window_found && gl_item != NULL; gl_item = gl_item->next) { 

      dwindow = gl_item->data;
      
      if (gdk_x11_window_get_xid (dwindow) == aw) active_window_found = TRUE;
      else for (i = 0; i < n; i++)  //aw didn't match this dwindow, so check all of aw_children
        if (gdk_x11_window_get_xid (dwindow) == aw_children[i]) active_window_found = TRUE;
      
      if (!active_window_found) g_object_unref (dwindow);
      else *gdkwindow = dwindow;
    } 
    g_list_free (gl);
  }
  return active_window_found;
}

/* SC_get_geometry_for (...) trys to get the Gdk geometry for the GdkWindow
 * matching the passed X11 window with children, getting both the internal
 * window geometry and it's extents (title-bar/frame). If can't get Gdk info
 * will get the X11 geometry, setting both inner and extents geometry to
 * the same values. 
 */

void
SC_get_geometry_for (Window aw, Window *aw_children, ssize_t n, GdkRectangle *win_rect, GdkRectangle *extents, GdkWindow* *dwindow) {
  unsigned int bwidth = 0, depth = 0, width, height;
  int x, y;
  Window root = 0;
  
  if (SC_get_active_gdkwindow (aw, aw_children, n, dwindow)) {
    gdk_window_get_frame_extents (*dwindow, extents); //{top-left corner, width & height} of title-bar/borders
    gdk_window_get_origin(*dwindow, &x, &y); //top-left corner of interior window (not title bar/borders)
    width = gdk_window_get_width (*dwindow); //width of interior window
    height = gdk_window_get_height (*dwindow); //height of interior window
    win_rect->x = x;
    win_rect->y = y;
    win_rect->width = (int) width;
    win_rect->height = (int) height;
  } else {
    fprintf (stderr, "Failed to get GdkWindow. Falling back on X11 geometry of active window, saved as both extents and interior geometry.");
    Display* d = XOpenDisplay (NULL); //mem leak
    if (d) {
      XGetGeometry (d, aw, &root, &x, &y, &width, &height, &bwidth, &depth);
      XCloseDisplay (d);
      extents->x = x;
      extents->y = y;
      extents->width = (int) width;
      extents->height = (int) height;
    }
  }
}

/* SC_get_active_windows_and_geometry (...) calls get_active_x11window (...) to get the active X11 window
 * and it's children, then calls SC_get_geometry_for (...) to get geometry (hopefully Gdk) that matches
 */
gboolean
SC_get_active_windows_and_geometry (Window *aw, Window* *aw_children, ssize_t *n, 
    GdkRectangle *win_rect, GdkRectangle *extents, GdkWindow* *dwindow) {
  
  switch (SC_get_active_X11window(aw, aw_children, n)) { //mem leak get aw, aw_children, and n (number of children)
case 0: SC_get_geometry_for (*aw, *aw_children, *n, win_rect, extents, dwindow); return TRUE; //mem leak
case SC_X11_E1: fprintf (stderr, SC_X11_ERROR1); break;
case SC_X11_E2: fprintf (stderr, SC_X11_ERROR2); break;
case SC_X11_E3: fprintf (stderr, SC_X11_ERROR3); break;
case SC_X11_E4: fprintf (stderr, SC_X11_ERROR4); break;
  }     
  return FALSE; //failed to get active window due to X11 error
}

