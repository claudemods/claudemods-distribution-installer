#!/bin/bash

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
CYAN='\033[0;36m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color
BOLD='\033[1m'

# ASCII Art Banner
echo -e "${RED}"
cat << "EOF"
 ░█████╗░██╗░░░░░░█████╗░██╗░░░██╗██████╗░███████╗███╗░░░███╗░█████╗░██████╗░░██████╗
 ██╔══██╗██║░░░░░██╔══██╗██║░░░██║██╔══██╗██╔════╝████╗░████║██╔══██╗██╔══██╗██╔════╝
 ██║░░╚═╝██║░░░░░███████║██║░░░██║██║░░██║█████╗░░██╔████╔██║██║░░██║██║░░██║╚█████╗░
 ██║░░██╗██║░░░░░██╔══██║██║░░░██║██║░░██║██╔══╝░░██║╚██╔╝██║██║░░██║██║░░██║░╚═══██╗
 ╚█████╔╝███████╗██║░░██║╚██████╔╝██████╔╝███████╗██║░╚═╝░██║╚█████╔╝██████╔╝██████╔╝
 ░╚════╝░╚══════╝╚═╝░░░░░░╚═════╝░╚══════╝░╚══════╝╚═╝░░░░░╚═╝░╚════╝░╚═════╝░╚═════╝░
EOF

echo -e "${CYAN}"
echo "           ClaudeMods Vanilla Arch Kde Grub to Apex CKGE Minimal v1.0 27-10-2025"
echo -e "${NC}"
echo "================================================================================"
echo ""

# Function for colored output
print_status() {
    echo -e "${GREEN}[✓]${NC} $1"
}

print_warning() {
    echo -e "${YELLOW}[!]${NC} $1"
}

print_error() {
    echo -e "${RED}[✗]${NC} $1"
}

print_info() {
    echo -e "${CYAN}[i]${NC} $1"
}

print_section() {
    echo -e "${BLUE}${BOLD}[=== $1 ===]${NC}"
}

# Check if running as root
if [[ $EUID -eq 0 ]]; then
    print_error "This script should not be run as root. Please run as regular user."
    exit 1
fi

# Warning message
echo -e "${YELLOW}${BOLD}"
echo "WARNING: This script will perform major system modifications including:"
echo "• Adding CachyOS repositories"
echo "• Installing 1000+ packages"
echo "• Modifying bootloader configuration"
echo "• Changing system themes and configurations"
echo -e "${NC}"
echo -e "${RED}${BOLD}Ensure you have backups and understand the risks before proceeding!${NC}"
echo ""

read -p "Do you want to continue? (yes/no): " confirm
if [[ $confirm != "yes" ]]; then
    print_error "Installation cancelled by user."
    exit 0
fi

# Check and remove virt-manager if installed
print_section "Checking for virt-manager"
if pacman -Qi virt-manager &>/dev/null; then
    print_info "virt-manager found, removing it..."
    sudo pacman -Rns --noconfirm virt-manager
    print_status "virt-manager removed successfully"
else
    print_info "virt-manager not installed, proceeding..."
fi

print_section "Starting CachyOS Conversion Process"

# Step 1: Download and setup CachyOS repositories
print_section "Step 1: Setting up CachyOS Repositories"
print_info "Downloading CachyOS repository package..."
curl https://mirror.cachyos.org/cachyos-repo.tar.xz -o cachyos-repo.tar.xz
print_status "Download completed"

print_info "Extracting repository files..."
tar xvf cachyos-repo.tar.xz && cd cachyos-repo
print_status "Extraction completed"

print_info "Running CachyOS repository setup..."
sudo ./cachyos-repo.sh
print_status "Repository setup completed"

# Step 2: Massive package installation
print_section "Step 2: Installing Packages"
print_info "Starting installation (this will take a while)..."
print_info "Please be patient as this process may take 30-60 minutes..."

