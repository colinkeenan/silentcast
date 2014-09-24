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

source=('silentcast' 'genffcom' 'temptoanim' 'transparent_window.py' 'silentcast.desktop' 'localREADME.md' '1.png' '2_Trans.png' '3_Trans.png' 'entirety.gif' 'gnome-stop.png' 'numix-stop.png' 'SetArea.png' 'Stop.png' '2.png' '3.png' 'convert-error.png' 'fullscreen.gif' 'interior.gif' 'SetRate.png' 'transparent.gif')
md5sums=('6ec03de75e7ed4688e2a463104fc3a50'
         '3b7311751662d8bd3034e4720239b070'
         '710f4e2d60322bf4574406482c468343'
         '6bf9e695750a3f65e4421544c53bdb96'
         'f1641d3959f479a97b6bcd26f103f900'
         '104164134221a2f7e724f94a7545f4d7'
         '538ba7d29d4a72c16657c49c9088fad1'
         'e45db6d934a8d0da0377a2cc63a6467c'
         '42d1c677812d5740484ed26d95a82707'
         '72e906f125486ec9883c6a13d40e0e65'
         'f223e5e72c0c9e79789e8108d9df7d92'
         '8cd5da03d8f5035ad176b7b69de55ac4'
         'd639c30d697ee43e3e947b7d718efd48'
         '0fc214d9ef5c6b37765cfd1252860c30'
         'f2abb898a2371c6401ee2867cae91d86'
         'ab5da85b01aeff7076241aab74b592d1'
         '08180c0ba20917e301a40ba786c8b8a2'
         '92c971c4485bc8ffbb0882c381a7a4c9'
         'bfa2c9a9d28c3e046664d36a8964a44a'
         '7b1c0fd0240028ed0aaecf02f9087147'
         '7b6f5e2ce6fdc1f57a7ee3f687e73a05')

package() {
  install -D -m755 silentcast "$pkgdir/usr/bin/silentcast"
  install -D -m755 genffcom "$pkgdir/usr/bin/genffcom"
  install -D -m755 temptoanim "$pkgdir/usr/bin/temptoanim"
  install -D -m755 transparent_window.py "$pkgdir/usr/share/silentcast/transparent_window.py"
  install -D -m755 silentcast.desktop "$pkgdir/usr/share/applications/silentcast.desktop"
  install -D -m755 localREADME.md "$pkgdir/usr/share/doc/silentcast/README.md"
  install -m755 *png "$pkgdir/usr/share/doc/silentcast"
  install -m755 *gif "$pkgdir/usr/share/doc/silentcast"
}
