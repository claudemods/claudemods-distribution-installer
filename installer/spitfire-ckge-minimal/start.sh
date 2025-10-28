#!/bin/bash
cp -r /opt/claudemods-distribution-installer/apex-ckge-minimal/main.xml /home/$USER/.local/share/plasma/plasmoids/deskhide/contents/config
nohup /usr/bin/plasmashell > /dev/null 2>&1 &
sleep 1
plasma-apply-wallpaperimage -f stretch /opt/Arch-Systemtool/systemtool-extras/SpitFire/spitfire.png > /dev/null 2>&1
plasma-apply-colorscheme SpitFire > /dev/null 2>&1
sudo -S rm -rf /opt/claudemods-distribution-installer
sudo -S rm -rf /home/$USER/appimages.zip
sudo -S rm -rf /home/$USER/apps
