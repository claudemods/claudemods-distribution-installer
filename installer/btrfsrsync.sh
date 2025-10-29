#!/bin/bash

COLOR_CYAN="\033[38;2;0;255;255m"
COLOR_RED="\033[31m"
COLOR_GREEN="\033[32m"
COLOR_YELLOW="\033[33m"
COLOR_BLUE="\033[34m"
COLOR_MAGENTA="\033[35m"
COLOR_ORANGE="\033[38;5;208m"
COLOR_PURPLE="\033[38;5;93m"
COLOR_RESET="\033[0m"

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

is_block_device() {
    local path="$1"
    if [ ! -b "$path" ]; then
        return 1
    fi
    return 0
}

directory_exists() {
    local path="$1"
    if [ ! -d "$path" ]; then
        return 1
    fi
    return 0
}

get_uk_date_time() {
    date +"%d-%m-%Y_%I:%M%P"
}

display_available_drives() {
    echo -e "${COLOR_YELLOW}"
    echo "╔══════════════════════════════════════════════════════════════╗"
    echo "║                    Available Drives                         ║"
    echo "╠══════════════════════════════════════════════════════════════╣"
    echo -e "${COLOR_RESET}"

    echo -e "${COLOR_CYAN}Block Devices:${COLOR_RESET}"
    lsblk -o NAME,SIZE,TYPE,MOUNTPOINT,FSTYPE,MODEL | grep -v "loop" | while read line; do
        echo -e "${COLOR_GREEN}  $line${COLOR_RESET}"
    done

    echo -e "${COLOR_YELLOW}"
    echo "╚══════════════════════════════════════════════════════════════╝"
    echo -e "${COLOR_RESET}"
    echo
}

display_header() {
    echo -e "${COLOR_RED}"
    cat << 'EOF'
░█████╗░██╗░░░░░░█████╗░██║░░░██╗██████╗░███████╗███╗░░░███╗░█████╗░██████╗░░██████╗
██╔══██╗██║░░░░░██╔══██╗██║░░░██║██╔══██╗██╔════╝████╗░████║██╔══██╗██╔══██╗██╔════╝
██║░░╚═╝██║░░░░░███████║██║░░░██║██║░░██║█████╗░░██╔████╔██║██║░░██║██║░░██║╚█████╗░
██║░░██╗██║░░░░░██╔══██║██║░░░██║██║░░██║██╔══╝░░██║╚██╔╝██║██║░░██║██║░░██║░╚═══██╗
╚█████╔╝███████╗██║░░██║╚██████╔╝██████╔╝███████╗██║░╚═╝░██║╚█████╔╝██████╔╝██████╔╝
░╚════╝░╚══════╝╚═╝░░░░░░╚═════╝░╚═════╝░╚══════╝╚═╝░░░░░╚═╝░╚════╝░╚═════╝░╚═════╝░
EOF
    echo -e "${COLOR_CYAN}claudemods Distribution Installer Btrfs v1.0 27-10-2025${COLOR_RESET}"
    echo -e "${COLOR_CYAN}Supports Btrfs (with Zstd compression 22) filesystem${COLOR_RESET}"
    echo
}

prepare_target_partitions() {
    local drive="$1"
    execute_command "umount -f ${drive}* 2>/dev/null || true"
    execute_command "wipefs -a $drive"
    execute_command "parted -s $drive mklabel gpt"
    execute_command "parted -s $drive mkpart primary fat32 1MiB 551MiB"
    execute_command "parted -s $drive mkpart primary btrfs 551MiB 100%"
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
    execute_command "mkfs.btrfs -f -L ROOT $root_part"
}

