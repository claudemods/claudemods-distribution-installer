#!/bin/bash

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
CYAN='\033[0;36m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color
BOLD='\033[1m'

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

# Check if username is provided as argument
if [ $# -eq 1 ]; then
    TARGET_USER="$1"
    print_info "Switching to user: $TARGET_USER"

    # Check if target user exists
    if id "$TARGET_USER" &>/dev/null; then
        # Re-execute script as target user
        exec su "$TARGET_USER" -c "bash $0"
    else
        print_error "User $TARGET_USER does not exist"
        exit 1
    fi
fi

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
echo "           ClaudeMods Vanilla Arch Kde Grub to CachyOS Kde Grub v1.01 27-10-2025"
echo -e "${NC}"
echo "================================================================================"
echo ""

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
    sudo -S pacman -Rns --noconfirm virt-manager
    wait
    print_status "virt-manager removed successfully"
else
    print_info "virt-manager not installed, proceeding..."
fi

print_section "Starting CachyOS Conversion Process"

# Step 1: Download and setup CachyOS repositories
print_section "Step 1: Setting up CachyOS Repositories"
print_info "Downloading CachyOS repository package..."
sudo -S curl https://mirror.cachyos.org/cachyos-repo.tar.xz -o cachyos-repo.tar.xz
wait
print_status "Download completed"

print_info "Extracting repository files..."
sudo -S tar xvf cachyos-repo.tar.xz && cd cachyos-repo
wait
print_status "Extraction completed"

print_info "Running CachyOS repository setup..."
sudo -S ./cachyos-repo.sh
wait
print_status "Repository setup completed"

# Step 2: Massive package installation
print_section "Step 2: Installing Packages"
print_info "Starting installation (this will take a while)..."
print_info "Please be patient as this process may take 30-60 minutes..."

sudo -S /opt/claudemods-distribution-installer/install-fullkde-grub/packages.sh

wait
print_status "Package installation completed"
# Step 3: System Configuration
print_section "Step 3: System Configuration"

print_info "Configuring GRUB bootloader..."
sudo -S cp -r /opt/claudemods-distribution-installer/install-fullkde-grub/grub /etc/default
wait
print_status "GRUB configuration copied"

print_info "Generating new GRUB configuration..."
sudo -S grub-mkconfig -o /boot/grub/grub.cfg
wait
print_status "GRUB configuration updated"

print_info "Setting Plymouth boot animation..."
sudo -S plymouth-set-default-theme -R cachyos-bootanimation
wait
print_status "Plymouth theme configured"

print_info "Configuring Fish shell..."
mkdir /home/$USER/.config/fish
wait
cp -r /opt/claudemods-distribution-installer/install-fullkde-grub/config.fish /home/$USER/.config/fish/config.fish
wait
cp -r /opt/claudemods-distribution-installer/install-fullkde-grub/.zshrc /home/$USER/.zshrc
wait
sudo -S chmod +X /home/$USER/.config/fish/config.fish
wait
chsh -s $(which fish)
wait
print_status "Fish configuration applied"
print_info "Cachyos Hello Will Now Open Please Close To Continue..."
cachyos-hello > /dev/null 2>&1
wait
print_info "Apply Cachyos Kde Theme..."
sudo -S chmod +x /opt/claudemods-distribution-installer/install-fullkde-grub/./installcachyostheme.sh
wait
sudo -S chmod +x /opt/claudemods-distribution-installer/install-fullkde-grub/start.sh
wait
cd /opt/claudemods-distribution-installer/install-fullkde-grub && ./installcachyostheme.sh
wait
print_info "Theme Applied..."

print_section "CachyOS Conversion Complete!"
echo -e "${GREEN}${BOLD}"
echo "Conversion to CachyOS has been completed successfully!"
sudo -S rm -rf /opt/claudemods-distribution-installer
wait
echo "Please reboot your system to apply all changes."
echo -e "${NC}"
