#!/bin/bash
nohup /usr/bin/plasmashell > /dev/null 2>&1 &
sleep 1
sudo -S rm -rf /opt/claudemods-distribution-installer
