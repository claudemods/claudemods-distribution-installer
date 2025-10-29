#!/bin/bash

# Color definitions
COLOR_CYAN='\033[38;2;0;255;255m'
COLOR_RED='\033[31m'
COLOR_GREEN='\033[32m'
COLOR_YELLOW='\033[33m'
COLOR_BLUE='\033[34m'
COLOR_MAGENTA='\033[35m'
COLOR_ORANGE='\033[38;5;208m'
COLOR_PURPLE='\033[38;5;93m'
COLOR_RESET='\033[0m'

# Function to execute commands with error handling
execute_command() {
    local cmd="$1"
    echo -e "${COLOR_CYAN}"
    local full_cmd="sudo $cmd"
    eval "$full_cmd"
    local status=$?
    echo -e "${COLOR_RESET}"
    if [ $status -ne 0 ]; then
        echo -e "${COLOR_RED}Error executing: $full_cmd${COLOR_RESET}" >&2
        exit 1
    fi
}

# Function to check if path is a block device
is_block_device() {
    local path="$1"
    if [ ! -b "$path" ]; then
        return 1
    fi
    return 0
}

# Function to check if directory exists
directory_exists() {
    local path="$1"
    if [ ! -d "$path" ]; then
        return 1
    fi
    return 0
}

# Function to get UK date time
get_uk_date_time() {
    date +"%d-%m-%Y_%I:%M%P"
}

# Function to display available drives
display_available_drives() {
    echo -e "${COLOR_YELLOW}"
    echo "╔══════════════════════════════════════════════════════════════╗"
    echo "║                    Available Drives                         ║"
    echo "╠══════════════════════════════════════════════════════════════╣"
    echo -e "${COLOR_RESET}"

    # Display block devices using lsblk with better formatting
    echo -e "${COLOR_CYAN}Block Devices:${COLOR_RESET}"
    lsblk -o NAME,SIZE,TYPE,MOUNTPOINT,FSTYPE,MODEL | grep -v "loop" | while read line; do
        echo -e "${COLOR_GREEN}  $line${COLOR_RESET}"
    done

    echo -e "${COLOR_YELLOW}"
    echo "╚══════════════════════════════════════════════════════════════╝"
    echo -e "${COLOR_RESET}"
    echo
}

# Function to display header
display_header() {
    echo -e "${COLOR_RED}"
    cat << "EOF"
░█████╗░██╗░░░░░░█████╗░██║░░░██╗██████╗░███████╗███╗░░░███╗░█████╗░██████╗░░██████╗
██╔══██╗██║░░░░░██╔══██╗██║░░░██║██╔══██╗██╔════╝████╗░████║██╔══██╗██╔══██╗██╔════╝
██║░░╚═╝██║░░░░░███████║██║░░░██║██║░░██║█████╗░░██╔████╔██║██║░░██║██║░░██║╚█████╗░
██║░░██╗██║░░░░░██╔══██║██║░░░██║██║░░██║██╔══╝░░██║╚██╔╝██║██║░░██║██║░░██║░╚═══██╗
╚█████╔╝███████╗██║░░██║╚██████╔╝██████╔╝███████╗██║░╚═╝░██║╚█████╔╝██████╔╝██████╔╝
░╚════╝░╚══════╝╚═╝░░░░░░╚═════╝░╚═════╝░╚══════╝╚═╝░░░░░╚═╝░╚════╝░╚═════╝░╚═════╝░
EOF
    echo -e "${COLOR_CYAN}claudemods distribution installer v1.0 28-10-2025${COLOR_RESET}"
    echo -e "${COLOR_CYAN}Supports Ext4 And Btrfs filesystems${COLOR_RESET}"
    echo
}

