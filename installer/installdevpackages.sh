#!/bin/bash
sudo -S pacman -Rsc --noconfirm phonon-qt6
sudo -S pacman -Rsc --noconfirm breeze-gtk
sudo -S pacman -Rsc --noconfirm kcoreaddons
sudo -S pacman -Rsc --noconfirm kconfig
sudo -S pacman -Rsc --noconfirm karchive kconfig
sudo -S pacman -Rsc --noconfirm raptor
sudo -S pacman -Rsc --noconfirm poppler-glib
sudo -S pacman -Rsc --noconfirm harfbuzz-icu
sudo -S pacman -Rsc --noconfirm ark karchive plasma-desktop dolphin kate konsole attica knewstuff discover
sudo -S cp -r /opt/claudemods-distribution-installer/pacman.conf /mnt/etc
sudo -S pacman -Sy
sudo -S pacman -S --noconfirm kdedevpackages
