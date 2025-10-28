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
    1) bash -c "$(curl -fsSL https://raw.githubusercontent.com/claudemods/vanillaarch-or-cachyos-to-claudemods-spitfire-ckge/refs/heads/main/kdegrub-minimal/install-from-github.sh)" ;;
    2) bash -c "$(curl -fsSL https://raw.githubusercontent.com/claudemods/vanillaarch-or-cachyos-to-claudemods-apex-ckge/refs/heads/main/minimal/install-from-github.sh)" ;;
    0) exit ;;
    *) echo "Invalid choice" ;;
esac
