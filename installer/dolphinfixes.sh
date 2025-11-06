#!/bin/bash
username="$1"
sed -i "s/cachyos/$username/g" "/home/$username/.local/share/user-places.xbel"
