#!/bin/bash
nohup /usr/bin/plasmashell > /dev/null 2>&1 &
sleep 1
sudo rm -rf /opt/claudemods-distribution-installer
