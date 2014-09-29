#!/usr/bin/python

from gi.repository import Gtk
from gi.repository import AppIndicator3 as appindicator
import os,argparse

parser = argparse.ArgumentParser()
parser.add_argument("silentcast_number", help="The number of Silentcast instances running. \
        The system tray indicator icon will show the silentcast_number in it: either 1 or 2.")
args = parser.parse_args()

class IndicatorSilentcast:
    def __init__(self):
        self.indicator = appindicator.Indicator.new_with_path (
                "silentcast", 
                "stop{}".format(args.silentcast_number),
                appindicator.IndicatorCategory.APPLICATION_STATUS,
                os.path.dirname(os.path.realpath(__file__)))
        self.indicator.set_status (appindicator.IndicatorStatus.ACTIVE)
        
        self.menu = Gtk.Menu()
        
        doneItem = Gtk.MenuItem("Done")
        doneItem.connect("activate", self.finishRecording)
        doneItem.show()
        self.menu.append(doneItem)
        
        self.menu.show()
        self.indicator.set_menu(self.menu)
        
    def finishRecording(self, widget):
            Gtk.main_quit()
            
def main():
    Gtk.main()
    exit(0)

if __name__ == "__main__":
    indicator = IndicatorSilentcast()
    main()
