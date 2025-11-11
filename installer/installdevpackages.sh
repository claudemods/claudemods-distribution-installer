#!/bin/bash
sudo -S pacman -Rns --noconfirm plasma-desktop dolphin kate konsole attica knewstuff discover
sudo -S cp -r /opt/claudemods-distribution-installer/pacman.conf /mnt/etc
sudo -S pacman -Sy
sudo -S pacman -S --noconfirm kdedevpackages