setup_btrfs_subvolumes() {
    local root_part="$1"
    execute_command "mount $root_part /mnt"
    execute_command "btrfs subvolume create /mnt/@"
    execute_command "btrfs subvolume create /mnt/@home"
    execute_command "btrfs subvolume create /mnt/@root")
    execute_command "btrfs subvolume create /mnt/@srv")
    execute_command "btrfs subvolume create /mnt/@cache")
    execute_command "btrfs subvolume create /mnt/@tmp")
    execute_command "btrfs subvolume create /mnt/@log")
    execute_command "mkdir -p /mnt/@/var/lib")
    execute_command "btrfs subvolume create /mnt/@/var/lib/portables")
    execute_command "btrfs subvolume create /mnt/@/var/lib/machines")
    execute_command "umount /mnt"

    execute_command "mount -o subvol=@,compress=zstd:22,compress-force=zstd:22 $root_part /mnt"
    execute_command "mkdir -p /mnt/{home,root,srv,tmp,var/{cache,log},var/lib/{portables,machines},boot/efi}"

    execute_command "mount -o subvol=@home,compress=zstd:22,compress-force=zstd:22 $root_part /mnt/home"
    execute_command "mount -o subvol=@root,compress=zstd:22,compress-force=zstd:22 $root_part /mnt/root"
    execute_command "mount -o subvol=@srv,compress=zstd:22,compress-force=zstd:22 $root_part /mnt/srv"
    execute_command "mount -o subvol=@cache,compress=zstd:22,compress-force=zstd:22 $root_part /mnt/var/cache"
    execute_command "mount -o subvol=@tmp,compress=zstd:22,compress-force=zstd:22 $root_part /mnt/tmp"
    execute_command "mount -o subvol=@log,compress=zstd:22,compress-force=zstd:22 $root_part /mnt/var/log"
    execute_command "mount -o subvol=@/var/lib/portables,compress=zstd:22,compress-force=zstd:22 $root_part /mnt/var/lib/portables"
    execute_command "mount -o subvol=@/var/lib/machines,compress=zstd:22,compress-force=zstd:22 $root_part /mnt/var/lib/machines"
}

install_grub_btrfs() {
    local drive="$1"
    execute_command "mount --bind /dev /mnt/dev"
    execute_command "mount --bind /dev/pts /mnt/dev/pts"
    execute_command "mount --bind /proc /mnt/proc"
    execute_command "mount --bind /sys /mnt/sys"
    execute_command "mount --bind /run /mnt/run"

    execute_command "chroot /mnt /bin/bash -c \"mount -t efivarfs efivarfs /sys/firmware/efi/efivars\""
    execute_command "chroot /mnt /bin/bash -c \"genfstab -U / >> /etc/fstab\""
    execute_command "chroot /mnt /bin/bash -c \"grub-install --target=x86_64-efi --efi-directory=/boot/efi --bootloader-id=GRUB --recheck\""
    execute_command "chroot /mnt /bin/bash -c \"grub-mkconfig -o /boot/grub/grub.cfg\""
    execute_command "chroot /mnt /bin/bash -c \"/opt/btrfsfstabcompressed.sh\""
    execute_command "chroot /mnt /bin/bash -c \"mkinitcpio -P\""
}

# Function to change username in the new system (for Arch TTY Grub)
change_username() {
    local fs_type="$1"
    local drive="$2"

    echo -e "${COLOR_CYAN}Enter new username: ${COLOR_RESET}"
    read -r new_username

    echo -e "${COLOR_CYAN}Mounting system for username change...${COLOR_RESET}"

    # Mount ALL btrfs subvolumes
    execute_command "mount -o subvol=@,compress=zstd:22,compress-force=zstd:22 ${drive}2 /mnt"
    execute_command "mount ${drive}1 /mnt/boot/efi"
    execute_command "mount -o subvol=@home,compress=zstd:22,compress-force=zstd:22 ${drive}2 /mnt/home"
    execute_command "mount -o subvol=@root,compress=zstd:22,compress-force=zstd:22 ${drive}2 /mnt/root"
    execute_command "mount -o subvol=@srv,compress=zstd:22,compress-force=zstd:22 ${drive}2 /mnt/srv"
    execute_command "mount -o subvol=@cache,compress=zstd:22,compress-force=zstd:22 ${drive}2 /mnt/var/cache"
    execute_command "mount -o subvol=@tmp,compress=zstd:22,compress-force=zstd:22 ${drive}2 /mnt/tmp"
    execute_command "mount -o subvol=@log,compress=zstd:22,compress-force=zstd:22 ${drive}2 /mnt/var/log"
    execute_command "mount -o subvol=@/var/lib/portables,compress=zstd:22,compress-force=zstd:22 ${drive}2 /mnt/var/lib/portables"
    execute_command "mount -o subvol=@/var/lib/machines,compress=zstd:22,compress-force=zstd:22 ${drive}2 /mnt/var/lib/machines"
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
    
    # Configure sudo for wheel group if not already configured
    execute_command "chroot /mnt /bin/bash -c \"echo '%wheel ALL=(ALL:ALL) ALL' | tee -a /etc/sudoers\""
    
    # Set password for root
    echo -e "${COLOR_CYAN}Setting password for user 'root'...${COLOR_RESET}"
    execute_command "chroot /mnt /bin/bash -c \"passwd root\""
    

    # Set password for the new user
    echo -e "${COLOR_CYAN}Setting password for user '$new_username'...${COLOR_RESET}"
    execute_command "chroot /mnt /bin/bash -c \"passwd $new_username\""

    # Cleanup
    execute_command "umount -R /mnt"

    echo -e "${COLOR_GREEN}Username changed from 'arch' to '$new_username'${COLOR_RESET}"
}

