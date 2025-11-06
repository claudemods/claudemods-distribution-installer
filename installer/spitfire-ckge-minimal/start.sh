#!/bin/bash
cp -r /home/$USER/claudemods-distribution-installer/installer/spitfire-ckge-minimal/main.xml /home/$USER/.local/share/plasma/plasmoids/deskhide/contents/config
nohup /usr/bin/plasmashell > /dev/null 2>&1 &
sleep 1
plasma-apply-wallpaperimage -f stretch /opt/Arch-Systemtool/systemtool-extras/SpitFire/spitfire.png > /dev/null 2>&1
cp -r /home/$USER/claudemods-distribution-installer/installer/spitfire-ckge-minimal/konsolerc /home/$USER/.config
