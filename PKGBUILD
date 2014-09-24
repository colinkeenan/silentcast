# Maintainer: Colin Keenan <colinnkeenan at gmail dot com>

pkgname=silentcast
pkgver=1.0
pkgrel=1
pkgdesc="Silent Screencast: video record your screen and make it into an animated gif"
arch=('any')
url="https://github.com/colinkeenan/silentcast"
license=('GPL')
depends=('bash' 'ffmpeg' 'imagemagick' 'yad' 'xorg-xrandr' 'wmctrl' 'xdotool' 'xorg-xwininfo' 'python-gobject' 'python-cairo' 'xdg-utils')
install=${pkgname}.install

source=(https://github.com/colinkeenan/silentcast/archive/master.zip)
md5sums=('SKIP')

package() {
  install -d -m755 silentcast "$pkgdir/usr/bin/"
  install -d -m755 genffcom "$pkgdir/usr/bin/"
  install -d -m755 temptoanim "$pkgdir/usr/bin/"
  install -d -m755 transparent_window.py "$pkgdir/usr/share/silentcast/"
  install -d -m755 silentcast.desktop "$pkgdir/usr/share/applications/"
  install -d -m755 localREADME.md "$pkgdir/usr/share/doc/silentcast/"
  install -m755 1.png "$pkgdir/usr/share/doc/silentcast"
  install -m755 2.png "$pkgdir/usr/share/doc/silentcast"
  install -m755 3.png "$pkgdir/usr/share/doc/silentcast"
  install -m755 2_Trans.png "$pkgdir/usr/share/doc/silentcast"
  install -m755 3_Trans.png "$pkgdir/usr/share/doc/silentcast"
  install -m755 convert-error.png "$pkgdir/usr/share/doc/silentcast"
  install -m755 gnome-stop.png "$pkgdir/usr/share/doc/silentcast"
  install -m755 numix-stop.png "$pkgdir/usr/share/doc/silentcast"
  install -m755 SetArea.png "$pkgdir/usr/share/doc/silentcast"
  install -m755 SetRate.png "$pkgdir/usr/share/doc/silentcast"
  install -m755 Stop.png "$pkgdir/usr/share/doc/silentcast"
  install -m755 entirety.gif "$pkgdir/usr/share/doc/silentcast"
  install -m755 fullscreen.gif "$pkgdir/usr/share/doc/silentcast"
  install -m755 interior.gif "$pkgdir/usr/share/doc/silentcast"
  install -m755 transparent.gif "$pkgdir/usr/share/doc/silentcast"
}