# Function to prepare target partitions
prepare_target_partitions() {
    local drive="$1"
    local fs_type="$2"

    execute_command "umount -f ${drive}* 2>/dev/null || true"
    execute_command "wipefs -a $drive"
    execute_command "parted -s $drive mklabel gpt"
    execute_command "parted -s $drive mkpart primary fat32 1MiB 551MiB"
    execute_command "parted -s $drive mkpart primary $fs_type 551MiB 100%"
    execute_command "parted -s $drive set 1 esp on"
    execute_command "partprobe $drive"
    sleep 2

    local efi_part="${drive}1"
    local root_part="${drive}2"

    if ! is_block_device "$efi_part" || ! is_block_device "$root_part"; then
        echo -e "${COLOR_RED}Error: Failed to create partitions${COLOR_RESET}" >&2
        exit 1
    fi

    execute_command "mkfs.vfat -F32 $efi_part"
    execute_command "mkfs.ext4 -F -L ROOT $root_part"
}

# Function to setup Ext4 filesystem
setup_ext4_filesystem() {
    local root_part="$1"
    execute_command "mount $root_part /mnt"
    execute_command "mkdir -p /mnt/{home,boot/efi,etc,usr,var,proc,sys,dev,tmp,run}"
}

# Function to copy system
copy_system() {
    local efi_part="$1"
    local rsync_cmd="sudo rsync -aHAXSr --numeric-ids --info=progress2 "
    rsync_cmd+="--exclude=/etc/udev/rules.d/70-persistent-cd.rules "
    rsync_cmd+="--exclude=/etc/udev/rules.d/70-persistent-net.rules "
    rsync_cmd+="--exclude=/etc/mtab "
    rsync_cmd+="--exclude=/etc/fstab "
    rsync_cmd+="--exclude=/dev/* "
    rsync_cmd+="--exclude=/proc/* "
    rsync_cmd+="--exclude=/sys/* "
    rsync_cmd+="--exclude=/tmp/* "
    rsync_cmd+="--exclude=/run/* "
    rsync_cmd+="--exclude=/mnt/* "
    rsync_cmd+="--exclude=/media/* "
    rsync_cmd+="--exclude=/lost+found "
    rsync_cmd+="--include=/dev "
    rsync_cmd+="--include=/proc "
    rsync_cmd+="--include=/tmp "
    rsync_cmd+="--include=/sys "
    rsync_cmd+="--include=/run "
    rsync_cmd+="--include=/usr "
    rsync_cmd+="--include=/etc "
    rsync_cmd+="/ /mnt"

    execute_command "$rsync_cmd"
    execute_command "mount $efi_part /mnt/boot/efi"
    execute_command "mkdir -p /mnt/{proc,sys,dev,run,tmp}"
}

# Function to install GRUB for Ext4
install_grub_ext4() {
    local drive="$1"
    execute_command "sudo tar -xzf mtab.tar.gz -C /mnt/etc"
    execute_command "mount --bind /dev /mnt/dev"
    execute_command "mount --bind /dev/pts /mnt/dev/pts"
    execute_command "mount --bind /proc /mnt/proc"
    execute_command "mount --bind /sys /mnt/sys"
    execute_command "mount --bind /run /mnt/run"
    execute_command "chroot /mnt /bin/bash -c \"mount -t efivarfs efivarfs /sys/firmware/efi/efivars \""
    execute_command "chroot /mnt /bin/bash -c \"genfstab -U /\""
    execute_command "chroot /mnt /bin/bash -c \"grub-install --target=x86_64-efi --efi-directory=/boot/efi --bootloader-id=GRUB --recheck\""
    execute_command "chroot /mnt /bin/bash -c \"grub-mkconfig -o /boot/grub/grub.cfg\""
    execute_command "chroot /mnt /bin/bash -c \"mkinitcpio -P\""
}

