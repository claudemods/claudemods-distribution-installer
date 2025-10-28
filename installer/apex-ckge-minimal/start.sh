#!/bin/bash
cp -r /home/$USER/vanillaarch-or-cachyos-to-claudemods-apex-ckge/minimal/main.xml /home/$USER/.local/share/plasma/plasmoids/deskhide/contents/config
nohup /usr/bin/plasmashell > /dev/null 2>&1 &
sleep 1
plasma-apply-wallpaperimage -f stretch /opt/Arch-Systemtool/systemtool-extras/Apex/rift.jpg > /dev/null 2>&1
plasma-apply-colorscheme Apex > /dev/null 2>&1
sudo rm -rf /home/$USER/vanillaarch-or-cachyos-to-claudemods-apex-ckge
sudo rm -rf /home/$USER/appimages.zip
sudo rm -rf /home/$USER/apps
