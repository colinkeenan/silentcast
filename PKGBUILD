# Maintainer: Colin Keenan <colinnkeenan at gmail dot com>

pkgname=silentcast
pkgver=1.0
pkgrel=1
pkgdesc="Run from Guake. Create silent mkv screencast and animated gif."
arch=('any')
url="https://github.com/colinkeenan/silentcast"
license=('GPL')
depends=('zenity' 'ffmpeg' 'imagemagick' 'wmctrl' 'yad')
install=${pkgname}.install

source=('silentcast' 'genffcom' 'temptoanim' 'README.md' 'LICENSE')
md5sums=('d0fe2b3b5721c3ba7af8a4b1a009fdea'
         '731132dfaf66d4a9db94cd890340d612'
         '7215e35b694057d8a7608a3a8c1236fc'
         '7557e9f21bd6c4b16efe2af09e3e3db3'
         '783b7e40cdfb4a1344d15b1f7081af66')

package() {
  install -d -m755 "$pkgdir/usr/bin"
  install -m755 silentcast "$pkgdir/usr/bin/silentcast"
  install -m755 genffcom "$pkgdir/usr/bin/genffcom"
  install -m755 temptoanim "$pkgdir/usr/bin/temptoanim"
  install -m755 -D README.md "/usr/share/doc/silentcast/README.md"
}