# Function to create new user (for desktop environments)
create_new_user() {
    local fs_type="$1"
    local drive="$2"

    echo -e "${COLOR_CYAN}Enter new username: ${COLOR_RESET}"
    read -r new_username

    echo -e "${COLOR_CYAN}Mounting system for user creation...${COLOR_RESET}"

    # Mount ALL btrfs subvolumes
    execute_command "mount -o subvol=@,compress=zstd:22,compress-force=zstd:22 ${drive}2 /mnt"
    execute_command "mount ${drive}1 /mnt/boot/efi"
    execute_command "mount -o subvol=@home,compress=zstd:22,compress-force=zstd:22 ${drive}2 /mnt/home"
    execute_command "mount -o subvol=@root,compress=zstd:22,compress-force=zstd:22 ${drive}2 /mnt/root"
    execute_command "mount -o subvol=@srv,compress=zstd:22,compress-force=zstd:22 ${drive}2 /mnt/srv"
    execute_command "mount -o subvol=@cache,compress=zstd:22,compress-force=zstd:22 ${drive}2 /mnt/var/cache"
    execute_command "mount -o subvol=@tmp,compress=zstd:22,compress-force=zstd:22 ${drive}2 /mnt/tmp"
    execute_command "mount -o subvol=@log,compress=zstd:22,compress-force=zstd:22 ${drive}2 /mnt/var/log"
    execute_command "mount -o subvol=@/var/lib/portables,compress=zstd:22,compress-force=zstd:22 ${drive}2 /mnt/var/lib/portables"
    execute_command "mount -o subvol=@/var/lib/machines,compress=zstd:22,compress-force=zstd:22 ${drive}2 /mnt/var/lib/machines"
    execute_command "mount --bind /dev /mnt/dev"
    execute_command "mount --bind /dev/pts /mnt/dev/pts"
    execute_command "mount --bind /proc /mnt/proc"
    execute_command "mount --bind /sys /mnt/sys"
    execute_command "mount --bind /run /mnt/run"

    echo -e "${COLOR_CYAN}Creating new user '$new_username'...${COLOR_RESET}"

    # Create new user with home directory and wheel group
    execute_command "chroot /mnt /bin/bash -c \"useradd -m -G wheel -s /bin/bash $new_username\""
    
    # Set password for the new user
    echo -e "${COLOR_CYAN}Setting password for user '$new_username'...${COLOR_RESET}"
    execute_command "chroot /mnt /bin/bash -c \"passwd $new_username\""
    
    # Set password for root
    echo -e "${COLOR_CYAN}Setting password for user 'root'...${COLOR_RESET}"
    execute_command "chroot /mnt /bin/bash -c \"passwd root\""

    # Configure sudo for wheel group if not already configured
    execute_command "chroot /mnt /bin/bash -c \"echo '%wheel ALL=(ALL:ALL) ALL' | tee -a /etc/sudoers\""

    # Cleanup
    execute_command "umount -R /mnt"

    echo -e "${COLOR_GREEN}User '$new_username' created successfully with sudo privileges${COLOR_RESET}"
}

# Function to select kernel for desktop installations
select_kernel() {
    while true; do
        echo -e "${COLOR_CYAN}"
        echo "╔══════════════════════════════════════════════════════════════╗"
        echo "║                      Select Kernel                          ║"
        echo "╠══════════════════════════════════════════════════════════════╣"
        echo "║  1. linux (Standard)                                        ║"
        echo "║  2. linux-lts (Long Term Support)                           ║"
        echo "║  3. linux-zen (Tuned for desktop performance)               ║"
        echo "║  4. linux-hardened (Security-focused)                       ║"
        echo "╚══════════════════════════════════════════════════════════════╝"
        echo -e "${COLOR_RESET}"

        echo -e "${COLOR_CYAN}Select kernel (1-4): ${COLOR_RESET}"
        read -r kernel_choice

        case $kernel_choice in
            1)
                echo "linux"
                break
                ;;
            2)
                echo "linux-lts"
                break
                ;;
            3)
                echo "linux-zen"
                break
                ;;
            4)
                echo "linux-hardened"
                break
                ;;
            *)
                echo -e "${COLOR_RED}Invalid selection. Please enter a number between 1-4.${COLOR_RESET}"
                ;;
        esac
    done
}

