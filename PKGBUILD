# Maintainer: Colin Keenan <colinnkeenan at gmail dot com>

pkgname=silentcast
pkgver=1.0
pkgrel=1
pkgdesc="Run from Guake. Create silent mkv screencast and animated gif."
arch=('any')
url="https://github.com/colinkeenan/silentcast"
license=('GPL')
depends=('ffmpeg' 'imagemagick' 'wmctrl' 'yad' 'xdotool')
install=${pkgname}.install

source=('silentcast' 'genffcom' 'temptoanim' 'transparent_window.py' 'silentcast.desktop' 'README.md' 'LICENSE')
md5sums=('95c75ec32bcf7ecca8768b683e55f1b3'
         'a8e85b898b7a2b1c5433753aec0b604c'
         'fe3483490fffcbd6c24561d5758332d9'
         'ef77cc762ea0b6e77665d64bedeee538'
         'ebe95d79842151e38ddd96fb5cc742c9'
         'da31900b9d06443b5c8d07889ff5a301'
         '783b7e40cdfb4a1344d15b1f7081af66')

package() {
  install -D -m755 silentcast "$pkgdir/usr/bin/silentcast"
  install -D -m755 genffcom "$pkgdir/usr/bin/genffcom"
  install -D -m755 temptoanim "$pkgdir/usr/bin/temptoanim"
  install -D -m755 transparent_window.py "$pkgdir/usr/share/silentcast/transparent_window.py"
  install -D -m755 silentcast.desktop "$pkgdir/usr/share/applications/silentcast.desktop"
  install -D -m755 README.md "$pkgdir/usr/share/doc/silentcast/README.md"
}
