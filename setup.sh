#!/bin/bash
sudo pacman -Sy
sudo pacman -S git unzip
cd /home/$USER && git clone https://github.com/claudemods/claudemods-distribution-installer
sudo mkdir /opt/claudemods-distribution-installer
sudo cp -r /home/$USER/claudemods-distribution-installer/installer/* /opt/claudemods-distribution-installer
sudo chmod +x /opt/claudemods-distribution-installer/*
sudo rm -rf /home/$USER/claudemods-distribution-installer
./opt/claudemods-distribution-installer/mainscript.sh