sudo pacman -S --needed \
a52dec \
aalib \
abseil-cpp \
accounts-qml-module \
accountsservice \
acl \
adobe-source-han-sans-cn-fonts \
adobe-source-han-sans-jp-fonts \
adobe-source-han-sans-kr-fonts \
adwaita-cursors \
adwaita-fonts \
adwaita-icon-theme \
adwaita-icon-theme-legacy \
aha \
alacritty \
alsa-card-profiles \
alsa-firmware \
alsa-lib \
alsa-plugins \
alsa-topology-conf \
alsa-ucm-conf \
alsa-utils \
amd-ucode \
ananicy-cpp \
android-tools \
android-udev \
aom \
appstream \
appstream-qt \
arch-install-scripts \
archlinux-appstream-data \
archlinux-keyring \
ark \
at-spi2-core \
atkmm \
attica \
attr \
audit \
aurorae \
autoconf \
automake \
avahi \
awesome-terminal-fonts \
baloo \
baloo-widgets \
base \
base-devel \
bash \
bash-completion \
bat \
bind \
binutils \
bison \
blas \
bluedevil \
bluez \
bluez-hid2hci \
bluez-libs \
bluez-qt \
bluez-utils \
bolt \
boost-libs \
bpf \
brave-bin \
breeze \
breeze-gtk \
breeze-icons \
brotli \
btop \
btrfs-progs \
bubblewrap \
bzip2 \
ca-certificates \
ca-certificates-mozilla \
ca-certificates-utils \
cachyos-alacritty-config \
cachyos-ananicy-rules \
cachyos-emerald-kde-theme-git \
cachyos-fish-config \
cachyos-grub-theme \
cachyos-hello \
cachyos-hooks \
cachyos-iridescent-kde \
cachyos-kde-settings \
cachyos-kernel-manager \
cachyos-keyring \
cachyos-micro-settings \
cachyos-mirrorlist \
cachyos-nord-kde-theme-git \
cachyos-packageinstaller \
cachyos-plymouth-bootanimation \
cachyos-rate-mirrors \
cachyos-settings \
cachyos-themes-sddm \
cachyos-v3-mirrorlist \
cachyos-v4-mirrorlist \
cachyos-wallpapers \
cachyos-zsh-config \
cairo \
cairomm \
cairomm-1.16 \
cantarell-fonts \
capitaine-cursors \
capstone \
cblas \
cdparanoia \
cdrtools \
cfitsio \
char-white \
chromaprint \
chwd \
cifs-utils \
ckbcomp \
clang \
clinfo \
cmake \
compiler-rt \
confuse \
convertlit \
coreutils \
cpio \
cppdap \
cpupower \
cracklib \
cryptsetup \
curl \
dav1d \
db5.3 \
dbus \
dbus-broker \
dbus-broker-units \
dbus-units \
dconf \
ddcutil \
debugedit \
default-cursors \
desktop-file-utils \
device-mapper \
dhclient \
diffutils \
ding-libs \
discount \
discover \
dmidecode \
dmraid \
dnsmasq \
dnssec-anchors \
dolphin \
dosfstools \
double-conversion \
dtc \
duf \
duktape \
e2fsprogs \
ebook-tools \
editorconfig-core-c \
edk2-ovmf \
efibootmgr \
efitools \
efivar \
egl-wayland \
eglexternalplatform \
ell \
enchant \
erofs-utils \
ethtool \
exfatprogs \
exiv2 \
expac \
exfatprogs \
exiv2 \
expac \
eza \
f2fs-tools \
faac \
faad2 \
fakeroot \
fastfetch \
fd \
ffmpeg \
ffmpegthumbnailer \
ffmpegthumbs \
fftw \
file \
filelight \
filesystem \
findutils \
fish \
fish-autopair \
fish-pure-prompt \
fisher \
flac \
flashrom \
flex \
fluidsynth \
fmt \
fontconfig \
frameworkintegration \
freeglut \
freetype2 \
fribidi \
fsarchiver \
fuse-common \
fuse2 \
fuse3 \
fwupd \
fwupd-efi \
fzf \
gawk \
gc \
gcc \
gcc-libs \
gcr-4 \
gdbm \
gdk-pixbuf2 \
gettext \
gfxstream \
ghostscript \
giflib \
git \
glances \
glib-networking \
glib2 \
glib2-devel \
glibc \
glibmm \
glibmm-2.68 \
glm \
glslang \
glu \
glycin \
gmp \
gnome-boxes \
gnulib-l10n \
gnupg \
gnutls \
gobject-introspection-runtime \
gparted \
gperftools \
gpgme \
gpgmepp \
gpm \
gptfdisk \
graphene \
graphite \
grep \
groff \
grub \
grub-hook \
gsettings-desktop-schemas \
gsettings-system-schemas \
gsm \
gssdp \
gssproxy \
gst-libav \
gst-plugin-pipewire \
gst-plugins-bad \
gst-plugins-bad-libs \
gst-plugins-base \
gst-plugins-base-libs \
gst-plugins-good \
gst-plugins-ugly \
gstreamer \
gtest \
gtk-update-icon-cache \
gtk-vnc \
gtk3 \
gtk4 \
gtkmm-4.0 \
gtkmm3 \
gtksourceview4 \
guile \
gupnp \
gupnp-igd \
gwenview \
gzip \
harfbuzz \
harfbuzz-icu \
haruna \
haveged \
hdparm \
hicolor-icon-theme \
hidapi \
highway \
htop \
hunspell \
hwdata \
hwdetect \
hwinfo \
hwloc \
hyphen \
i2c-tools \
iana-etc \
icu \
ijs \
imagemagick \
imath \
imlib2 \
inetutils \
iniparser \
inxi \
iproute2 \
iptables-nft \
iputils \
iso-codes \
iw \
iwd \
jack2 \
jansson \
jasper \
jbig2dec \
jbigkit \
jemalloc \
jfsutils \
jq \
json-c \
json-glib \
jsoncpp \
kaccounts-integration \
kactivitymanagerd \
karchive \
kate \
kauth \
kbd \
kbookmarks \
kcalc \
kcmutils \
kcodecs \
kcolorpicker \
kcolorscheme \
kcompletion \
kconfig \
kconfigwidgets \
kcontacts \
kcoreaddons \
kcrash \
kdbusaddons \
kde-cli-tools \
kde-gtk-config \
kdeclarative \
kdeconnect \
kdecoration \
kded \
kdegraphics-mobipocket \
kdegraphics-thumbnailers \
kdeplasma-addons \
kdesu \
kdialog \
kdnssd \
kdsingleapplication \
kdsoap-qt6 \
kdsoap-ws-discovery-client \
kernel-modules-hook \
keyutils \
kfilemetadata \
kglobalaccel \
kglobalacceld \
kguiaddons \
kholidays \
ki18n \
kiconthemes \
kidletime \
kimageannotator \
kimageformats \
kinfocenter \
kio \
kio-admin \
kio-extras \
kio-fuse \
kirigami \
kirigami-addons \
kitemmodels \
kitemviews \
kjobwidgets \
kmenuedit \
kmod \
knewstuff \
knighttime \
knotifications \
knotifyconfig \
konsole \
kpackage \
kparts \
kpeople \
kpipewire \
kpmcore \
kpty \
kquickcharts \
krb5 \
krunner \
kscreen \
kscreenlocker \
kservice \
kstatusnotifieritem \
ksvg \
ksystemlog \
ksystemstats \
ktexteditor \
ktextwidgets \
kunitconversion \
kuserfeedback \
kwallet \
kwallet-pam \
kwalletmanager \
kwayland \
kwidgetsaddons \
kwin \
kwindowsystem \
kxmlgui \
l-smash \
lame \
lapack \
layer-shell-qt \
layer-shell-qt5 \
lcms2 \
ldb \
leancrypto \
less \
libaccounts-glib \
libaccounts-qt \
libaemu \
libaio \
libappimage \
libarchive \
libass \
libassuan \
libasyncns \
libatasmart \
libavc1394 \
libavif \
libavtp \
libb2 \
libblockdev \
libblockdev-crypto \
libblockdev-fs \
libblockdev-loop \
libblockdev-mdraid \
libblockdev-nvme \
libblockdev-part \
libblockdev-swap \
libbluray \
libbpf \
libbs2b \
libbsd \
libburn \
libbytesize \
libcaca \
libcacard \
libcanberra \
libcap \
libcap-ng \
libcbor \
libcdio \
libcdio-paranoia \
libcloudproviders \
libcolord \
libcups \
libdaemon \
libdatrie \
libdc1394 \
libdca \
libde265 \
libdecor \
libdeflate \
libdisplay-info \
libdmtx \
libdnet \
libdovi \
libdrm \
libdv \
libdvdcss \
libdvdnav \
libdvdread \
libebur128 \
libedit \
libei \
libelf \
libepoxy \
libevdev \
libevent \
libfakekey \
libfdk-aac \
libffi \
libfontenc \
libfreeaptx \
libftdi \
libfyaml \
libgbinder \
libgcrypt \
libgirepository \
libgit2 \
libglibutil \
libglvnd \
libgme \
libgpg-error \
libgsf \
libgudev \
libhandy \
libice \
libidn \
libidn2 \
libiec61883 \
libimobiledevice \
libimobiledevice-glue \
libinih \
libinput \
libinstpatch \
libisl \
libisoburn \
libisofs \
libjcat \
libjpeg-turbo \
libjxl \
libkdcraw \
libkexiv2 \
libksba \
libkscreen \
libksysguard \
liblc3 \
libldac \
libldap \
liblqr \
liblrdf \
libltc \
libmanette \
libmaxminddb \
libmbim \
libmd \
libmicrodns \
libmm-glib \
libmng \
libmnl \
libmodplug \
libmpc \
libmpcdec \
libmpeg2 \
libmspack \
libmtp \
libmysofa \
libnbd \
libndp \
libnetfilter_conntrack \
libnewt \
libnfnetlink \
libnfs \
libnftnl \
libnghttp2 \
libnghttp3 \
libnice \
libnl \
libnm \
libnma-common \
libnma-gtk4 \
libnotify \
libnsl \
libnvme \
libogg \
libopenmpt \
libopenraw \
libosinfo \
libp11-kit \
libpaper \
libpcap \
libpciaccess \
libpgm \
libpipeline \
libpipewire \
libplacebo \
libplasma \
libplist \
libpng \
libportal \
libportal-gtk3 \
libproxy \
libpsl \
libpulse \
libpwquality \
libqaccessibilityclient-qt6 \
libqalculate \
libqmi \
libqrtr-glib \
libraqm \
libratbag \
libraw \
libraw1394 \
librsvg \
libsamplerate \
libsasl \
libseccomp \
libsecret \
libshout \
libsigc++ \
libsigc++-3.0 \
libsixel \
libslirp \
libsm \
libsndfile \
libsodium \
libsoup3 \
libsoxr \
libsrtp \
libssh \
libssh2 \
libstemmer \
libsysprof-capture \
libtasn1 \
libtatsu \
libteam \
libthai \
libtheora \
libtiff \
libtirpc \
libtommath \
libtool \
libtraceevent \
libtracefs \
libunibreak \
libunistring \
libunwind \
liburcu \
liburing \
libusb \
libusbmuxd \
libutempter \
libuv \
libva \
libvdpau \
libverto \
libvirt \
libvirt-glib \
libvirt-python \
libvlc \
libvorbis \
libvpl \
libvpx \
libwacom \
libwbclient \
libwebp \
libwireplumber \
libwnck3 \
libx11 \
libx86emu \
libxau \
libxaw \
libxcb \
libxcomposite \
libxcrypt \
libxcursor \
libxcvt \
libxdamage \
libxdmcp \
libxdp \
libxext \
libxfixes \
libxfont2 \
libxft \
libxi \
libxinerama \
libxkbcommon \
libxkbcommon-x11 \
libxkbfile \
libxml2 \
libxmlb \
libxmu \
libxpm \
libxpresent \
libxrandr \
libxrender \
libxres \
libxshmfence \
libxslt \
libxss \
libxt \
libxtst \
libxv \
libxvmc \
libxxf86vm \
libyaml \
libyuv \
libzip \
licenses \
lilv \
linux-api-headers \
linux-cachyos \
linux-cachyos-headers \
linux-cachyos-lts \
linux-cachyos-lts-headers \
linux-firmware \
linux-firmware-amdgpu \
linux-firmware-atheros \
linux-firmware-broadcom \
linux-firmware-cirrus \
linux-firmware-intel \
linux-firmware-mediatek \
linux-firmware-nvidia \
linux-firmware-other \
linux-firmware-radeon \
linux-firmware-realtek \
linux-firmware-whence \
lld \
llhttp \
llvm \
llvm-libs \
lm_sensors \
lmdb \
logrotate \
lsb-release \
lsscsi \
lua \
luajit \
lv2 \
lvm2 \
lxc \
lz4 \
lzo \
m4 \
mailcap \
make \
man-db \
man-pages \
md4c \
mdadm \
media-player-info \
meld \
mesa \
mesa-utils \
micro \
milou \
minizip \
mjpegtools \
mkinitcpio \
mkinitcpio-archiso \
mkinitcpio-busybox \
mkinitcpio-nfs-utils \
mkinitcpio-openswap \
mobile-broadband-provider-info \
modemmanager \
modemmanager-qt \
mpdecimal \
mpfr \
mpg123 \
mpv \
mpvqt \
mtdev \
mtools \
mujs \
nano \
nano-syntax-highlighting \
nbd \
ncurses \
ndctl \
neon \
netctl \
nettle \
networkmanager \
networkmanager-openvpn \
networkmanager-qt \
networkmanager-vpn-plugin-openvpn \
nfs-utils \
nfsidmap \
nftables \
nilfs-utils \
noto-color-emoji-fontconfig \
noto-fonts \
noto-fonts-cjk \
noto-fonts-emoji \
npth \
nspr \
nss \
nss-mdns \
ntp \
numactl \
ocean-sound-theme \
ocl-icd \
octopi \
oh-my-zsh-git \
onetbb \
oniguruma \
open-vm-tools \
openal \
opencore-amr \
opencv \
opendesktop-fonts \
openexr \
openh264 \
openjpeg2 \
openssh \
openssl \
openvpn \
openxr \
opus \
orc \
os-prober \
osinfo-db \
p11-kit \
pacman \
pacman-contrib \
pacman-mirrorlist \
pacutils \
pahole \
pam \
pambase \
pango \
pangomm \
pangomm-2.48 \
parallel \
parted \
paru \
passim \
patch \
pavucontrol \
pciutils \
pcre \
pcre2 \
pcsclite \
perl \
perl-clone \
perl-encode-locale \
perl-error \
perl-file-listing \
perl-html-parser \
perl-html-tagset \
perl-http-cookiejar \
perl-http-cookies \
perl-http-daemon \
perl-http-date \
perl-http-message \
perl-http-negotiate \
perl-io-html \
perl-libwww \
perl-lwp-mediatypes \
perl-mailtools \
perl-net-http \
perl-timedate \
perl-try-tiny \
perl-uri \
perl-www-robotrules \
perl-xml-parser \
perl-xml-writer \
phodav \
phonon-qt6 \
phonon-qt6-vlc \
pinentry \
piper \
pipewire \
pipewire-alsa \
pipewire-audio \
pipewire-pulse \
pixman \
pkcs11-helper \
pkgconf \
pkgfile \
plasma-activities \
plasma-activities-stats \
plasma-browser-integration \
plasma-desktop \
plasma-firewall \
plasma-integration \
plasma-nm \
plasma-pa \
plasma-systemmonitor \
plasma-thunderbolt \
plasma-workspace \
plasma5support \
plocate \
plymouth \
plymouth-kcm \
polkit \
polkit-kde-agent \
polkit-qt6 \
poppler \
poppler-data \
poppler-glib \
poppler-qt6 \
popt \
portaudio \
power-profiles-daemon \
powerdevil \
powerline-fonts \
ppp \
prison \
procps-ng \
protobuf \
protobuf-c \
psmisc \
pulseaudio-qt \
purpose \
pv \
python \
python-annotated-types \
python-babel \
python-cairo \
python-chardet \
python-charset-normalizer \
python-dbus \
python-defusedxml \
python-docutils \
python-evdev \
python-gbinder \
python-gobject \
python-idna \
python-imagesize \
python-jinja \
python-lxml \
python-markupsafe \
python-orjson \
python-packaging \
python-psutil \
python-pydantic \
python-pydantic-core \
python-pygments \
python-pytz \
python-requests \
python-roman-numerals-py \
python-snowballstemmer \
python-sphinx \
python-sphinx-alabaster-theme \
python-sphinxcontrib-applehelp \
python-sphinxcontrib-devhelp \
python-sphinxcontrib-htmlhelp \
python-sphinxcontrib-jsmath \
python-sphinxcontrib-qthelp \
python-sphinxcontrib-serializinghtml \
python-typing-inspection \
python-typing_extensions \
python-urllib3 \
qca-qt6 \
qcoro \
qemu-audio-alsa \
qemu-audio-dbus \
qemu-audio-jack \
qemu-audio-oss \
qemu-audio-pa \
qemu-audio-pipewire \
qemu-audio-sdl \
qemu-audio-spice \
qemu-base \
qemu-block-curl \
qemu-block-dmg \
qemu-block-nfs \
qemu-block-ssh \
qemu-chardev-spice \
qemu-common \
qemu-desktop \
qemu-guest-agent \
qemu-hw-display-qxl \
qemu-hw-display-virtio-gpu \
qemu-hw-display-virtio-gpu-gl \
qemu-hw-display-virtio-gpu-pci \
qemu-hw-display-virtio-gpu-pci-gl \
qemu-hw-display-virtio-gpu-pci-rutabaga \
qemu-hw-display-virtio-gpu-rutabaga \
qemu-hw-display-virtio-vga \
qemu-hw-display-virtio-vga-gl \
qemu-hw-display-virtio-vga-rutabaga \
qemu-hw-uefi-vars \
qemu-hw-usb-host \
qemu-hw-usb-redirect \
qemu-hw-usb-smartcard \
qemu-img \
qemu-system-x86 \
qemu-system-x86-firmware \
qemu-ui-curses \
qemu-ui-dbus \
qemu-ui-egl-headless \
qemu-ui-gtk \
qemu-ui-opengl \
qemu-ui-sdl \
qemu-ui-spice-app \
qemu-ui-spice-core \
qemu-vhost-user-gpu \
qqc2-breeze-style \
qqc2-desktop-style \
qrencode \
qt-sudo \
qt5-base \
qt5-declarative \
qt5-tools \
qt5-translations \
qt5-wayland \
qt6-5compat \
qt6-base \
qt6-connectivity \
qt6-declarative \
qt6-imageformats \
qt6-location \
qt6-multimedia \
qt6-multimedia-ffmpeg \
qt6-positioning \
qt6-quick3d \
qt6-quicktimeline \
qt6-sensors \
qt6-shadertools \
qt6-speech \
qt6-svg \
qt6-tools \
qt6-translations \
qt6-virtualkeyboard \
qt6-wayland \
qt6-webchannel \
qt6-webengine \
qt6-websockets \
qt6-webview \
qtermwidget \
raptor \
rate-mirrors \
rav1e \
rdma-core \
re2 \
readline \
rebuild-detector \
reflector \
rhash \
ripgrep \
ripgrep-all \
rpcbind \
rsync \
rtkit \
rtmpdump \
rubberband \
run-parts \
rutabaga-ffi \
s-nail \
sbc \
scrcpy \
scx-manager \
scx-scheds \
sd \
sddm \
sddm-kcm \
sdl2-compat \
sdl2_image \
sdl3 \
seabios \
sed \
serd \
sg3_utils \
shaderc \
shadow \
shared-mime-info \
signon-kwallet-extension \
signon-plugin-oauth2 \
signon-ui \
signond \
slang \
smartmontools \
smbclient \
snappy \
socat \
sof-firmware \
solid \
sonnet \
sord \
sound-theme-freedesktop \
soundtouch \
spandsp \
spdlog \
spectacle \
speex \
speexdsp \
spice \
spice-gtk \
spice-protocol \
spice-vdagent \
spirv-tools \
sqlite \
squashfs-tools \
squashfuse \
sratom \
srt \
sshfs \
startup-notification \
sudo \
svt-av1 \
svt-hevc \
syndication \
syntax-highlighting \
sysfsutils \
syslinux \
systemd \
systemd-libs \
systemd-resolvconf \
systemd-sysvcompat \
systemsettings \
taglib \
talloc \
tar \
tcl \
tdb \
tealdeer \
tevent \
texinfo \
thin-provisioning-tools \
tinysparql \
tpm2-tss \
tslib \
ttf-bitstream-vera \
ttf-dejavu \
ttf-fantasque-nerd \
ttf-fira-sans \
ttf-hack \
ttf-liberation \
ttf-meslo-nerd \
ttf-opensans \
twolame \
tzdata \
uchardet \
udisks2 \
ufw \
unrar \
unzip \
upower \
uriparser \
usb_modeswitch \
usbredir \
usbutils \
util-linux \
util-linux-libs \
v4l-utils \
vapoursynth \
vde2 \
verdict \
vi \
vid.stab \
vim \
vim-runtime \
virglrenderer \
virt-install \
virt-manager \
virtiofsd \
virtualbox-guest-utils \
vlc-plugin-a52dec \
vlc-plugin-alsa \
vlc-plugin-archive \
vlc-plugin-dav1d \
vlc-plugin-dbus \
vlc-plugin-dbus-screensaver \
vlc-plugin-faad2 \
vlc-plugin-flac \
vlc-plugin-gnutls \
vlc-plugin-inflate \
vlc-plugin-journal \
vlc-plugin-jpeg \
vlc-plugin-mpg123 \
vlc-plugin-ogg \
vlc-plugin-opus \
vlc-plugin-png \
vlc-plugin-shout \
vlc-plugin-speex \
vlc-plugin-tag \
vlc-plugin-theora \
vlc-plugin-twolame \
vlc-plugin-vorbis \
vlc-plugin-vpx \
vlc-plugin-xml \
vlc-plugins-base \
vmaf \
volume_key \
vte-common \
vte3 \
vulkan-icd-loader \
vulkan-mesa-device-select \
vulkan-tools \
vulkan-virtio \
wavpack \
waydroid \
wayland \
wayland-utils \
webkit2gtk-4.1 \
webrtc-audio-processing-1 \
wget \
which \
wildmidi \
wireless-regdb \
wireplumber \
woff2 \
wolfssl \
wpa_supplicant \
x264 \
x265 \
xcb-proto \
xcb-util \
xcb-util-cursor \
xcb-util-image \
xcb-util-keysyms \
xcb-util-renderutil \
xcb-util-wm \
xdg-dbus-proxy \
xdg-desktop-portal \
xdg-desktop-portal-gtk \
xdg-desktop-portal-kde \
xdg-user-dirs \
xdg-utils \
xf86-input-elographics \
xf86-input-evdev \
xf86-input-libinput \
xf86-input-synaptics \
xf86-input-vmmouse \
xf86-input-void \
xf86-input-wacom \
xf86-video-amdgpu \
xf86-video-ati \
xf86-video-dummy \
xf86-video-fbdev \
xf86-video-intel \
xf86-video-nouveau \
xf86-video-sisusb \
xf86-video-vesa \
xf86-video-vmware \
xf86-video-voodoo \
xfsdump \
xfsprogs \
xkeyboard-config \
xl2tpd \
xmlsec \
xorg-fonts-encodings \
xorg-server \
xorg-server-common \
xorg-setxkbmap \
xorg-xauth \
xorg-xdpyinfo \
xorg-xinit \
xorg-xinput \
xorg-xkbcomp \
xorg-xkill \
xorg-xmessage \
xorg-xmodmap \
xorg-xprop \
xorg-xrandr \
xorg-xrdb \
xorg-xset \
xorg-xwayland \
xorgproto \
xsettingsd \
xvidcore \
xxhash \
xz \
yaml-cpp \
yyjson \
zbar \
zeromq \
zimg \
zix \
zlib-ng \
zlib-ng-compat \
zram-generator \
zsh \
zsh-autosuggestions \
zsh-completions \
zsh-history-substring-search \
zsh-syntax-highlighting \
zsh-theme-powerlevel10k \
zstd \
zvbi \
zxing-cpp

