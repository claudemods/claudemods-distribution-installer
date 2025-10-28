#!/bin/bash

RED='\033[38;2;255;0;0m'
CYAN='\033[38;2;0;255;255m'
NC='\033[0m'

# Check if username is provided as argument
if [ $# -eq 0 ]; then
    echo "Usage: $0 <username>"
    echo "Please provide a username as argument"
    exit 1
fi

TARGET_USER="$1"

# Check if target user exists
if ! id "$TARGET_USER" &>/dev/null; then
    echo "Error: User $TARGET_USER does not exist"
    exit 1
fi

clear
echo -e "${RED}
 ░█████╗░██╗░░░░░░█████╗░██║░░░██╗██████╗░███████╗███╗░░░███╗░█████╗░██████╗░░██████╗
 ██╔══██╗██║░░░░░██╔══██╗██║░░░██║██╔══██╗██╔════╝████╗░████║██╔══██╗██╔══██╗██╔════╝
 ██║░░╚═╝██║░░░░░███████║██║░░░██║██║░░██║█████╗░░██╔████╔██║██║░░██║██║░░██║╚█████╗░
 ██║░░██╗██║░░░░░██╔══██║██║░░░██║██║░░██║██╔══╝░░██║╚██╔╝██║██║░░██║██║░░██║░╚═══██╗
 ╚█████╔╝███████╗██║░░██║╚██████╔╝██████╔╝███████╗██║░╚═╝░██║╚█████╔╝██████╔╝██████╔╝
 ░╚════╝░╚══════╝╚═╝░░╚═╝░╚═════╝░╚═════╝░╚══════╝╚═╝░░░░░╚═╝░╚════╝░╚═════╝░╚═════╝░
${CYAN}"
echo "claudemods Distributions Installer v1.0 28-10-2025"
echo "==================================================="
echo "Installing for user: $TARGET_USER"
echo ""
echo "Please select an installation option:"
echo ""
echo "  1) Spitfire CKGE"
echo "  2) Apex CKGE"
echo ""
echo "  0) Exit"
echo ""
echo -n "Enter your choice [0-2]: "
read choice

case $choice in
    1) bash /opt/claudemods-distribution-installer/spitfire-ckge-minimal/install-repo-and-full-cachyos-kde-grub-packages.sh "$TARGET_USER" ;;
    2) bash /opt/claudemods-distribution-installer/apex-ckge-minimal/install-repo-and-full-cachyos-kde-grub-packages.sh "$TARGET_USER" ;;
    0) exit ;;
    *) echo "Invalid choice" ;;
esac
