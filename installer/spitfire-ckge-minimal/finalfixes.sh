#!/bin/bash
echo last minute dolphin fixes please enter password

for username in $(ls /home); do
    target_file="/home/${username}/.local/share/user-places.xbel"
    [ -f "$target_file" ] && sed -i "s/spitfire/${username}/g" "$target_file"

    sudo rm -rf /opt/finalfixes.sh
    sudo rm -rf /home/$USER/.config/autostart/finalfixes.desktop