# Function to change username in the new system
change_username() {
    local fs_type="$1"
    local drive="$2"

    echo -e "${COLOR_CYAN}Enter new username: ${COLOR_RESET}"
    read -r new_username

    echo -e "${COLOR_CYAN}Mounting system for username change...${COLOR_RESET}"

    execute_command "mount ${drive}2 /mnt"
    execute_command "mount ${drive}1 /mnt/boot/efi"
    execute_command "mount --bind /dev /mnt/dev"
    execute_command "mount --bind /dev/pts /mnt/dev/pts"
    execute_command "mount --bind /proc /mnt/proc"
    execute_command "mount --bind /sys /mnt/sys"
    execute_command "mount --bind /run /mnt/run"

    echo -e "${COLOR_CYAN}Changing username from 'arch' to '$new_username'...${COLOR_RESET}"

    # Change username from arch to new username
    execute_command "chroot /mnt /bin/bash -c \"usermod -l $new_username arch\""
    
    # Change home directory name
    execute_command "chroot /mnt /bin/bash -c \"mv /home/arch /home/$new_username\""
    
    # Change home directory in passwd
    execute_command "chroot /mnt /bin/bash -c \"usermod -d /home/$new_username $new_username\""
    
    # Change group name
    execute_command "chroot /mnt /bin/bash -c \"groupmod -n $new_username arch\""

     echo -e "${COLOR_CYAN}Adding $new_username to sudo group...${COLOR_RESET}"
    execute_command "chroot /mnt /bin/bash -c \"gpasswd -a $new_username wheel\""

    # Set password for the new user
    echo -e "${COLOR_CYAN}Setting password for user '$new_username'...${COLOR_RESET}"
    execute_command "chroot /mnt /bin/bash -c \"passwd $new_username\""

    # Cleanup
    execute_command "umount -R /mnt"

    echo -e "${COLOR_GREEN}Username changed from 'arch' to '$new_username'${COLOR_RESET}"
}

# Function to install arch tty grub (complete installation)
install_arch_tty_grub() {
    local drive="$1"
    local fs_type="ext4"

    echo -e "${COLOR_CYAN}Starting Arch TTY Grub installation...${COLOR_RESET}"
    
    # Prepare partitions
    echo -e "${COLOR_CYAN}Preparing partitions...${COLOR_RESET}"
    prepare_target_partitions "$drive" "$fs_type"
    
    local efi_part="${drive}1"
    local root_part="${drive}2"
    
    # Setup filesystem
    echo -e "${COLOR_CYAN}Setting up filesystem...${COLOR_RESET}"
    setup_ext4_filesystem "$root_part"
    
    # Copy system
    echo -e "${COLOR_CYAN}Copying system...${COLOR_RESET}"
    copy_system "$efi_part"
    
    # Install GRUB
    echo -e "${COLOR_CYAN}Installing GRUB...${COLOR_RESET}"
    install_grub_ext4 "$drive"
    
    # Change username
    echo -e "${COLOR_CYAN}Setting up user account...${COLOR_RESET}"
    change_username "$fs_type" "$drive"
    
    echo -e "${COLOR_GREEN}Arch TTY Grub installation completed successfully!${COLOR_RESET}"
}