# Function to install arch tty grub (complete installation) using pacstrap
install_arch_tty_grub() {
    local drive="$1"
    local fs_type="btrfs"

    echo -e "${COLOR_CYAN}Starting Arch TTY Grub installation...${COLOR_RESET}"
    
    # Kernel selection for Arch TTY Grub
    local selected_kernel=$(select_kernel)
    echo -e "${COLOR_GREEN}Selected kernel: $selected_kernel${COLOR_RESET}"
    
    # Prepare partitions
    echo -e "${COLOR_CYAN}Preparing partitions...${COLOR_RESET}"
    prepare_target_partitions "$drive"
    
    local efi_part="${drive}1"
    local root_part="${drive}2"
    
    # Setup btrfs filesystem
    echo -e "${COLOR_CYAN}Setting up btrfs filesystem...${COLOR_RESET}"
    setup_btrfs_subvolumes "$root_part"
    
    # Install base system using pacstrap (like other desktop environments)
    echo -e "${COLOR_CYAN}Installing base system with pacstrap...${COLOR_RESET}"
    execute_command "pacstrap /mnt base $selected_kernel linux-firmware grub efibootmgr os-prober sudo vim nano bash-completion"
    
    # Copy btrfsfstabcompressed.sh to the new system
    execute_command "cp btrfsfstabcompressed.sh /mnt/opt"
    execute_command "chmod +x /mnt/opt/btrfsfstabcompressed.sh"
    
    # Mount EFI partition
    execute_command "mount $efi_part /mnt/boot/efi"
    
    # Install GRUB
    echo -e "${COLOR_CYAN}Installing GRUB...${COLOR_RESET}"
    install_grub_btrfs "$drive"
    
    # Create new user (using new method like other desktop environments)
    echo -e "${COLOR_CYAN}Setting up user account...${COLOR_RESET}"
    create_new_user "$fs_type" "$drive"
    
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
            
            # Kernel selection for desktop environments
            local selected_kernel=$(select_kernel)
            echo -e "${COLOR_GREEN}Selected kernel: $selected_kernel${COLOR_RESET}"
            
            # Prepare partitions
            prepare_target_partitions "$drive"
            local efi_part="${drive}1"
            local root_part="${drive}2"
            
            # Setup btrfs filesystem
            setup_btrfs_subvolumes "$root_part"
            
            # Install base system with GNOME and selected kernel
            execute_command "pacstrap /mnt base gnome gnome-extra gdm grub efibootmgr os-prober arch-install-scripts mkinitcpio $selected_kernel linux-firmware sudo"
            
            # Copy btrfsfstabcompressed.sh to the new system
            execute_command "cp btrfsfstabcompressed.sh /mnt/opt"
            execute_command "chmod +x /mnt/opt/btrfsfstabcompressed.sh"
            
            # Mount EFI partition
            execute_command "mount $efi_part /mnt/boot/efi"
            
            # Install GRUB
            install_grub_btrfs "$drive"
            
            # Create new user (using new method for desktop environments)
            echo -e "${COLOR_CYAN}Setting up user account...${COLOR_RESET}"
            create_new_user "$fs_type" "$drive"
            
            # Enable services
            execute_command "chroot /mnt /bin/bash -c \"systemctl enable gdm\""
            
            # Cleanup
            execute_command "umount -R /mnt"
            echo -e "${COLOR_GREEN}GNOME installation completed!${COLOR_RESET}"
            ;;
        3)
            echo -e "${COLOR_CYAN}Installing KDE Plasma...${COLOR_RESET}"
            
            # Kernel selection for desktop environments
            local selected_kernel=$(select_kernel)
            echo -e "${COLOR_GREEN}Selected kernel: $selected_kernel${COLOR_RESET}"
            
            # Prepare partitions
            prepare_target_partitions "$drive"
            local efi_part="${drive}1"
            local root_part="${drive}2"
            
            # Setup btrfs filesystem
            setup_btrfs_subvolumes "$root_part"
            
            # Install base system with KDE and selected kernel
            execute_command "pacstrap /mnt base plasma sddm dolphin konsole grub efibootmgr os-prober arch-install-scripts mkinitcpio $selected_kernel linux-firmware sudo"
            
            # Copy btrfsfstabcompressed.sh to the new system
            execute_command "cp btrfsfstabcompressed.sh /mnt/opt"
            execute_command "chmod +x /mnt/opt/btrfsfstabcompressed.sh"
            
            # Mount EFI partition
            execute_command "mount $efi_part /mnt/boot/efi"
            
            # Install GRUB
            install_grub_btrfs "$drive"
            
            # Create new user (using new method for desktop environments)
            echo -e "${COLOR_CYAN}Setting up user account...${COLOR_RESET}"
            create_new_user "$fs_type" "$drive"
            
            # Enable services
            execute_command "chroot /mnt /bin/bash -c \"systemctl enable sddm\""
            
            # Cleanup
            execute_command "umount -R /mnt"
            echo -e "${COLOR_GREEN}KDE Plasma installation completed!${COLOR_RESET}"
            ;;
        4)
            echo -e "${COLOR_CYAN}Installing XFCE...${COLOR_RESET}"
            
            # Kernel selection for desktop environments
            local selected_kernel=$(select_kernel)
            echo -e "${COLOR_GREEN}Selected kernel: $selected_kernel${COLOR_RESET}"
            
            # Prepare partitions
            prepare_target_partitions "$drive"
            local efi_part="${drive}1"
            local root_part="${drive}2"
            
            # Setup btrfs filesystem
            setup_btrfs_subvolumes "$root_part"
            
            # Install base system with XFCE and selected kernel
            execute_command "pacstrap /mnt base xfce4 xfce4-goodies lightdm lightdm-gtk-greeter grub efibootmgr os-prober arch-install-scripts mkinitcpio $selected_kernel linux-firmware sudo"
            
            # Copy btrfsfstabcompressed.sh to the new system
            execute_command "cp btrfsfstabcompressed.sh /mnt/opt"
            execute_command "chmod +x /mnt/opt/btrfsfstabcompressed.sh"
            
            # Mount EFI partition
            execute_command "mount $efi_part /mnt/boot/efi"
            
            # Install GRUB
            install_grub_btrfs "$drive"
            
            # Create new user (using new method for desktop environments)
            echo -e "${COLOR_CYAN}Setting up user account...${COLOR_RESET}"
            create_new_user "$fs_type" "$drive"
            
            # Enable services
            execute_command "chroot /mnt /bin/bash -c \"systemctl enable lightdm\""
            
            # Cleanup
            execute_command "umount -R /mnt"
            echo -e "${COLOR_GREEN}XFCE installation completed!${COLOR_RESET}"
            ;;
        5)
            echo -e "${COLOR_CYAN}Installing LXQt...${COLOR_RESET}"
            
            # Kernel selection for desktop environments
            local selected_kernel=$(select_kernel)
            echo -e "${COLOR_GREEN}Selected kernel: $selected_kernel${COLOR_RESET}"
            
            # Prepare partitions
            prepare_target_partitions "$drive"
            local efi_part="${drive}1"
            local root_part="${drive}2"
            
            # Setup btrfs filesystem
            setup_btrfs_subvolumes "$root_part"
            
            # Install base system with LXQt and selected kernel
            execute_command "pacstrap /mnt base lxqt sddm grub efibootmgr os-prober arch-install-scripts mkinitcpio $selected_kernel linux-firmware sudo"
            
            # Copy btrfsfstabcompressed.sh to the new system
            execute_command "cp btrfsfstabcompressed.sh /mnt/opt"
            execute_command "chmod +x /mnt/opt/btrfsfstabcompressed.sh"
            
            # Mount EFI partition
            execute_command "mount $efi_part /mnt/boot/efi"
            
            # Install GRUB
            install_grub_btrfs "$drive"
            
            # Create new user (using new method for desktop environments)
            echo -e "${COLOR_CYAN}Setting up user account...${COLOR_RESET}"
            create_new_user "$fs_type" "$drive"
            
            # Enable services
            execute_command "chroot /mnt /bin/bash -c \"systemctl enable sddm\""
            
            # Cleanup
            execute_command "umount -R /mnt"
            echo -e "${COLOR_GREEN}LXQt installation completed!${COLOR_RESET}"
            ;;
        6)
            echo -e "${COLOR_CYAN}Installing Cinnamon...${COLOR_RESET}"
            
            # Kernel selection for desktop environments
            local selected_kernel=$(select_kernel)
            echo -e "${COLOR_GREEN}Selected kernel: $selected_kernel${COLOR_RESET}"
            
            # Prepare partitions
            prepare_target_partitions "$drive"
            local efi_part="${drive}1"
            local root_part="${drive}2"
            
            # Setup btrfs filesystem
            setup_btrfs_subvolumes "$root_part"
            
            # Install base system with Cinnamon and selected kernel
            execute_command "pacstrap /mnt base cinnamon lightdm lightdm-gtk-greeter grub efibootmgr os-prober arch-install-scripts mkinitcpio $selected_kernel linux-firmware sudo"
            
            # Copy btrfsfstabcompressed.sh to the new system
            execute_command "cp btrfsfstabcompressed.sh /mnt/opt"
            execute_command "chmod +x /mnt/opt/btrfsfstabcompressed.sh"
            
            # Mount EFI partition
            execute_command "mount $efi_part /mnt/boot/efi"
            
            # Install GRUB
            install_grub_btrfs "$drive"
            
            # Create new user (using new method for desktop environments)
            echo -e "${COLOR_CYAN}Setting up user account...${COLOR_RESET}"
            create_new_user "$fs_type" "$drive"
            
            # Enable services
            execute_command "chroot /mnt /bin/bash -c \"systemctl enable lightdm\""
            
            # Cleanup
            execute_command "umount -R /mnt"
            echo -e "${COLOR_GREEN}Cinnamon installation completed!${COLOR_RESET}"
            ;;
        7)
            echo -e "${COLOR_CYAN}Installing MATE...${COLOR_RESET}"
            
            # Kernel selection for desktop environments
            local selected_kernel=$(select_kernel)
            echo -e "${COLOR_GREEN}Selected kernel: $selected_kernel${COLOR_RESET}"
            
            # Prepare partitions
            prepare_target_partitions "$drive"
            local efi_part="${drive}1"
            local root_part="${drive}2"
            
            # Setup btrfs filesystem
            setup_btrfs_subvolumes "$root_part"
            
            # Install base system with MATE and selected kernel
            execute_command "pacstrap /mnt base mate mate-extra lightdm lightdm-gtk-greeter grub efibootmgr os-prober arch-install-scripts mkinitcpio $selected_kernel linux-firmware sudo"
            
            # Copy btrfsfstabcompressed.sh to the new system
            execute_command "cp btrfsfstabcompressed.sh /mnt/opt"
            execute_command "chmod +x /mnt/opt/btrfsfstabcompressed.sh"
            
            # Mount EFI partition
            execute_command "mount $efi_part /mnt/boot/efi"
            
            # Install GRUB
            install_grub_btrfs "$drive"
            
            # Create new user (using new method for desktop environments)
            echo -e "${COLOR_CYAN}Setting up user account...${COLOR_RESET}"
            create_new_user "$fs_type" "$drive"
            
            # Enable services
            execute_command "chroot /mnt /bin/bash -c \"systemctl enable lightdm\""
            
            # Cleanup
            execute_command "umount -R /mnt"
            echo -e "${COLOR_GREEN}MATE installation completed!${COLOR_RESET}"
            ;;
        8)
            echo -e "${COLOR_CYAN}Installing Budgie...${COLOR_RESET}"
            
            # Kernel selection for desktop environments
            local selected_kernel=$(select_kernel)
            echo -e "${COLOR_GREEN}Selected kernel: $selected_kernel${COLOR_RESET}"
            
            # Prepare partitions
            prepare_target_partitions "$drive"
            local efi_part="${drive}1"
            local root_part="${drive}2"
            
            # Setup btrfs filesystem
            setup_btrfs_subvolumes "$root_part"
            
            # Install base system with Budgie and selected kernel
            execute_command "pacstrap /mnt base budgie-desktop lightdm lightdm-gtk-greeter grub efibootmgr os-prober arch-install-scripts mkinitcpio $selected_kernel linux-firmware sudo"
            
            # Copy btrfsfstabcompressed.sh to the new system
            execute_command "cp btrfsfstabcompressed.sh /mnt/opt"
            execute_command "chmod +x /mnt/opt/btrfsfstabcompressed.sh"
            
            # Mount EFI partition
            execute_command "mount $efi_part /mnt/boot/efi"
            
            # Install GRUB
            install_grub_btrfs "$drive"
            
            # Create new user (using new method for desktop environments)
            echo -e "${COLOR_CYAN}Setting up user account...${COLOR_RESET}"
            create_new_user "$fs_type" "$drive"
            
            # Enable services
            execute_command "chroot /mnt /bin/bash -c \"systemctl enable lightdm\""
            
            # Cleanup
            execute_command "umount -R /mnt"
            echo -e "${COLOR_GREEN}Budgie installation completed!${COLOR_RESET}"
            ;;
        9)
            echo -e "${COLOR_CYAN}Installing i3 (tiling WM)...${COLOR_RESET}"
            
            # Kernel selection for desktop environments
            local selected_kernel=$(select_kernel)
            echo -e "${COLOR_GREEN}Selected kernel: $selected_kernel${COLOR_RESET}"
            
            # Prepare partitions
            prepare_target_partitions "$drive"
            local efi_part="${drive}1"
            local root_part="${drive}2"
            
            # Setup btrfs filesystem
            setup_btrfs_subvolumes "$root_part"
            
            # Install base system with i3 and selected kernel
            execute_command "pacstrap /mnt base i3-wm i3status i3lock dmenu lightdm lightdm-gtk-greeter grub efibootmgr os-prober arch-install-scripts mkinitcpio $selected_kernel linux-firmware sudo"
            
            # Copy btrfsfstabcompressed.sh to the new system
            execute_command "cp btrfsfstabcompressed.sh /mnt/opt"
            execute_command "chmod +x /mnt/opt/btrfsfstabcompressed.sh"
            
            # Mount EFI partition
            execute_command "mount $efi_part /mnt/boot/efi"
            
            # Install GRUB
            install_grub_btrfs "$drive"
            
            # Create new user (using new method for desktop environments)
            echo -e "${COLOR_CYAN}Setting up user account...${COLOR_RESET}"
            create_new_user "$fs_type" "$drive"
            
            # Enable services
            execute_command "chroot /mnt /bin/bash -c \"systemctl enable lightdm\""
            
            # Cleanup
            execute_command "umount -R /mnt"
            echo -e "${COLOR_GREEN}i3 installation completed!${COLOR_RESET}"
            ;;
        10)
            echo -e "${COLOR_CYAN}Installing Sway (Wayland tiling)...${COLOR_RESET}"
            
            # Kernel selection for desktop environments
            local selected_kernel=$(select_kernel)
            echo -e "${COLOR_GREEN}Selected kernel: $selected_kernel${COLOR_RESET}"
            
            # Prepare partitions
            prepare_target_partitions "$drive"
            local efi_part="${drive}1"
            local root_part="${drive}2"
            
            # Setup btrfs filesystem
            setup_btrfs_subvolumes "$root_part"
            
            # Install base system with Sway and selected kernel
            execute_command "pacstrap /mnt base sway swaybg waybar wofi lightdm lightdm-gtk-greeter grub efibootmgr os-prober arch-install-scripts mkinitcpio $selected_kernel linux-firmware sudo"
            
            # Copy btrfsfstabcompressed.sh to the new system
            execute_command "cp btrfsfstabcompressed.sh /mnt/opt"
            execute_command "chmod +x /mnt/opt/btrfsfstabcompressed.sh"
            
            # Mount EFI partition
            execute_command "mount $efi_part /mnt/boot/efi"
            
            # Install GRUB
            install_grub_btrfs "$drive"
            
            # Create new user (using new method for desktop environments)
            echo -e "${COLOR_CYAN}Setting up user account...${COLOR_RESET}"
            create_new_user "$fs_type" "$drive"
            
            # Enable services
            execute_command "chroot /mnt /bin/bash -c \"systemctl enable lightdm\""
            
            # Cleanup
            execute_command "umount -R /mnt"
            echo -e "${COLOR_GREEN}Sway installation completed!${COLOR_RESET}"
            ;;
        11)
            echo -e "${COLOR_PURPLE}Installing Hyprland (Modern Wayland Compositor)...${COLOR_RESET}"
            
            # Kernel selection for desktop environments
            local selected_kernel=$(select_kernel)
            echo -e "${COLOR_GREEN}Selected kernel: $selected_kernel${COLOR_RESET}"
            
            # Prepare partitions
            prepare_target_partitions "$drive"
            local efi_part="${drive}1"
            local root_part="${drive}2"
            
            # Setup btrfs filesystem
            setup_btrfs_subvolumes "$root_part"
            
            # Install base system with Hyprland and selected kernel
            execute_command "pacstrap /mnt base hyprland waybar rofi wl-clipboard sddm grub efibootmgr os-prober arch-install-scripts mkinitcpio $selected_kernel linux-firmware sudo"
            
            # Copy btrfsfstabcompressed.sh to the new system
            execute_command "cp btrfsfstabcompressed.sh /mnt/opt"
            execute_command "chmod +x /mnt/opt/btrfsfstabcompressed.sh"
            
            # Mount EFI partition
            execute_command "mount $efi_part /mnt/boot/efi"
            
            # Install GRUB
            install_grub_btrfs "$drive"
            
            # Create new user (using new method for desktop environments)
            echo -e "${COLOR_CYAN}Setting up user account...${COLOR_RESET}"
            create_new_user "$fs_type" "$drive"
            
            # Enable services
            execute_command "chroot /mnt /bin/bash -c \"systemctl enable sddm\""
            
            # Cleanup
            execute_command "umount -R /mnt"
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

    echo -e "${COLOR_CYAN}Installing CachyOS...${COLOR_RESET}"
    
    # Kernel selection for CachyOS
    local selected_kernel=$(select_kernel)
    echo -e "${COLOR_GREEN}Selected kernel: $selected_kernel${COLOR_RESET}"
    
    # Prepare partitions
    prepare_target_partitions "$drive"
    local efi_part="${drive}1"
    local root_part="${drive}2"
    
    # Setup btrfs filesystem
    setup_btrfs_subvolumes "$root_part"
    
    # Install base system with selected kernel
    execute_command "pacstrap /mnt base grub efibootmgr os-prober arch-install-scripts mkinitcpio $selected_kernel linux-firmware sudo"
    
    # Copy btrfsfstabcompressed.sh to the new system
    execute_command "cp btrfsfstabcompressed.sh /mnt/opt"
    execute_command "chmod +x /mnt/opt/btrfsfstabcompressed.sh"
    
    # Mount EFI partition
    execute_command "mount $efi_part /mnt/boot/efi"
    
    # Install GRUB
    install_grub_btrfs "$drive"
    
    # Create new user (using new method for desktop environments)
    echo -e "${COLOR_CYAN}Setting up user account...${COLOR_RESET}"
    create_new_user "$fs_type" "$drive"
    
    # Download and setup CachyOS repositories
    execute_command "curl https://mirror.cachyos.org/cachyos-repo.tar.xz -o cachyos-repo.tar.xz"
    execute_command "tar xvf cachyos-repo.tar.xz && cd cachyos-repo"
    execute_command "./cachyos-repo.sh"

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

    echo -e "${COLOR_CYAN}Installing claudemods distribution...${COLOR_RESET}"
    
    # Kernel selection for claudemods distribution
    local selected_kernel=$(select_kernel)
    echo -e "${COLOR_GREEN}Selected kernel: $selected_kernel${COLOR_RESET}"
    
    # Prepare partitions
    prepare_target_partitions "$drive"
    local efi_part="${drive}1"
    local root_part="${drive}2"
    
    # Setup btrfs filesystem
    setup_btrfs_subvolumes "$root_part"
    
    # Install base system with selected kernel
    execute_command "pacstrap /mnt base grub efibootmgr os-prober arch-install-scripts mkinitcpio $selected_kernel linux-firmware sudo"
    
    # Copy btrfsfstabcompressed.sh to the new system
    execute_command "cp btrfsfstabcompressed.sh /mnt/opt"
    execute_command "chmod +x /mnt/opt/btrfsfstabcompressed.sh"
    
    # Mount EFI partition
    execute_command "mount $efi_part /mnt/boot/efi"
    
    # Install GRUB
    install_grub_btrfs "$drive"
    
    # Create new user (using new method for desktop environments)
    echo -e "${COLOR_CYAN}Setting up user account...${COLOR_RESET}"
    create_new_user "$fs_type" "$drive"

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

    # Show main menu for btrfs
    main_menu "btrfs" "$drive"
}

# Run main function
main "$@"
