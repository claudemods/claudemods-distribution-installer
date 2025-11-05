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
echo "           claudemods Vanilla Arch Kde Grub to Spitfire CKGE Minimal v1.01 04-11-2025"
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


print_info "Configuring GRUB bootloader..."
sudo cp -r /home/$USER/vanillaarch-or-cachyos-to-claudemods-spitfire-ckge/kdegrub-minimal/grub /etc/default
print_status "GRUB configuration copied"

print_info "Generating new GRUB configuration..."
sudo cp -r /home/$USER/vanillaarch-or-cachyos-to-claudemods-spitfire-ckge/kdegrub-minimal/10_linux /etc/grub.d
sudo cp -r /home/$USER/vanillaarch-or-cachyos-to-claudemods-spitfire-ckge/kdegrub-minimal/grub.cfg /boot/grub
sudo cp -r /home/$USER/vanillaarch-or-cachyos-to-claudemods-spitfire-ckge/kdegrub-minimal/cachyos /usr/share/grub/themes
sudo grub-mkconfig -o /boot/grub/grub.cfg
print_status "GRUB configuration updated"

print_info "Setting Plymouth boot animation..."
sudo cp -r /home/$USER/vanillaarch-or-cachyos-to-claudemods-spitfire-ckge/kdegrub-minimal/cachyos-bootanimation /usr/share/plymouth/themes/
sudo cp -r /home/$USER/vanillaarch-or-cachyos-to-claudemods-spitfire-ckge/kdegrub-minimal/term.sh /usr/local/bin
sudo chmod +x /usr/local/bin/term.sh
sudo cp -r /home/$USER/vanillaarch-or-cachyos-to-claudemods-spitfire-ckge/kdegrub-minimal/term.service /etc/systemd/system/
sudo systemctl enable term.service >/dev/null 2>&1
sudo plymouth-set-default-theme -R cachyos-bootanimation
print_status "Plymouth theme configured"

print_info "Configuring Fish shell..."
cp -r /home/$USER/vanillaarch-or-cachyos-to-claudemods-spitfire-ckge/kdegrub-minimal/claudemods-cyan.colorscheme /home/$USER/.local/share/konsole
cp -r /home/$USER/vanillaarch-or-cachyos-to-claudemods-spitfire-ckge/kdegrub-minimal/claudemods-cyan.profile /home/$USER/.local/share/konsole
cp -r /home/$USER/vanillaarch-or-cachyos-to-claudemods-spitfire-ckge/kdegrub-minimal/konsolerc /home/$USER/.config
mkdir /home/$USER/.config/fish
cp -r /home/$USER/vanillaarch-or-cachyos-to-claudemods-spitfire-ckge/kdegrub-minimal/config.fish /home/$USER/.config/fish/config.fish
cp -r /home/$USER/vanillaarch-or-cachyos-to-claudemods-spitfire-ckge/kdegrub-minimal/.zshrc /home/$USER/.zshrc
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
cd /home/$USER/vanillaarch-or-cachyos-to-claudemods-spitfire-ckge/kdegrub-minimal && unzip kio.zip -d /home/$USER/.local/share
cd /home/$USER/vanillaarch-or-cachyos-to-claudemods-spitfire-ckge/kdegrub-minimal && unzip color-schemes.zip -d /home/$USER/.local/share
sudo chown $USER /home/$USER/.local/share/plasma
sudo chown $USER /home/$USER/.local/share/color-schemes
sudo chown $USER /home/$USER/.local/share/kio
cd /home/$USER/vanillaarch-or-cachyos-to-claudemods-spitfire-ckge/kdegrub-minimal && sudo unzip SpitFireLogin.zip -d /usr/share/sddm/themes
sudo cp -r /home/$USER/vanillaarch-or-cachyos-to-claudemods-spitfire-ckge/kdegrub-minimal/kde_settings.conf /etc/sddm.conf.d
unzip /home/$USER/vanillaarch-or-cachyos-to-claudemods-spitfire-ckge/kdegrub-minimal/autostart.zip -d /home/$USER/.config
sudo chmod +x /home/$USER/.local/.config/autostart
cd /home/$USER/vanillaarch-or-cachyos-to-claudemods-spitfire-ckge/kdegrub-minimal && unzip theme.zip -d /home/$USER/vanillaarch-or-cachyos-to-claudemods-spitfire-ckge/kdegrub-minimal
plasma-apply-colorscheme SpitFire > /dev/null 2>&1
mkdir /home/$USER/.icons
unzip Windows10Dark.zip -d /home/$USER/.icons > /dev/null 2>&1
cp -r /home/$USER/vanillaarch-or-cachyos-to-claudemods-spitfire-ckge/kdegrub-minimal/aurorae /home/$USER/.local/share
print_info "Cachyos Hello Will Now Open Please Close To Continue..."
cachyos-hello > /dev/null 2>&1
print_status "Proceeding..."
sudo chmod +x /home/$USER/vanillaarch-or-cachyos-to-claudemods-spitfire-ckge/kdegrub-minimal/installspitfiretheme.sh
sudo chmod +x /home/$USER/vanillaarch-or-cachyos-to-claudemods-spitfire-ckge/kdegrub-minimal/start.sh
cd /home/$USER/vanillaarch-or-cachyos-to-claudemods-spitfire-ckge/kdegrub-minimal && ./installspitfiretheme.sh
print_info "Theme Applied..."

print_section "CachyOS Conversion Complete!!"
echo -e "${GREEN}${BOLD}"
echo "Conversion to CachyOS has been completed successfully!"
sudo rm -rf /home/$USER/vanillaarch-to-cachyos
echo "Please reboot your system to apply all changes."
echo -e "${NC}"