print_status "Package installation completed"

# Step 3: System Configuration
print_section "Step 3: System Configuration"
echo 'blacklist ntfs3' | sudo tee /etc/modprobe.d/disable-ntfs3.conf >/dev/null 2>&1
sudo chmod 4755 /usr/lib/spice-client-glib-usb-acl-helper
sudo pacman -R firefox


print_info "Configuring GRUB bootloader..."
sudo cp -r /home/$USER/vanillaarch-or-cachyos-to-claudemods-apex-ckge/minimal/grub /etc/default
print_status "GRUB configuration copied"

print_info "Generating new GRUB configuration..."
sudo cp -r /home/$USER/vanillaarch-or-cachyos-to-claudemods-apex-ckge/minimal/10_linux /etc/grub.d
sudo cp -r /home/$USER/vanillaarch-or-cachyos-to-claudemods-apex-ckge/minimal/grub.cfg /boot/grub
sudo cp -r /home/$USER/vanillaarch-or-cachyos-to-claudemods-apex-ckge/minimal/cachyos /usr/share/grub/themes
sudo grub-mkconfig -o /boot/grub/grub.cfg
print_status "GRUB configuration updated"

print_info "Setting Plymouth boot animation..."
sudo cp -r /home/$USER/vanillaarch-or-cachyos-to-claudemods-apex-ckge/minimal/term.sh /usr/local/bin
sudo chmod +x /usr/local/bin/term.sh
sudo cp -r /home/$USER/vanillaarch-or-cachyos-to-claudemods-apex-ckge/minimal/term.service /etc/systemd/system/
sudo systemctl enable term.service >/dev/null 2>&1
sudo plymouth-set-default-theme -R cachyos-bootanimation
print_status "Plymouth theme configured"

