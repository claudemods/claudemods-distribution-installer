#!/bin/bash
username="$1"
sed -i "s/cachyos/$username/g" "/home/$USER/.local/share/user-places.xbel"
