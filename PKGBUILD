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

source=('silentcast' 'genffcom' 'temptoanim' 'transparent_window.py' 'silentcast.desktop' 'README.md')
md5sums=('4e5bdee7c9c4dd6a23291888c89a0103'
         '3b7311751662d8bd3034e4720239b070'
         '38d4c1ca4e96fe929d2081dcde6c0a09'
         '6bf9e695750a3f65e4421544c53bdb96'
         'f1641d3959f479a97b6bcd26f103f900'
         'b5f20e1a42c132da14eda9f3f4a247ca')

package() {
  install -D -m755 silentcast "$pkgdir/usr/bin/silentcast"
  install -D -m755 genffcom "$pkgdir/usr/bin/genffcom"
  install -D -m755 temptoanim "$pkgdir/usr/bin/temptoanim"
  install -D -m755 transparent_window.py "$pkgdir/usr/share/silentcast/transparent_window.py"
  install -D -m755 silentcast.desktop "$pkgdir/usr/share/applications/silentcast.desktop"
  install -D -m755 README.md "$pkgdir/usr/share/doc/silentcast/README.md"
}
