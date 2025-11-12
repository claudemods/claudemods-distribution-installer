#!/bin/bash

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
CYAN='\033[0;36m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color
BOLD='\033[1m'

# Use provided username
TARGET_USER="$1"

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
echo "           claudemods Spitfire CKGE Minimal v1.03.1 06-11-2025"
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
echo "• Modifying bootloader configuration"
echo "• Changing system themes and configurations"
echo -e "${NC}"
echo ""

# Automatic execution - removed yes/no prompt
echo -e "${GREEN}${BOLD}Starting automated installation in 1 seconds...${NC}"
echo -e "${YELLOW}Press Ctrl+C to cancel now...${NC}"
sleep 1

print_info "Configuration started..."

print_info "Configuring GRUB bootloader..."
sudo -S cp -r /home/$TARGET_USER/claudemods-distribution-installer/installer/spitfire-ckge-minimal/grub /etc/default
print_status "GRUB configuration copied"

print_info "Generating new GRUB configuration..."
sudo -S cp -r /home/$TARGET_USER/claudemods-distribution-installer/installer/spitfire-ckge-minimal/10_linux /etc/grub.d
sudo -S cp -r /home/$TARGET_USER/claudemods-distribution-installer/installer/spitfire-ckge-minimal/grub.cfg /boot/grub
sudo -S cp -r /home/$TARGET_USER/claudemods-distribution-installer/installer/spitfire-ckge-minimal/cachyos /usr/share/grub/themes
sudo -S grub-mkconfig -o /boot/grub/grub.cfg
print_status "GRUB configuration updated"

print_info "Setting Plymouth boot animation..."
sudo -S cp -r /home/$TARGET_USER/claudemods-distribution-installer/installer/spitfire-ckge-minimal/cachyos-bootanimation /usr/share/plymouth/themes/
sudo -S cp -r /home/$TARGET_USER/claudemods-distribution-installer/installer/spitfire-ckge-minimal/term.sh /usr/local/bin
sudo -S chmod +x /usr/local/bin/term.sh
sudo -S cp -r /home/$TARGET_USER/claudemods-distribution-installer/installer/spitfire-ckge-minimal/term.service /etc/systemd/system/
sudo -S systemctl enable term.service >/dev/null 2>&1
sudo -S plymouth-set-default-theme -R cachyos-bootanimation
print_status "Plymouth theme configured"

print_info "Configuring Fish color scheme..."
cp -r /home/$TARGET_USER/claudemods-distribution-installer/installer/spitfire-ckge-minimal/claudemods-cyan.colorscheme /home/$TARGET_USER/.local/share/konsole
cp -r /home/$TARGET_USER/claudemods-distribution-installer/installer/spitfire-ckge-minimal/claudemods-cyan.profile /home/$TARGET_USER/.local/share/konsole
cp -r /home/$TARGET_USER/claudemods-distribution-installer/installer/spitfire-ckge-minimal/konsolerc /home/$TARGET_USER/.config
chsh -s $(which fish)
sudo chmod +X /home/$USER/.config/fish/config.fish
print_status "Fish configuration applied"

print_info "Apply Cachyos Kde Theme..."
sudo chown $TARGET_USER:$TARGET_USER /home
sudo chown $TARGET_USER:$TARGET_USER /home/$TARGET_USER
sed -i '/^\[Desktop Entry\]/,/^\[/ s/^DefaultProfile=.*/DefaultProfile=claudemods-cyan.profile/' ~/.config/konsolerc
cp -r /home/$TARGET_USER/claudemods-distribution-installer/installer/spitfire-ckge-minimal/wallpaperupdated.desktop /home/$TARGET_USER/.config/autostart
sudo -S chmod +x /home/$TARGET_USER/.config/autostart/wallpaper.desktop
sudo -S chown $TARGET_USER /home/$TARGET_USER/.config/autostart/wallpaper.desktop
cd /home/$TARGET_USER/claudemods-distribution-installer/installer/spitfire-ckge-minimal && sudo -S unzip -uo SpitFireLogin.zip -d /usr/share/sddm/themes
sudo -S cp -r /home/$TARGET_USER/claudemods-distribution-installer/installer/spitfire-ckge-minimal/kde_settings.conf /etc/sddm.conf.d
cd /home/$TARGET_USER/claudemods-distribution-installer/installer/spitfire-ckge-minimal && unzip -uo updated.zip -d /home/$TARGET_USER/claudemods-distribution-installer/installer/spitfire-ckge-minimal
unzip -uo Windows10Dark.zip -d /home/$TARGET_USER/.icons > /dev/null 2>&1
cp -r /home/$TARGET_USER/claudemods-distribution-installer/installer/spitfire-ckge-minimal/aurorae /home/$TARGET_USER/.local/share
print_status "Proceeding..."
sudo -S chmod +x /home/$TARGET_USER/claudemods-distribution-installer/installer/spitfire-ckge-minimal/installspitfiretheme.sh
cd /home/$TARGET_USER/claudemods-distribution-installer/installer/spitfire-ckge-minimal && ./installspitfiretheme.sh $TARGET_USER

print_info "Theme Applied..."
