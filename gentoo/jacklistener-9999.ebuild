# media-sound/jacklistener/jacklistener-9999.ebuild
EAPI=3

inherit autotools git-2 systemd

DESCRIPTION="Headphones Jack Listener Daemon"
HOMEPAGE="https://github.com/gentoo-root/jacklistener"
EGIT_REPO_URI="git://github.com/gentoo-root/jacklistener.git"

LICENSE="GPL-3"
SLOT="0"
KEYWORDS="~amd64 ~x86"
IUSE="systemd"

RDEPEND="sys-apps/dbus
		>=sys-kernel/linux-headers-3.2"
DEPEND="${RDEPEND}"

src_prepare() {
	eautoreconf
}

src_configure() {
	econf \
		--enable-openrc \
		$(use_enable systemd)
}

src_install() {
	emake DESTDIR="${D}" install || die
}