# Function to install desktop environments
install_desktop() {
    local fs_type="$1"
    local drive="$2"

    # Display desktop options - Top 10 Arch package list
    echo -e "${COLOR_CYAN}"
    echo "╔══════════════════════════════════════════════════════════════╗"
    echo "║                   Desktop Environments                       ║"
    echo "╠══════════════════════════════════════════════════════════════╣"
    echo "║  1. Arch TTY Grub (Complete Installation)                   ║"
    echo "║  2. GNOME                                                   ║"
    echo "║  3. KDE Plasma                                              ║"
    echo "║  4. XFCE                                                    ║"
    echo "║  5. LXQt                                                   ║"
    echo "║  6. Cinnamon                                                ║"
    echo "║  7. MATE                                                    ║"
    echo "║  8. Budgie                                                  ║"
    echo "║  9. i3 (tiling WM)                                          ║"
    echo "║ 10. Sway (Wayland tiling)                                   ║"
    echo "║ 11. Hyprland (Wayland)                                      ║"
    echo "║ 12. Return to Main Menu                                     ║"
    echo "╚══════════════════════════════════════════════════════════════╝"
    echo -e "${COLOR_RESET}"

    echo -e "${COLOR_CYAN}Select desktop environment (1-12): ${COLOR_RESET}"
    read -r desktop_choice

    case $desktop_choice in
        1)
            echo -e "${COLOR_CYAN}Starting Arch TTY Grub installation...${COLOR_RESET}"
            install_arch_tty_grub "$drive"
            ;;
        2)
            echo -e "${COLOR_CYAN}Installing GNOME Desktop...${COLOR_RESET}"
            install_arch_tty_grub() {
                local drive="$1"
                local fs_type="ext4"
                # Setup filesystem
                echo -e "${COLOR_CYAN}Setting up filesystem...${COLOR_RESET}"
                setup_ext4_filesystem "$root_part"
                # Install GRUB
                echo -e "${COLOR_CYAN}Installing Stuff...${COLOR_RESET}"
                execute_command "mount ${drive}2 /mnt"
                execute_command "mount ${drive}1 /mnt/boot/efi"
                execute_command "mount --bind /dev /mnt/dev"
                execute_command "mount --bind /dev/pts /mnt/dev/pts"
                execute_command "mount --bind /proc /mnt/proc"
                execute_command "mount --bind /sys /mnt/sys"
                execute_command "mount --bind /run /mnt/run"
                execute_command "sudo pacstrap /mnt base gnome gnome-extra gdm grub efibootmgr os-prober"
                install_grub_ext4 "$drive"
                # Change username
                echo -e "${COLOR_CYAN}Setting up user account...${COLOR_RESET}"
                change_username "$fs_type" "$drive"
                execute_command "chroot /mnt /bin/bash -c \"systemctl enable gdm\""
            }
            install_arch_tty_grub "$drive"
            ;;
        3)
            echo -e "${COLOR_CYAN}Installing KDE Plasma...${COLOR_RESET}"
            execute_command "chroot /mnt /bin/bash -c \"pacman -S --noconfirm plasma-desktop sddm dolphin konsole\""
            execute_command "chroot /mnt /bin/bash -c \"systemctl enable sddm\""
            ;;
        4)
            echo -e "${COLOR_CYAN}Installing XFCE...${COLOR_RESET}"
            execute_command "chroot /mnt /bin/bash -c \"pacman -S --noconfirm xfce4 xfce4-goodies lightdm lightdm-gtk-greeter\""
            execute_command "chroot /mnt /bin/bash -c \"systemctl enable lightdm\""
            ;;
        5)
            echo -e "${COLOR_CYAN}Installing LXQt...${COLOR_RESET}"
            execute_command "chroot /mnt /bin/bash -c \"pacman -S --noconfirm lxqt sddm\""
            execute_command "chroot /mnt /bin/bash -c \"systemctl enable sddm\""
            ;;
        6)
            echo -e "${COLOR_CYAN}Installing Cinnamon...${COLOR_RESET}"
            execute_command "chroot /mnt /bin/bash -c \"pacman -S --noconfirm cinnamon lightdm lightdm-gtk-greeter\""
            execute_command "chroot /mnt /bin/bash -c \"systemctl enable lightdm\""
            ;;
        7)
            echo -e "${COLOR_CYAN}Installing MATE...${COLOR_RESET}"
            execute_command "chroot /mnt /bin/bash -c \"pacman -S --noconfirm mate mate-extra lightdm lightdm-gtk-greeter\""
            execute_command "chroot /mnt /bin/bash -c \"systemctl enable lightdm\""
            ;;
        8)
            echo -e "${COLOR_CYAN}Installing Budgie...${COLOR_RESET}"
            execute_command "chroot /mnt /bin/bash -c \"pacman -S --noconfirm budgie-desktop lightdm lightdm-gtk-greeter\""
            execute_command "chroot /mnt /bin/bash -c \"systemctl enable lightdm\""
            ;;
        9)
            echo -e "${COLOR_CYAN}Installing i3 (tiling WM)...${COLOR_RESET}"
            execute_command "chroot /mnt /bin/bash -c \"pacman -S --noconfirm i3-wm i3status i3lock dmenu lightdm lightdm-gtk-greeter\""
            execute_command "chroot /mnt /bin/bash -c \"systemctl enable lightdm\""
            ;;
        10)
            echo -e "${COLOR_CYAN}Installing Sway (Wayland tiling)...${COLOR_RESET}"
            execute_command "chroot /mnt /bin/bash -c \"pacman -S --noconfirm sway swaybg waybar wofi lightdm lightdm-gtk-greeter\""
            execute_command "chroot /mnt /bin/bash -c \"systemctl enable lightdm\""
            ;;
        11)
            echo -e "${COLOR_PURPLE}Installing Hyprland (Modern Wayland Compositor)...${COLOR_RESET}"
            execute_command "chroot /mnt /bin/bash -c \"pacman -S --noconfirm hyprland waybar rofi wl-clipboard sddm\""
            execute_command "chroot /mnt /bin/bash -c \"systemctl enable sddm\""
            echo -e "${COLOR_PURPLE}Hyprland installed! Note: You may need to configure ~/.config/hypr/hyprland.conf${COLOR_RESET}"
            ;;
        12)
            echo -e "${COLOR_CYAN}Returning to main menu...${COLOR_RESET}"
            ;;
        *)
            echo -e "${COLOR_RED}Invalid option. Returning to main menu.${COLOR_RESET}"
            ;;
    esac

    # Cleanup
    echo -e "${COLOR_CYAN}Cleaning up...${COLOR_RESET}"
    execute_command "umount -R /mnt"
}

