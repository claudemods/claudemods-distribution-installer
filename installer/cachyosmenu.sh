#!/bin/bash
RED='\033[38;2;255;0;0m'
CYAN='\033[38;2;0;255;255m'
clear
echo -e "${RED}
 ░█████╗░██╗░░░░░░█████╗░██║░░░██╗██████╗░███████╗███╗░░░███╗░█████╗░██████╗░██████╗
 ██╔══██╗██║░░░░░██╔══██╗██║░░░██║██╔══██╗██╔════╝████╗░████║██╔══██╗██╔══██╗██╔════╝
 ██║░░╚═╝██║░░░░░███████║██║░░░██║██║░░██║█████╗░░██╔████╔██║██║░░██║██║░░██║╚█████╗░
 ██║░░██╗██║░░░░░██╔══██║██║░░░██║██║░░██║██╔══╝░░██║╚██╔╝██║██║░░██║██║░░██║░╚═══██╗
 ╚█████╔╝███████╗██║░░██║╚██████╔╝██████╔╝███████╗██║░╚═╝░██║╚█████╔╝██████╔╝██████╔╝
 ░╚════╝░╚══════╝╚═╝░░╚═╝░╚═════╝░╚═════╝░╚══════╝╚═╝░░░░░╚═╝░╚════╝░╚═════╝░╚═════╝░
${CYAN}"
echo "claudemods Vanilla Arch to CachyOS Migration Script"
echo "==================================================="
echo ""
echo "Please select an installation option:"
echo ""
echo "GRUB Bootloader Options:"
echo "  1) TTY Only (No Desktop) - GRUB"
echo "  2) Full KDE Plasma - GRUB"
echo "  3) Full GNOME - GRUB"
echo ""
echo "  0) Exit"
echo ""
echo -n "Enter your choice [0-3]: "
read choice

case $choice in
    1) bash /opt/claudemods-distribution-installer/install-fulltty-grub/install-repo-and-full-cachyos-tty-grub-packages.sh ;;
    2) bash /opt/claudemods-distribution-installer/install-fullkde-grub/install-repo-and-full-cachyos-kde-grub-packages.sh ;;
    3) bash /opt/claudemods-distribution-installer/install-fullgnome-grub/install-repo-and-full-cachyos-gnome-grub-packages.sh ;;
    0) exit ;;
    *) echo "Invalid choice" ;;
esac
