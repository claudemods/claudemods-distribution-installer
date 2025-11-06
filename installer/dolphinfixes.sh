#!/bin/bash
username="$1"
sed -i "s/cachyos/$username/g" "/home/cachyos/.local/share/user-places.xbel"
