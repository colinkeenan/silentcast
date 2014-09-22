# Maintainer: Colin Keenan <colinnkeenan at gmail dot com>

pkgname=silentcast
pkgver=1.0
pkgrel=1
pkgdesc="Run from Guake. Create silent mkv screencast and animated gif."
arch=('any')
url="https://github.com/colinkeenan/silentcast"
license=('GPL')
depends=('ffmpeg' 'imagemagick' 'yad' 'xrandr' 'wmctrl' 'xdotool' 'xorg-xwininfo' 'python-gobject' 'python-cairo')
install=${pkgname}.install

source=('silentcast' 'genffcom' 'temptoanim' 'transparent_window.py' 'silentcast.desktop' 'README.md' 'fullscreen.gif' 'transparent.gif' 'interior.gif' 'entirety.gif' 'LICENSE')
md5sums=('95c75ec32bcf7ecca8768b683e55f1b3'
         'a8e85b898b7a2b1c5433753aec0b604c'
         '6ce4638b3358ca388838cf650083a528'
         'ef77cc762ea0b6e77665d64bedeee538'
         'ebe95d79842151e38ddd96fb5cc742c9'
         'c176276cce05f42d826ed997ac6b271f'
         '92c971c4485bc8ffbb0882c381a7a4c9'
         '7b6f5e2ce6fdc1f57a7ee3f687e73a05'
         'bfa2c9a9d28c3e046664d36a8964a44a'
         '72e906f125486ec9883c6a13d40e0e65'
         '783b7e40cdfb4a1344d15b1f7081af66')

package() {
  install -D -m755 silentcast "$pkgdir/usr/bin/silentcast"
  install -D -m755 genffcom "$pkgdir/usr/bin/genffcom"
  install -D -m755 temptoanim "$pkgdir/usr/bin/temptoanim"
  install -D -m755 transparent_window.py "$pkgdir/usr/share/silentcast/transparent_window.py"
  install -D -m755 silentcast.desktop "$pkgdir/usr/share/applications/silentcast.desktop"
  install -D -m755 README.md "$pkgdir/usr/share/doc/silentcast/README.md"
}
