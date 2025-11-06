#!/bin/bash

# Set output colors
CYAN='\033[0;36m'
YELLOW='\033[1;33m'
BOLD='\033[1m'
NC='\033[0m' # No Color

SCRIPT_DIR=$(pwd)
CONFIG_PATH="${XDG_CONFIG_HOME:-$HOME/.config}"
DATA_PATH="${XDG_DATA_HOME:-$HOME/.local/share}"
CONFIG_FOLDER="$SCRIPT_DIR/spitfire-theme"

LOG_TEXT=""

update_status() {
    echo -e "${CYAN}> $1${NC}"
    LOG_TEXT+="> $1\n"
}

print_info() {
    echo -e "${CYAN}$1${NC}"
}

print_warning() {
    echo -e "${YELLOW}$1${NC}"
}

copy_directory_recursive() {
    local fromDir="$1"
    local toDir="$2"
    local overwriteExisting="$3"

    if [ ! -d "$fromDir" ]; then
        LOG_TEXT+="✗ Source directory doesn't exist: $fromDir\n"
        return 1
    fi

    if [ ! -d "$toDir" ]; then
        if ! mkdir -p "$toDir"; then
            LOG_TEXT+="✗ Failed to create directory: $toDir\n"
            return 1
        fi
    fi

    for item in "$fromDir"/*; do
        if [[ "$item" == "$fromDir/*" ]]; then
            continue
        fi

        local filename=$(basename "$item")
        if [ "$filename" == "." ] || [ "$filename" == ".." ]; then
            continue
        fi

        local srcFilePath="$item"
        local dstFilePath="$toDir/$filename"

        if [ -d "$srcFilePath" ]; then
            if ! copy_directory_recursive "$srcFilePath" "$dstFilePath" "$overwriteExisting"; then
                return 1
            fi
        else
            if [ "$overwriteExisting" == "false" ] && [ -f "$dstFilePath" ]; then
                continue
            fi

            if [ -f "$dstFilePath" ]; then
                rm -f "$dstFilePath"
            fi

            if ! cp "$srcFilePath" "$dstFilePath"; then
                LOG_TEXT+="✗ Failed to copy file: $filename\n"
                return 1
            else
                LOG_TEXT+="✓ Copied file: $filename\n"
            fi
        fi
    done
    return 0
}

handle_directory() {
    local src_dir="$1"
    local dest_dir="$2"

    if [ ! -d "$src_dir" ]; then
        LOG_TEXT+="✗ Source directory doesn't exist: $src_dir\n"
        return
    fi

    LOG_TEXT+="Copying directory: $src_dir to $dest_dir\n"

    if ! copy_directory_recursive "$src_dir" "$dest_dir" "true"; then
        LOG_TEXT+="✗ Error copying directory: $src_dir to $dest_dir\n"
    else
        LOG_TEXT+="✓ Successfully copied directory: $src_dir\n"
    fi
}

handle_file() {
    local src_file="$1"
    local dest_file="$2"

    if [ ! -f "$src_file" ]; then
        LOG_TEXT+="✗ Source file doesn't exist: $src_file\n"
        return
    fi

    local dest_dir=$(dirname "$dest_file")
    if [ ! -d "$dest_dir" ]; then
        if ! mkdir -p "$dest_dir"; then
            LOG_TEXT+="✗ Failed to create directory: $dest_dir\n"
            return
        fi
    fi

    if [ -f "$dest_file" ]; then
        LOG_TEXT+="Removing existing file: $dest_file\n"
        if ! rm -f "$dest_file"; then
            LOG_TEXT+="✗ Failed to remove file: $dest_file\n"
            return
        fi
    fi

    if cp "$src_file" "$dest_file"; then
        LOG_TEXT+="✓ Copied file: $src_file\n"
    else
        LOG_TEXT+="✗ Error copying file: $src_file\n"
    fi
}

install_backup() {
    local config_folder="$1"
    local config_path="$2"
    local data_path="$3"

    local dirs=("icons" "color-schemes" "plasma" "wallpapers" "kfontinst" "latte" "autostart" "Kvantum")
    LOG_TEXT+="Installing directories...\n"

    for dir_name in "${dirs[@]}"; do
        local srcDir="$config_folder/$dir_name"
        local destDir="$data_path/$dir_name"
        handle_directory "$srcDir" "$destDir"
    done

    local files=(
        "dolphinrc"
        "kactivitymanagerd-statsrc"
        "kcminputrc"
        "kde_settings.conf"
        "kdeglobals"
        "kglobalshortcutsrc"
        "konsolerc"
        "kscreenlockerrc"
        "ksmserverrc"
        "ksplashrc"
        "kwinrc"
        "plasma-org.kde.plasma.desktop-appletsrc"
        "plasma_workspace.notifyrc"
        "plasmarc"
        "plasmashellrc"
        "powerdevilrc"
    )

    LOG_TEXT+="Installing configuration files...\n"
    for file_name in "${files[@]}"; do
        local srcFile="$config_folder/$file_name"
        local destFile="$config_path/$file_name"
        handle_file "$srcFile" "$destFile"
    done

    LOG_TEXT+="Applying KDE configurations...\n"
    if [ -f "/home/$USER/claudemods-distribution-installer/installer/spitfire-ckge-minimal/start.sh" ]; then
        cd "/home/$USER/claudemods-distribution-installer/installer/spitfire-ckge-minimal" && ./start.sh > /dev/null 2>&1
        LOG_TEXT+="✓ KDE configuration applied successfully\n"
    else
        LOG_TEXT+="✗ KDE configuration script not found\n"
    fi
}

# MAIN EXECUTION START
update_status "Starting theme installation..."

if [ ! -d "$CONFIG_FOLDER" ]; then
    update_status "Error: Theme folder not found at $CONFIG_FOLDER"
    exit 1
fi

update_status "Installing from: $CONFIG_FOLDER"
update_status "Config path: $CONFIG_PATH"
update_status "Data path: $DATA_PATH"

install_backup "$CONFIG_FOLDER" "$CONFIG_PATH" "$DATA_PATH"

update_status "Installation complete!"
echo -e "\n${CYAN}Installation log:${NC}"
echo -e "$LOG_TEXT"
