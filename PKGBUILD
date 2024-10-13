# Note that you'll need to supply the UE1 games you want to play yourself
# And need a Vulkan capable GPU to run SurrealEngine

pkgname=surrealengine-git
pkgdesc="A reimplementation of Unreal Engine 1 (Git version)"
pkgver=r1042.784a270
pkgrel=1
arch=('x86_64')
depends=('libx11' 'spirv-tools' 'vulkan-icd-loader' 'vulkan-driver' 'sdl2' 'alsa-lib')
makedepends=('gcc' 'git' 'cmake' 'vulkan-driver' 'sdl2' 'alsa-lib' 'waylandpp')
optdepends=('waylandpp: For ZWidget Wayland backend')
provides=('surrealengine')
url="https://github.com/dpjudas/SurrealEngine"
license=('BSD-3-Clause AND MIT AND various others')
source=("git+${url}.git")
sha512sums=('SKIP')

pkgver() {
  cd "${srcdir}/SurrealEngine"
  printf "r%s.%s" "$(git rev-list --count HEAD)" "$(git rev-parse --short=7 HEAD)"
}

build() {
  cd SurrealEngine
  [[ -d build ]] && rm -rf build
  mkdir build && cd build
  cmake -DCMAKE_BUILD_TYPE=Release ..

  make -j 16
}

package() {

  # Copy the built binaries
  install -Dm755 "${srcdir}/SurrealEngine/build/SurrealEngine" "${pkgdir}/usr/bin/surrealengine"
  install -Dm755 "${srcdir}/SurrealEngine/build/SurrealEditor" "${pkgdir}/usr/bin/surrealeditor"
  install -Dm755 "${srcdir}/SurrealEngine/build/SurrealDebugger" "${pkgdir}/usr/bin/surrealdebugger"

  # Copy SurrealEngine.pk3
  install -Dm755 "${srcdir}/SurrealEngine/build/SurrealEngine.pk3" "${pkgdir}/usr/share/surrealengine/SurrealEngine.pk3"

  # Copy license file and readme
  install -Dm644 "${srcdir}/SurrealEngine/LICENSE.md" "${pkgdir}/usr/share/licenses/${pkgname}"

  mkdir -p "${pkgdir}/usr/share/doc/surrealengine/"

  install -Dm644 "${srcdir}/SurrealEngine/README.md" "${pkgdir}/usr/share/doc/surrealengine/readme.md"
}

post_install() {
	cat <<- EOF
		:: You need the legit copies of UE1 games you want to play installed.
		:: You can either run them from the command line like such:
		:: surrealengine /path/to/UE1game
		:: Or copy the surrealengine executable into the system folders of the
		:: said UE1 games
	EOF
}

post_upgrade() {
	post_install $1
}