print_info "Configuring Fish shell..."
mkdir /home/$USER/.config/fish
cp -r /home/$USER/vanillaarch-or-cachyos-to-claudemods-apex-ckge/minimal/config.fish /home/$USER/.config/fish/config.fish
cp -r /home/$USER/vanillaarch-or-cachyos-to-claudemods-apex-ckge/minimal/.zshrc /home/$USER/.zshrc
sudo chmod +X /home/$USER/.config/fish/config.fish
chsh -s $(which fish)
print_status "Fish configuration applied"

print_info "Apply Cachyos Kde Theme..."
cd /home/$USER && wget --show-progress --no-check-certificate 'https://drive.usercontent.google.com/download?id=1Bt7EhwB2qXTBEW2xhYrfUhPEriA5_uQ1&export=download&authuser=0&confirm=t&uuid=c02fc7dc-cdf0-4c44-b717-d431d3266bc1&at=AKSUxGOhRSb5QgLKDMIGL5_258gK:1761418156799'
cd /home/$USER && mv download* /home/$USER/appimages.zip >/dev/null 2>&1
cd /home/$USER && unzip appimages.zip -d /home/$USER/apps
mkdir /home/$USER/.local/bin
mkdir /home/$USER/.local/share/plasma
sudo mkdir /etc/sddm.conf.d
cd /home/$USER/apps && sudo unzip symlinks.zip -d /home/$USER/.local/bin && sudo unzip bauh.zip -d /home/$USER/.local/share/ && sudo unzip Arch-Systemtool.zip -d /opt && sudo unzip applications -d /home/$USER/.local/share/ >/dev/null 2>&1
cd /home/$USER/vanillaarch-or-cachyos-to-claudemods-apex-ckge/minimal && unzip kio.zip -d /home/$USER/.local/share
cd /home/$USER/vanillaarch-or-cachyos-to-claudemods-apex-ckge/minimal && unzip color-schemes.zip -d /home/$USER/.local/share
sudo chown $USER /home/$USER/.local/share/plasma
sudo chown $USER /home/$USER/.local/share/color-schemes
sudo chown $USER /home/$USER/.local/share/kio
cd /home/$USER/vanillaarch-or-cachyos-to-claudemods-apex-ckge/minimal && sudo unzip ApexLogin2.zip -d /usr/share/sddm/themes
sudo cp -r /home/$USER/vanillaarch-or-cachyos-to-claudemods-apex-ckge/minimal/kde_settings.conf /etc/sddm.conf.d
unzip /home/$USER/vanillaarch-or-cachyos-to-claudemods-apex-ckge/minimal/autostart.zip -d /home/$USER/.config
sudo chmod +x /home/$USER/.local/.config/autostart
cd /home/$USER/vanillaarch-or-cachyos-to-claudemods-apex-ckge/minimal && unzip apex-theme.zip -d /home/$USER/vanillaarch-or-cachyos-to-claudemods-apex-ckge/minimal
plasma-apply-colorscheme Apex > /dev/null 2>&1
mkdir /home/$USER/.icons
unzip Windows10Dark.zip -d /home/$USER/.icons > /dev/null 2>&1
cp -r /home/$USER/vanillaarch-or-cachyos-to-claudemods-apex-ckge/minimal/aurorae /home/$USER/.local/share
print_info "Cachyos Hello Will Now Open Please Close To Continue..."
cachyos-hello > /dev/null 2>&1
print_status "Proceeding..."
sudo chmod +x /home/$USER/vanillaarch-or-cachyos-to-claudemods-apex-ckge/minimal/installapextheme.sh
sudo chmod +x /home/$USER/vanillaarch-or-cachyos-to-claudemods-apex-ckge/minimal/start.sh
cd /home/$USER/vanillaarch-or-cachyos-to-claudemods-apex-ckge/minimal && ./installapextheme.sh
print_info "Theme Applied..."

print_section "CachyOS Conversion Complete!!"
echo -e "${GREEN}${BOLD}"
echo "Conversion to CachyOS has been completed successfully!"
sudo rm -rf /home/$USER/vanillaarch-to-cachyos
echo "Please reboot your system to apply all changes."
echo -e "${NC}"
