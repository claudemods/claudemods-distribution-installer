#!/bin/bash

# Check if username argument is provided
if [ $# -eq 0 ]; then
    echo "Error: No username provided"
    echo "Usage: $0 <username>"
    exit 1
fi

username="$1"
target_file="/home/$USER/.local/share/user-places.xbel"

# Check if the target file exists
if [ ! -f "$target_file" ]; then
    echo "Error: File $target_file does not exist"
    exit 1
fi

# Create a backup of the original file
backup_file="${target_file}.backup.$(date +%Y%m%d_%H%M%S)"
if ! cp "$target_file" "$backup_file"; then
    echo "Error: Failed to create backup file"
    exit 1
fi

echo "Backup created: $backup_file"

# Replace spitfire with the new username in the file
if sed -i "s/spitfire/${username}/g" "$target_file"; then
    echo "Successfully replaced 'spitfire' with '${username}' in $target_file"
else
    echo "Error: Failed to modify the file"
    # Restore from backup if the operation failed?
    exit 1
fi
