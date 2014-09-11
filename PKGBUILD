# Maintainer: Colin Keenan <colinnkeenan at gmail dot com>

pkgname=silentcast
pkgver=1.0
pkgrel=1
pkgdesc="Run from Guake. Create silent mkv screencast and animated gif."
arch=('any')
url="https://github.com/colinkeenan/silentcast"
license=('GPL')
depends=('zenity' 'ffmpeg' 'imagemagick')
install=${pkgname}.install

source=('silentcast' 'genffcom' 'temptoanim')
md5sums=('9b40ab463a80019dfe58856f0190c784')

package() {
  install -d -m755 "$pkgdir/usr/bin"
  install -m755 silentcast "$pkgdir/usr/bin/silentcast"
  install -m755 genffcom "$pkgdir/usr/bin/genffcom"
  install -m755 temptoanim "$pkgdir/usr/bin/temptoanim"
}
