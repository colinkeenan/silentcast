# Run with python2, not using !/usr/bin/env python2 because if ran by double clicking,
# most people won't realize how to stop it. Even getting the window closed doesn't 
# actually stop this process

# Copied from the answer at http://stackoverflow.com/questions/19577367/python-how-to-make-transparent-window-with-gtk-window-but-not-with-gtk-window
import cairo
import gtk
import os,sys,re                  # I added this so I could show the pid and Resize window keybinding in the title

class MyWin (gtk.Window):
  def __init__(self):
    super(MyWin, self).__init__()
    self.set_position(gtk.WIN_POS_NONE) # I changed CENTER to NONE because it was hiding behind my dialogue window
                                        # I want to position it at top left, but all the WIN_POS are based on 
                                        # center related stuff, so moving it with wmctrl in the bash script
    
    self.set_title("PID:" + str(os.getpid()) + " Resize with " + self.get_resize_hotkey(str(sys.argv[1]))) 
                              # Showing pid in title so that if this is run outside of silentcast,
                              # it will be easy to kill this process. Also showing hotkey for resizing the window.
    self.set_deletable(False) # I added this to remove the close button because closing
                              # the window doesn't actually kill this process

    self.set_size_request(500, 220)
    self.set_border_width(11)

    self.screen = self.get_screen()
    colormap = self.screen.get_rgba_colormap()
    if (colormap is not None and self.screen.is_composited()):
        self.set_colormap(colormap)

    self.set_app_paintable(True)
    self.connect("expose-event", self.area_draw)
    self.show_all()

  def area_draw(self, widget, event):
    cr = widget.get_window().cairo_create()
    cr.set_source_rgba(.2, .2, .2, 0.3)
    cr.set_operator(cairo.OPERATOR_SOURCE)
    cr.paint()
    cr.set_operator(cairo.OPERATOR_OVER)
    return False

  def get_resize_hotkey(self, all_hotkeys):
      try: 
          resize_hotkey=re.search('(?<=custom\/)<.+>\w*(?=.+resize_window_key)', all_hotkeys).group()
      except AttributeError:
          resize_hotkey=""
      return resize_hotkey

MyWin()
gtk.main()
