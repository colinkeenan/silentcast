Silentcast
==========

<!--
BETTER TO VIEW THIS IN A MARKDOWN VIEWER
When viewed in a markdown viewer, you'll get pictures and even animations made with silentcast. If you don't have a markdown viewer, I personally use a Google Chrome Extension:

https://chrome.google.com/webstore/detail/markdown-preview/jmchmkecamhbiokiopfpnfgbidieafmd

After installing the Markdown Preview extenstion, you can't use it until you go into

chrome://extensions/

to checkmark the box under

Markdown Preview
 for *Allow access to file URLs*

Then, just open this file with Chrome. You'll probably want to make Google Chrome the default for .md files. You can also just drag and drop this file to an empty tab in Chrome.
-->

##Demos
Notice there's a stop icon in the Notificaton Area before I even start Silentcast, then a 2nd stop icon appears when recording begins. That's because I already had Silentcast running to make these animated gifs of how to use Silentcast. Silentcast 1 keeps going after I stop Silentcast 2.
###Fullscreen: How to use Silentcast to record Gimp
![Fullscreen](http://i.imgur.com/I1mXz9N.gif)
###Transparent: How to use Silentcast to record 2 windows
![Transparent](http://i.imgur.com/ak6NQZB.gif)
###Interior: How to use Silentcast to only record the drawing
![Interior](http://i.imgur.com/VAmUl8d.gif)
###Entirety: How to use Silentcast to record 1 window
![Entirety](http://i.imgur.com/XlWzLRW.gif)
##Guides
###Installation

- Desktop Environment Requirements
    - A desktop that includes a standard system-tray notification area with clickable icons.
    - [A compositing window manager](http://en.wikipedia.org/wiki/Compositing_window_manager#List_of_compositing_window_managers) that is compatible with the [EWMH/NetWM](http://en.wikipedia.org/wiki/Extended_Window_Manager_Hints) specification.

- Dependencies (Package names may vary slightly across different Linux distros. These are how they're named in Arch Linux.)
<table>
  <thead>
    <tr>
      <th>package</th>
      <th>reason</th>
    </tr>
  </thead>
  <tr>
    <td>bash</td>
    <td>because Silentcast is mostly bash scripts and I use bashisms</td>
  </tr>
  <tr>
    <td>ffmpeg</td>
    <td>for recording and extracting images</td>
  </tr>
  <tr>
    <td>imagemagick</td>
    <td>for 'convert' to animate images</td>
  </tr>
  <tr>
    <td>yad</td>
    <td>for the GUI - popup dialogue windows</td>
  </tr>
  <tr>
    <td>xrandr</td>
    <td>for getting screen size</td>
  </tr>
  <tr>
    <td>xdotool</td>
    <td>for getting the active window id</td>
  </tr>
  <tr>
    <td>xorg-xwininfo</td>
    <td>for getting window size and position</td>
  </tr>
  <tr>
    <td>wmctrl</td>
    <td>for resizing and positioning windows</td>
  </tr>
  <tr>
    <td>python-gobject</td>
    <td>for 'gi.repository' which has Gtk</td>
  </tr>
  <tr>
    <td>python-cairo</td>
    <td>for making Gtk+ window transparent</td>
  </tr>
  <tr>
    <td>xdg-utils</td>
    <td>for 'xdg-open' to open the file-browser</td>
  </tr>
</table>

- Arch Linux
    - After I've completed this README.md, I'll put it in the AUR so it would be installed with an AUR helper, like `yaourt -S silentcast`. This will automatically install missing dependencies. **Uninstall** with `pacman -R silentcast`
    - Without an AUR helper, just [Download Silentcast master.zip from github.com](https://github.com/colinkeenan/silentcast/archive/master.zip), extract, and do `makepkg -si` from the extracted directory. This will automatically install missing dependencies. **Uninstall** with `pacman -R silentcast`

- Any Linux Distro
    - Install missing dependencies (see the second point in this list)
    - [Download Silentcast master.zip from github.com](https://github.com/colinkeenan/silentcast/archive/master.zip) and extract. Then, either open the extracted folder from a file browser **as root** and double-click **install**, or from a terminal, `cd` into the extracted directory and `sudo ./install` **Uninstall** instructions are the same replacing *install* with *uninstall*. The **install** (or **uninstall**) bash script just copies (or deletes) files. You may want to edit them if your distro puts files in unusual places.

###Launch Methods

- Menu Hierarchy
    - Graphics -> Silentcast
    - Multimedia -> Silentcast
- Search Box Terms
    - silentcast
    - screencast
    - record
    - gif
    - (and other things will work too)
- ALT+F2
    - silentcast
- Terminal
    - silentcast

Find *Silentcast* in the menu under either *Graphics* or *Multimedia*, type *silentcast* into the search box, or *ALT+F2 silentcast*. It can also be run from a terminal as *silentcast*.
### Dialogues: Set 3 2 1 Record Stop Convert
#### Set Area to be recorded and Frames per second
#### 3 Auto Resize
#### 2 Manual Resize and Position
#### 1 Get Ready
#### Record
#### Stop
#### Convert
##List of Tips
