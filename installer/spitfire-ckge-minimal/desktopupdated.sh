#!/bin/bash
echo final step needed for wallpaper and clean up
plasma-apply-wallpaperimage -f stretch /home/$USER/.config/Arch-Systemtool/systemtool-extras/SpitFire/spitfire.png > /dev/null 2>&1
cd /home/$USER/claudemods-distribution-installer/installer && chmod +x cleanup.sh
rm -rf /home/$USER/.config/autostart/wallpaper.desktop
cd /home/$USER/claudemods-distribution-installer/installer && ./cleanup.sh
