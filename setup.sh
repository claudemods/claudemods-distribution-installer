#!/bin/bash
cd /home/$USER && git clone https://github.com/claudemods/claudemods-distribution-installer
sudo mkdir /opt/claudemods-distribution-installer
sudo cp -r /home/$USER/claudemods-distribution-installer/installer/* /opt/claudemods-distribution-installer
cd /opt/claudemods-distribution-installer && sudo g++ -o mainscript mainscript.cpp -std=c++23
sudo chmod +x /opt/claudemods-distribution-installer/*
sudo rm -rf /home/$USER/claudemods-distribution-installer
cd /opt/claudemods-distribution-installer && sudo ./mainscript
