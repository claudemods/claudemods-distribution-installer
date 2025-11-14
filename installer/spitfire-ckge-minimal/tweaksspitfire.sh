#!/bin/bash

username="$1"
target_file="/home/$USER/.local/share/user-places.xbel"

sed -i "s/spitfire/${username}/g" "$target_file"

echo 'blacklist ntfs3' | sudo -S tee /etc/modprobe.d/disable-ntfs3.conf >/dev/null 2>&1
sudo -S chsh -s $(which fish)
sudo -S chown -R ${username}:${username} /home/${username} 
sudo -S chown -R ${username}:${username} /home/${username}/*