# Function to install Cachyos options
install_cachyos_options() {
    local fs_type="$1"
    local drive="$2"

    echo -e "${COLOR_CYAN}Mounting system for Cachyos installation...${COLOR_RESET}"

    execute_command "mount ${drive}2 /mnt"
    execute_command "mount ${drive}1 /mnt/boot/efi"
    execute_command "mount --bind /dev /mnt/dev"
    execute_command "mount --bind /dev/pts /mnt/dev/pts"
    execute_command "mount --bind /proc /mnt/proc"
    execute_command "mount --bind /sys /mnt/sys"
    execute_command "mount --bind /run /mnt/run"
    # Step 1: Download and setup CachyOS repositories
    execute_command "sudo curl https://mirror.cachyos.org/cachyos-repo.tar.xz -o cachyos-repo.tar.xz"
    execute_command "sudo tar xvf cachyos-repo.tar.xz && cd cachyos-repo"
    execute_command "sudo ./cachyos-repo.sh"


    # Check if cachyosmenu.sh exists in current directory
    if [ -f "cachyosmenu.sh" ]; then
        echo -e "${COLOR_GREEN}Copying cachyosmenu.sh to chroot...${COLOR_RESET}"
        execute_command "cp cachyosmenu.sh /mnt"
        execute_command "chmod +x /mnt/cachyosmenu.sh"
        
        echo -e "${COLOR_GREEN}Executing cachyosmenu.sh with username $new_username...${COLOR_RESET}"
        execute_command "chroot /mnt /bin/bash -c \"/cachyosmenu.sh $new_username\""
        echo -e "${COLOR_GREEN}Cachyos installation completed!${COLOR_RESET}"
    else
        echo -e "${COLOR_RED}Error: cachyosmenu.sh not found in current directory${COLOR_RESET}"
        echo -e "${COLOR_YELLOW}Please ensure cachyosmenu.sh is in the same directory as this script${COLOR_RESET}"
    fi

    # Cleanup
    echo -e "${COLOR_CYAN}Cleaning up...${COLOR_RESET}"
    execute_command "umount -R /mnt"
}

