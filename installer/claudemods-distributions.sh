#!/bin/bash
RED='\033[38;2;255;0;0m'
CYAN='\033[38;2;0;255;255m'
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
    1) bash /opt/claudemods-distribution-installer/spitfire-ckge-minimal/install-repo-and-full-cachyos-kde-grub-packages.sh ;;
    2) bash /opt/claudemods-distribution-installer/apex-ckge-minimal/install-repo-and-full-cachyos-kde-grub-packages.sh ;;
    0) exit ;;
    *) echo "Invalid choice" ;;
esac
