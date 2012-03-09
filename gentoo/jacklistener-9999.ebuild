# media-sound/jacklistener/jacklistener-9999.ebuild
EAPI=3

inherit git-2

DESCRIPTION="Headphones Jack Listener Daemon"
HOMEPAGE="https://github.com/gentoo-root/jacklistener"
EGIT_REPO_URI="git://github.com/gentoo-root/jacklistener.git"

LICENSE="GPL-3"
SLOT="0"
KEYWORDS="~amd64 ~x86"
IUSE=""

RDEPEND="sys-apps/dbus"
DEPEND="${RDEPEND}"

src_install() {
	emake DESTDIR="${D}" install || die
	dodoc AUTHORS NEWS README* ChangeLog
}
