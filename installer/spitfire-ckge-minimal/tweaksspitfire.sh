#!/bin/bash

username="$1"
target_file="/home/$USER/.local/share/user-places.xbel"

sed -i "s/spitfire/${username}/g" "$target_file"

echo 'blacklist ntfs3' | sudo tee /etc/modprobe.d/disable-ntfs3.conf >/dev/null 2>&1
chsh -s $(which fish)
