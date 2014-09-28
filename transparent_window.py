#!/usr/bin/python
import cairo
from gi.repository import Gtk
import os,argparse,re                  # I added this so I could show the pid and Resize window keybinding in the title

parser = argparse.ArgumentParser()
parser.add_argument("all_keys", help='ie: all_keys=`xfconf-query -c xfce4-keyboard-shortcuts -lv`; python ./transparent_window.py \\\""$all_keys"\\\"')
args = parser.parse_args()

class MyWin (Gtk.Window):
    def __init__(self):
        super(MyWin, self).__init__()
        self.set_position(Gtk.WindowPosition.NONE)# I changed CENTER to NONE because it was hiding behind my dialogue window
                                        # I want to position it at top left, but all the WIN_POS are based on 
                                        # center or mouse related stuff, so moving it with wmctrl in the bash script
        self.set_title("PID: {} Resize with {} mouse".format(str(os.getpid()), self.get_resize_hotkey(args.all_keys)))
                                  # Showing pid in title so that if this is run outside of silentcast,
                                  # it will be easy to kill this process. Also showing hotkey for resizing the window.
        self.set_deletable(False) # I added this to remove the close button because closing
                                  # the window doesn't actually kill this process
        self.set_size_request(500, 220) # replaced the border setting which ends up being ignored with this size setting
        
        self.screen = self.get_screen()
        self.visual = self.screen.get_rgba_visual()
        if self.visual != None and self.screen.is_composited():
            self.set_visual(self.visual)

# removed some code about a box and button because I don't need it

        self.set_app_paintable(True)
        self.connect("draw", self.area_draw)
        self.show_all()

    def area_draw(self, widget, cr):
        cr.set_source_rgba(.2, .2, .2, 0.3)
        cr.set_operator(cairo.OPERATOR_SOURCE)
        cr.paint()
        cr.set_operator(cairo.OPERATOR_OVER)

    def get_resize_hotkey(self, all_hotkeys):
        try: 
            resize_hotkey=re.search('(?<=custom\/)<.+>\w*(?=.+resize_window_key)', all_hotkeys).group() + " and "
        except AttributeError:
            resize_hotkey=""
        return resize_hotkey

MyWin()
Gtk.main()