# Function to install claudemods distribution options
install_claudemods_distribution() {
    local fs_type="$1"
    local drive="$2"

    echo -e "${COLOR_CYAN}Mounting system for claudemods distribution installation...${COLOR_RESET}"

    execute_command "mount ${drive}2 /mnt"
    execute_command "mount ${drive}1 /mnt/boot/efi"
    execute_command "mount --bind /dev /mnt/dev"
    execute_command "mount --bind /dev/pts /mnt/dev/pts"
    execute_command "mount --bind /proc /mnt/proc"
    execute_command "mount --bind /sys /mnt/sys"
    execute_command "mount --bind /run /mnt/run"

    # Check if claudemods-distributions.sh exists in current directory
    if [ -f "claudemods-distributions.sh" ]; then
        echo -e "${COLOR_GREEN}Copying claudemods-distributions.sh to chroot...${COLOR_RESET}"
        execute_command "cp claudemods-distributions.sh /mnt"
        execute_command "chmod +x /mnt/claudemods-distributions.sh"
        echo -e "${COLOR_GREEN}Executing claudemods-distributions.sh with username $new_username...${COLOR_RESET}"
        execute_command "chroot /mnt /bin/bash -c \"/claudemods-distributions.sh $new_username\""
        echo -e "${COLOR_GREEN}Claudemods distribution installation completed!${COLOR_RESET}"
    else
        echo -e "${COLOR_RED}Error: claudemods-distributions.sh not found in current directory${COLOR_RESET}"
        echo -e "${COLOR_YELLOW}Please ensure claudemods-distributions.sh is in the same directory as this script${COLOR_RESET}"
    fi

    # Cleanup
    echo -e "${COLOR_CYAN}Cleaning up...${COLOR_RESET}"
    execute_command "umount -R /mnt"
}

# Function to display main menu
main_menu() {
    local fs_type="$1"
    local drive="$2"

    while true; do
        echo -e "${COLOR_CYAN}"
        echo "╔══════════════════════════════════════╗"
        echo "║              Main Menu               ║"
        echo "╠══════════════════════════════════════╣"
        echo "║ 1. Install Vanilla Arch Desktop      ║"
        echo "║ 2. Vanilla Cachyos Options           ║"
        echo "║ 3. Claudemods Distribution Options   ║"
        echo "║ 4. Reboot System                     ║"
        echo "║ 5. Exit                              ║"
        echo "╚══════════════════════════════════════╝"
        echo -e "${COLOR_RESET}"

        echo -e "${COLOR_CYAN}Select an option (1-5): ${COLOR_RESET}"
        read -r choice

        case $choice in
            1)
                install_desktop "$fs_type" "$drive"
                ;;
            2)
                install_cachyos_options "$fs_type" "$drive"
                ;;
            3)
                install_claudemods_distribution "$fs_type" "$drive"
                ;;
            4)
                echo -e "${COLOR_GREEN}Rebooting system...${COLOR_RESET}"
                execute_command "umount -R /mnt 2>/dev/null || true"
                sudo reboot
                ;;
            5)
                echo -e "${COLOR_GREEN}Exiting. Goodbye!${COLOR_RESET}"
                exit 0
                ;;
            *)
                echo -e "${COLOR_RED}Invalid option. Please try again.${COLOR_RESET}"
                ;;
        esac

        echo
        echo -e "${COLOR_YELLOW}Press Enter to continue...${COLOR_RESET}"
        read -r
    done
}

# Main script
main() {
    display_header
    display_available_drives

    echo -e "${COLOR_CYAN}Enter target drive (e.g., /dev/sda): ${COLOR_RESET}"
    read -r drive
    if ! is_block_device "$drive"; then
        echo -e "${COLOR_RED}Error: $drive is not a valid block device${COLOR_RESET}" >&2
        exit 1
    fi

    echo -e "${COLOR_CYAN}Choose filesystem type (ext4/btrfs): ${COLOR_RESET}"
    read -r fs_type

    if [ "$fs_type" = "btrfs" ]; then
        echo -e "${COLOR_CYAN}Executing btrfsrsync.sh with drive: $drive${COLOR_RESET}"
        execute_command "./btrfsrsync.sh $drive"
        echo -e "${COLOR_GREEN}Btrfs installation complete!${COLOR_RESET}"
        exit 0
    fi

    # Show main menu for ext4
    main_menu "$fs_type" "$drive"
}

# Run main function
main "$@"
