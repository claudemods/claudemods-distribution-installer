#!/bin/bash
cp -r /home/$USER/vanillaarch-or-cachyos-to-claudemods-spitfire-ckge/kdegrub-minimal/main.xml /home/$USER/.local/share/plasma/plasmoids/deskhide/contents/config
nohup /usr/bin/plasmashell > /dev/null 2>&1 &
sleep 1
plasma-apply-wallpaperimage -f stretch /opt/Arch-Systemtool/systemtool-extras/SpitFire/spitfire.png > /dev/null 2>&1
plasma-apply-colorscheme SpitFire > /dev/null 2>&1
sudo rm -rf /home/$USER/vanillaarch-or-cachyos-to-claudemods-spitfire-ckge
sudo rm -rf /home/$USER/appimages.zip
sudo rm -rf /home/$USER/apps
