#include <iostream>
#include <string>
#include <vector>
#include <cstdlib>
#include <cstring>
#include <sstream>
#include <iomanip>
#include <map>
#include <algorithm>
#include <functional>

// Color definitions
const std::string COLOR_CYAN = "\033[38;2;0;255;255m";
const std::string COLOR_RED = "\033[31m";
const std::string COLOR_GREEN = "\033[32m";
const std::string COLOR_YELLOW = "\033[33m";
const std::string COLOR_BLUE = "\033[34m";
const std::string COLOR_MAGENTA = "\033[35m";
const std::string COLOR_ORANGE = "\033[38;5;208m";
const std::string COLOR_PURPLE = "\033[38;5;93m";
const std::string COLOR_RESET = "\033[0m";

class ArchInstaller {
private:
    // Function to execute commands with error handling
    int execute_command(const std::string& cmd) {
        std::cout << COLOR_CYAN;
        std::string full_cmd = "sudo " + cmd;
        int status = system(full_cmd.c_str());
        std::cout << COLOR_RESET;
        if (status != 0) {
            std::cerr << COLOR_RED << "Error executing: " << full_cmd << COLOR_RESET << std::endl;
            exit(1);
        }
        return status;
    }

    // Function to check if path is a block device
    bool is_block_device(const std::string& path) {
        std::string cmd = "test -b " + path;
        return system(cmd.c_str()) == 0;
    }

    // Function to check if directory exists
    bool directory_exists(const std::string& path) {
        std::string cmd = "test -d " + path;
        return system(cmd.c_str()) == 0;
    }

    // Function to get UK date time
    std::string get_uk_date_time() {
        std::string cmd = "date +\"%d-%m-%Y_%I:%M%P\"";
        std::string result;
        char buffer[128];
        FILE* pipe = popen(cmd.c_str(), "r");
        if (!pipe) return "";
        while (fgets(buffer, sizeof(buffer), pipe) != nullptr) {
            result += buffer;
        }
        pclose(pipe);
        // Remove newline
        if (!result.empty() && result[result.length()-1] == '\n') {
            result.erase(result.length()-1);
        }
        return result;
    }

    // Function to display available drives
    void display_available_drives() {
        std::cout << COLOR_YELLOW;
        std::cout << "╔══════════════════════════════════════════════════════════════╗" << std::endl;
        std::cout << "║                    Available Drives                         ║" << std::endl;
        std::cout << "╠══════════════════════════════════════════════════════════════╣" << std::endl;
        std::cout << COLOR_RESET;

        std::cout << COLOR_CYAN << "Block Devices:" << COLOR_RESET << std::endl;
        execute_command("lsblk -o NAME,SIZE,TYPE,MOUNTPOINT,FSTYPE,MODEL | grep -v \"loop\"");
        
        std::cout << COLOR_YELLOW;
        std::cout << "╚══════════════════════════════════════════════════════════════╝" << std::endl;
        std::cout << COLOR_RESET << std::endl;
    }

    // Function to display header
    void display_header() {
        std::cout << COLOR_RED;
        std::cout << "░█████╗░██╗░░░░░░█████╗░██║░░░██╗██████╗░███████╗███╗░░░███╗░█████╗░██████╗░░██████╗" << std::endl;
        std::cout << "██╔══██╗██║░░░░░██╔══██╗██║░░░██║██╔══██╗██╔════╝████╗░████║██╔══██╗██╔══██╗██╔════╝" << std::endl;
        std::cout << "██║░░╚═╝██║░░░░░███████║██║░░░██║██║░░██║█████╗░░██╔████╔██║██║░░██║██║░░██║╚█████╗░" << std::endl;
        std::cout << "██║░░██╗██║░░░░░██╔══██║██║░░░██║██║░░██║██╔══╝░░██║╚██╔╝██║██║░░██║██║░░██║░╚═══██╗" << std::endl;
        std::cout << "╚█████╔╝███████╗██║░░██║╚██████╔╝██████╔╝███████╗██║░╚═╝░██║╚█████╔╝██████╔╝██████╔╝" << std::endl;
        std::cout << "░╚════╝░╚══════╝╚═╝░░░░░░╚═════╝░╚═════╝░╚══════╝╚═╝░░░░░╚═╝░╚════╝░╚═════╝░╚═════╝░" << std::endl;
        std::cout << COLOR_CYAN << "claudemods distribution installer v1.0 28-10-2025" << COLOR_RESET << std::endl;
        std::cout << COLOR_CYAN << "Supports Ext4 And Btrfs filesystems" << COLOR_RESET << std::endl;
        std::cout << std::endl;
    }

    // Function to prepare target partitions
    void prepare_target_partitions(const std::string& drive, const std::string& fs_type) {
        execute_command("umount -f " + drive + "* 2>/dev/null || true");
        execute_command("wipefs -a " + drive);
        execute_command("parted -s " + drive + " mklabel gpt");
        execute_command("parted -s " + drive + " mkpart primary fat32 1MiB 551MiB");
        execute_command("parted -s " + drive + " mkpart primary " + fs_type + " 551MiB 100%");
        execute_command("parted -s " + drive + " set 1 esp on");
        execute_command("partprobe " + drive);
        
        // Sleep for 2 seconds
        system("sleep 2");

        std::string efi_part = drive + "1";
        std::string root_part = drive + "2";

        if (!is_block_device(efi_part) || !is_block_device(root_part)) {
            std::cerr << COLOR_RED << "Error: Failed to create partitions" << COLOR_RESET << std::endl;
            exit(1);
        }

        execute_command("mkfs.vfat -F32 " + efi_part);
        execute_command("mkfs.ext4 -F -L ROOT " + root_part);
    }

    // Function to setup Ext4 filesystem
    void setup_ext4_filesystem(const std::string& root_part) {
        execute_command("mount " + root_part + " /mnt");
        execute_command("mkdir -p /mnt/{home,boot/efi,etc,usr,var,proc,sys,dev,tmp,run}");
    }

    // Function to install GRUB for Ext4
    void install_grub_ext4(const std::string& drive) {
        execute_command("mount --bind /dev /mnt/dev");
        execute_command("mount --bind /dev/pts /mnt/dev/pts");
        execute_command("mount --bind /proc /mnt/proc");
        execute_command("mount --bind /sys /mnt/sys");
        execute_command("mount --bind /run /mnt/run");
        execute_command("chroot /mnt /bin/bash -c \"mount -t efivarfs efivarfs /sys/firmware/efi/efivars \"");
        execute_command("chroot /mnt /bin/bash -c \"genfstab -U / >> /etc/fstab\"");
        execute_command("chroot /mnt /bin/bash -c \"grub-install --target=x86_64-efi --efi-directory=/boot/efi --bootloader-id=GRUB --recheck\"");
        execute_command("chroot /mnt /bin/bash -c \"grub-mkconfig -o /boot/grub/grub.cfg\"");
        execute_command("chroot /mnt /bin/bash -c \"mkinitcpio -P\"");
    }

    // Function to change username in the new system (for Arch TTY Grub)
    void change_username(const std::string& fs_type, const std::string& drive) {
        std::string new_username;
        std::cout << COLOR_CYAN << "Enter new username: " << COLOR_RESET;
        std::getline(std::cin, new_username);

        std::cout << COLOR_CYAN << "Mounting system for username change..." << COLOR_RESET << std::endl;

        execute_command("mount " + drive + "2 /mnt");
        execute_command("mount " + drive + "1 /mnt/boot/efi");
        execute_command("mount --bind /dev /mnt/dev");
        execute_command("mount --bind /dev/pts /mnt/dev/pts");
        execute_command("mount --bind /proc /mnt/proc");
        execute_command("mount --bind /sys /mnt/sys");
        execute_command("mount --bind /run /mnt/run");

        std::cout << COLOR_CYAN << "Changing username from 'arch' to '" << new_username << "'..." << COLOR_RESET << std::endl;

        // Change username from arch to new username
        execute_command("chroot /mnt /bin/bash -c \"usermod -l " + new_username + " arch\"");
        
        // Change home directory name
        execute_command("chroot /mnt /bin/bash -c \"mv /home/arch /home/" + new_username + "\"");
        
        // Change home directory in passwd
        execute_command("chroot /mnt /bin/bash -c \"usermod -d /home/" + new_username + " " + new_username + "\"");
        
        // Change group name
        execute_command("chroot /mnt /bin/bash -c \"groupmod -n " + new_username + " arch\"");

        std::cout << COLOR_CYAN << "Adding " << new_username << " to sudo group..." << COLOR_RESET << std::endl;
        execute_command("chroot /mnt /bin/bash -c \"gpasswd -a " + new_username + " wheel\"");

        // Set password for root
        std::cout << COLOR_CYAN << "Setting password for user 'root'..." << COLOR_RESET << std::endl;
        execute_command("chroot /mnt /bin/bash -c \"passwd root\"");
        
        // Set password for the new user
        std::cout << COLOR_CYAN << "Setting password for user '" << new_username << "'..." << COLOR_RESET << std::endl;
        execute_command("chroot /mnt /bin/bash -c \"passwd " + new_username + "\"");
        
        // Configure sudo for wheel group if not already configured
        execute_command("chroot /mnt /bin/bash -c \"echo '%wheel ALL=(ALL:ALL) ALL' | tee -a /etc/sudoers\"");

        // Cleanup
        execute_command("umount -R /mnt");

        std::cout << COLOR_GREEN << "Username changed from 'arch' to '" << new_username << "'" << COLOR_RESET << std::endl;
    }

    // Function to create new user (for desktop environments)
    void create_new_user(const std::string& fs_type, const std::string& drive) {
        std::string new_username;
        std::cout << COLOR_CYAN << "Enter new username: " << COLOR_RESET;
        std::getline(std::cin, new_username);

        std::cout << COLOR_CYAN << "Mounting system for user creation..." << COLOR_RESET << std::endl;

        execute_command("mount " + drive + "2 /mnt");
        execute_command("mount " + drive + "1 /mnt/boot/efi");
        execute_command("mount --bind /dev /mnt/dev");
        execute_command("mount --bind /dev/pts /mnt/dev/pts");
        execute_command("mount --bind /proc /mnt/proc");
        execute_command("mount --bind /sys /mnt/sys");
        execute_command("mount --bind /run /mnt/run");

        std::cout << COLOR_CYAN << "Creating new user '" << new_username << "'..." << COLOR_RESET << std::endl;

        // Create new user with home directory and wheel group
        execute_command("chroot /mnt /bin/bash -c \"useradd -m -G wheel -s /bin/bash " + new_username + "\"");
        
        // Set password for root
        std::cout << COLOR_CYAN << "Setting password for user 'root'..." << COLOR_RESET << std::endl;
        execute_command("chroot /mnt /bin/bash -c \"passwd root\"");
        
        // Set password for the new user
        std::cout << COLOR_CYAN << "Setting password for user '" << new_username << "'..." << COLOR_RESET << std::endl;
        execute_command("chroot /mnt /bin/bash -c \"passwd " + new_username + "\"");

        // Configure sudo for wheel group if not already configured
        execute_command("chroot /mnt /bin/bash -c \"echo '%wheel ALL=(ALL:ALL) ALL' | tee -a /etc/sudoers\"");

        // Cleanup
        execute_command("umount -R /mnt");

        std::cout << COLOR_GREEN << "User '" << new_username << "' created successfully with sudo privileges" << COLOR_RESET << std::endl;
    }

    // Function to select kernel for desktop installations
    std::string select_kernel() {
        while (true) {
            std::cout << COLOR_CYAN;
            std::cout << "╔══════════════════════════════════════════════════════════════╗" << std::endl;
            std::cout << "║                      Select Kernel                          ║" << std::endl;
            std::cout << "╠══════════════════════════════════════════════════════════════╣" << std::endl;
            std::cout << "║  1. linux (Standard)                                        ║" << std::endl;
            std::cout << "║  2. linux-lts (Long Term Support)                           ║" << std::endl;
            std::cout << "║  3. linux-zen (Tuned for desktop performance)               ║" << std::endl;
            std::cout << "║  4. linux-hardened (Security-focused)                       ║" << std::endl;
            std::cout << "╚══════════════════════════════════════════════════════════════╝" << std::endl;
            std::cout << COLOR_RESET;

            std::cout << COLOR_CYAN << "Select kernel (1-4): " << COLOR_RESET;
            std::string kernel_choice;
            std::getline(std::cin, kernel_choice);

            if (kernel_choice == "1") {
                return "linux";
            } else if (kernel_choice == "2") {
                return "linux-lts";
            } else if (kernel_choice == "3") {
                return "linux-zen";
            } else if (kernel_choice == "4") {
                return "linux-hardened";
            } else {
                std::cout << COLOR_RED << "Invalid selection. Please enter a number between 1-4." << COLOR_RESET << std::endl;
            }
        }
    }

    // Function to install arch tty grub (complete installation) using pacstrap
    void install_arch_tty_grub(const std::string& drive) {
        std::string fs_type = "ext4";

        std::cout << COLOR_CYAN << "Starting Arch TTY Grub installation..." << COLOR_RESET << std::endl;
        
        // Kernel selection for Arch TTY Grub
        std::string selected_kernel = select_kernel();
        std::cout << COLOR_GREEN << "Selected kernel: " << selected_kernel << COLOR_RESET << std::endl;
        
        // Prepare partitions
        std::cout << COLOR_CYAN << "Preparing partitions..." << COLOR_RESET << std::endl;
        prepare_target_partitions(drive, fs_type);
        
        std::string efi_part = drive + "1";
        std::string root_part = drive + "2";
        
        // Setup filesystem
        std::cout << COLOR_CYAN << "Setting up filesystem..." << COLOR_RESET << std::endl;
        setup_ext4_filesystem(root_part);
        
        // Install base system using pacstrap (like other desktop environments)
        std::cout << COLOR_CYAN << "Installing base system with pacstrap..." << COLOR_RESET << std::endl;
        execute_command("pacstrap /mnt base " + selected_kernel + " linux-firmware grub efibootmgr os-prober sudo arch-install-scripts mkinitcpio vim nano bash-completion");
        
        // Mount EFI partition
        execute_command("mount " + efi_part + " /mnt/boot/efi");
        
        // Install GRUB
        std::cout << COLOR_CYAN << "Installing GRUB..." << COLOR_RESET << std::endl;
        install_grub_ext4(drive);
        
        // Create new user (using new method like other desktop environments)
        std::cout << COLOR_CYAN << "Setting up user account..." << COLOR_RESET << std::endl;
        create_new_user(fs_type, drive);
        
        std::cout << COLOR_GREEN << "Arch TTY Grub installation completed successfully!" << COLOR_RESET << std::endl;
    }

    // Function to install desktop environments
    void install_desktop(const std::string& fs_type, const std::string& drive) {
        // Display desktop options - Top 10 Arch package list
        std::cout << COLOR_CYAN;
        std::cout << "╔══════════════════════════════════════════════════════════════╗" << std::endl;
        std::cout << "║                   Desktop Environments                       ║" << std::endl;
        std::cout << "╠══════════════════════════════════════════════════════════════╣" << std::endl;
        std::cout << "║  1. Arch TTY Grub (Complete Installation)                   ║" << std::endl;
        std::cout << "║  2. GNOME                                                   ║" << std::endl;
        std::cout << "║  3. KDE Plasma                                              ║" << std::endl;
        std::cout << "║  4. XFCE                                                    ║" << std::endl;
        std::cout << "║  5. LXQt                                                   ║" << std::endl;
        std::cout << "║  6. Cinnamon                                                ║" << std::endl;
        std::cout << "║  7. MATE                                                    ║" << std::endl;
        std::cout << "║  8. Budgie                                                  ║" << std::endl;
        std::cout << "║  9. i3 (tiling WM)                                          ║" << std::endl;
        std::cout << "║ 10. Sway (Wayland tiling)                                   ║" << std::endl;
        std::cout << "║ 11. Hyprland (Wayland)                                      ║" << std::endl;
        std::cout << "║ 12. Return to Main Menu                                     ║" << std::endl;
        std::cout << "╚══════════════════════════════════════════════════════════════╝" << std::endl;
        std::cout << COLOR_RESET;

        std::cout << COLOR_CYAN << "Select desktop environment (1-12): " << COLOR_RESET;
        std::string desktop_choice;
        std::getline(std::cin, desktop_choice);

        if (desktop_choice == "1") {
            std::cout << COLOR_CYAN << "Starting Arch TTY Grub installation..." << COLOR_RESET << std::endl;
            install_arch_tty_grub(drive);
        } else if (desktop_choice == "2") {
            std::cout << COLOR_CYAN << "Installing GNOME Desktop..." << COLOR_RESET << std::endl;
            
            // Kernel selection for desktop environments
            std::string selected_kernel = select_kernel();
            std::cout << COLOR_GREEN << "Selected kernel: " << selected_kernel << COLOR_RESET << std::endl;
            
            // Prepare partitions
            prepare_target_partitions(drive, "ext4");
            std::string efi_part = drive + "1";
            std::string root_part = drive + "2";
            
            // Setup filesystem
            setup_ext4_filesystem(root_part);
            
            // Install base system with GNOME and selected kernel
            execute_command("pacstrap /mnt base gnome gnome-extra gdm grub efibootmgr os-prober arch-install-scripts mkinitcpio " + selected_kernel + " linux-firmware sudo");
            
            // Mount EFI partition
            execute_command("mount " + efi_part + " /mnt/boot/efi");
            
            // Install GRUB
            install_grub_ext4(drive);
            
            // Create new user (using new method for desktop environments)
            std::cout << COLOR_CYAN << "Setting up user account..." << COLOR_RESET << std::endl;
            create_new_user(fs_type, drive);
            
            // Enable services
            execute_command("chroot /mnt /bin/bash -c \"systemctl enable gdm\"");
            
            // Cleanup
            execute_command("umount -R /mnt");
            std::cout << COLOR_GREEN << "GNOME installation completed!" << COLOR_RESET << std::endl;
        } else if (desktop_choice == "3") {
            std::cout << COLOR_CYAN << "Installing KDE Plasma..." << COLOR_RESET << std::endl;
            
            // Kernel selection for desktop environments
            std::string selected_kernel = select_kernel();
            std::cout << COLOR_GREEN << "Selected kernel: " << selected_kernel << COLOR_RESET << std::endl;
            
            // Prepare partitions
            prepare_target_partitions(drive, "ext4");
            std::string efi_part = drive + "1";
            std::string root_part = drive + "2";
            
            // Setup filesystem
            setup_ext4_filesystem(root_part);
            
            // Install base system with KDE and selected kernel
            execute_command("pacstrap /mnt base plasma sddm dolphin konsole grub efibootmgr os-prober arch-install-scripts mkinitcpio " + selected_kernel + " linux-firmware sudo");
            
            // Mount EFI partition
            execute_command("mount " + efi_part + " /mnt/boot/efi");
            
            // Install GRUB
            install_grub_ext4(drive);
            
            // Create new user (using new method for desktop environments)
            std::cout << COLOR_CYAN << "Setting up user account..." << COLOR_RESET << std::endl;
            create_new_user(fs_type, drive);
            
            // Enable services
            execute_command("chroot /mnt /bin/bash -c \"systemctl enable sddm\"");
            
            // Cleanup
            execute_command("umount -R /mnt");
            std::cout << COLOR_GREEN << "KDE Plasma installation completed!" << COLOR_RESET << std::endl;
        } else if (desktop_choice == "4") {
            std::cout << COLOR_CYAN << "Installing XFCE..." << COLOR_RESET << std::endl;
            
            // Kernel selection for desktop environments
            std::string selected_kernel = select_kernel();
            std::cout << COLOR_GREEN << "Selected kernel: " << selected_kernel << COLOR_RESET << std::endl;
            
            // Prepare partitions
            prepare_target_partitions(drive, "ext4");
            std::string efi_part = drive + "1";
            std::string root_part = drive + "2";
            
            // Setup filesystem
            setup_ext4_filesystem(root_part);
            
            // Install base system with XFCE and selected kernel
            execute_command("pacstrap /mnt base xfce4 xfce4-goodies lightdm lightdm-gtk-greeter grub efibootmgr os-prober arch-install-scripts mkinitcpio " + selected_kernel + " linux-firmware sudo");
            
            // Mount EFI partition
            execute_command("mount " + efi_part + " /mnt/boot/efi");
            
            // Install GRUB
            install_grub_ext4(drive);
            
            // Create new user (using new method for desktop environments)
            std::cout << COLOR_CYAN << "Setting up user account..." << COLOR_RESET << std::endl;
            create_new_user(fs_type, drive);
            
            // Enable services
            execute_command("chroot /mnt /bin/bash -c \"systemctl enable lightdm\"");
            
            // Cleanup
            execute_command("umount -R /mnt");
            std::cout << COLOR_GREEN << "XFCE installation completed!" << COLOR_RESET << std::endl;
        } else if (desktop_choice == "5") {
            std::cout << COLOR_CYAN << "Installing LXQt..." << COLOR_RESET << std::endl;
            
            // Kernel selection for desktop environments
            std::string selected_kernel = select_kernel();
            std::cout << COLOR_GREEN << "Selected kernel: " << selected_kernel << COLOR_RESET << std::endl;
            
            // Prepare partitions
            prepare_target_partitions(drive, "ext4");
            std::string efi_part = drive + "1";
            std::string root_part = drive + "2";
            
            // Setup filesystem
            setup_ext4_filesystem(root_part);
            
            // Install base system with LXQt and selected kernel
            execute_command("pacstrap /mnt base lxqt sddm grub efibootmgr os-prober arch-install-scripts mkinitcpio " + selected_kernel + " linux-firmware sudo");
            
            // Mount EFI partition
            execute_command("mount " + efi_part + " /mnt/boot/efi");
            
            // Install GRUB
            install_grub_ext4(drive);
            
            // Create new user (using new method for desktop environments)
            std::cout << COLOR_CYAN << "Setting up user account..." << COLOR_RESET << std::endl;
            create_new_user(fs_type, drive);
            
            // Enable services
            execute_command("chroot /mnt /bin/bash -c \"systemctl enable sddm\"");
            
            // Cleanup
            execute_command("umount -R /mnt");
            std::cout << COLOR_GREEN << "LXQt installation completed!" << COLOR_RESET << std::endl;
        } else if (desktop_choice == "6") {
            std::cout << COLOR_CYAN << "Installing Cinnamon..." << COLOR_RESET << std::endl;
            
            // Kernel selection for desktop environments
            std::string selected_kernel = select_kernel();
            std::cout << COLOR_GREEN << "Selected kernel: " << selected_kernel << COLOR_RESET << std::endl;
            
            // Prepare partitions
            prepare_target_partitions(drive, "ext4");
            std::string efi_part = drive + "1";
            std::string root_part = drive + "2";
            
            // Setup filesystem
            setup_ext4_filesystem(root_part);
            
            // Install base system with Cinnamon and selected kernel
            execute_command("pacstrap /mnt base cinnamon lightdm lightdm-gtk-greeter grub efibootmgr os-prober arch-install-scripts mkinitcpio " + selected_kernel + " linux-firmware sudo");
            
            // Mount EFI partition
            execute_command("mount " + efi_part + " /mnt/boot/efi");
            
            // Install GRUB
            install_grub_ext4(drive);
            
            // Create new user (using new method for desktop environments)
            std::cout << COLOR_CYAN << "Setting up user account..." << COLOR_RESET << std::endl;
            create_new_user(fs_type, drive);
            
            // Enable services
            execute_command("chroot /mnt /bin/bash -c \"systemctl enable lightdm\"");
            
            // Cleanup
            execute_command("umount -R /mnt");
            std::cout << COLOR_GREEN << "Cinnamon installation completed!" << COLOR_RESET << std::endl;
        } else if (desktop_choice == "7") {
            std::cout << COLOR_CYAN << "Installing MATE..." << COLOR_RESET << std::endl;
            
            // Kernel selection for desktop environments
            std::string selected_kernel = select_kernel();
            std::cout << COLOR_GREEN << "Selected kernel: " << selected_kernel << COLOR_RESET << std::endl;
            
            // Prepare partitions
            prepare_target_partitions(drive, "ext4");
            std::string efi_part = drive + "1";
            std::string root_part = drive + "2";
            
            // Setup filesystem
            setup_ext4_filesystem(root_part);
            
            // Install base system with MATE and selected kernel
            execute_command("pacstrap /mnt base mate mate-extra lightdm lightdm-gtk-greeter grub efibootmgr os-prober arch-install-scripts mkinitcpio " + selected_kernel + " linux-firmware sudo");
            
            // Mount EFI partition
            execute_command("mount " + efi_part + " /mnt/boot/efi");
            
            // Install GRUB
            install_grub_ext4(drive);
            
            // Create new user (using new method for desktop environments)
            std::cout << COLOR_CYAN << "Setting up user account..." << COLOR_RESET << std::endl;
            create_new_user(fs_type, drive);
            
            // Enable services
            execute_command("chroot /mnt /bin/bash -c \"systemctl enable lightdm\"");
            
            // Cleanup
            execute_command("umount -R /mnt");
            std::cout << COLOR_GREEN << "MATE installation completed!" << COLOR_RESET << std::endl;
        } else if (desktop_choice == "8") {
            std::cout << COLOR_CYAN << "Installing Budgie..." << COLOR_RESET << std::endl;
            
            // Kernel selection for desktop environments
            std::string selected_kernel = select_kernel();
            std::cout << COLOR_GREEN << "Selected kernel: " << selected_kernel << COLOR_RESET << std::endl;
            
            // Prepare partitions
            prepare_target_partitions(drive, "ext4");
            std::string efi_part = drive + "1";
            std::string root_part = drive + "2";
            
            // Setup filesystem
            setup_ext4_filesystem(root_part);
            
            // Install base system with Budgie and selected kernel
            execute_command("pacstrap /mnt base budgie-desktop lightdm lightdm-gtk-greeter grub efibootmgr os-prober arch-install-scripts mkinitcpio " + selected_kernel + " linux-firmware sudo");
            
            // Mount EFI partition
            execute_command("mount " + efi_part + " /mnt/boot/efi");
            
            // Install GRUB
            install_grub_ext4(drive);
            
            // Create new user (using new method for desktop environments)
            std::cout << COLOR_CYAN << "Setting up user account..." << COLOR_RESET << std::endl;
            create_new_user(fs_type, drive);
            
            // Enable services
            execute_command("chroot /mnt /bin/bash -c \"systemctl enable lightdm\"");
            
            // Cleanup
            execute_command("umount -R /mnt");
            std::cout << COLOR_GREEN << "Budgie installation completed!" << COLOR_RESET << std::endl;
        } else if (desktop_choice == "9") {
            std::cout << COLOR_CYAN << "Installing i3 (tiling WM)..." << COLOR_RESET << std::endl;
            
            // Kernel selection for desktop environments
            std::string selected_kernel = select_kernel();
            std::cout << COLOR_GREEN << "Selected kernel: " << selected_kernel << COLOR_RESET << std::endl;
            
            // Prepare partitions
            prepare_target_partitions(drive, "ext4");
            std::string efi_part = drive + "1";
            std::string root_part = drive + "2";
            
            // Setup filesystem
            setup_ext4_filesystem(root_part);
            
            // Install base system with i3 and selected kernel
            execute_command("pacstrap /mnt base i3-wm i3status i3lock dmenu lightdm lightdm-gtk-greeter grub efibootmgr os-prober arch-install-scripts mkinitcpio " + selected_kernel + " linux-firmware sudo");
            
            // Mount EFI partition
            execute_command("mount " + efi_part + " /mnt/boot/efi");
            
            // Install GRUB
            install_grub_ext4(drive);
            
            // Create new user (using new method for desktop environments)
            std::cout << COLOR_CYAN << "Setting up user account..." << COLOR_RESET << std::endl;
            create_new_user(fs_type, drive);
            
            // Enable services
            execute_command("chroot /mnt /bin/bash -c \"systemctl enable lightdm\"");
            
            // Cleanup
            execute_command("umount -R /mnt");
            std::cout << COLOR_GREEN << "i3 installation completed!" << COLOR_RESET << std::endl;
        } else if (desktop_choice == "10") {
            std::cout << COLOR_CYAN << "Installing Sway (Wayland tiling)..." << COLOR_RESET << std::endl;
            
            // Kernel selection for desktop environments
            std::string selected_kernel = select_kernel();
            std::cout << COLOR_GREEN << "Selected kernel: " << selected_kernel << COLOR_RESET << std::endl;
            
            // Prepare partitions
            prepare_target_partitions(drive, "ext4");
            std::string efi_part = drive + "1";
            std::string root_part = drive + "2";
            
            // Setup filesystem
            setup_ext4_filesystem(root_part);
            
            // Install base system with Sway and selected kernel
            execute_command("pacstrap /mnt base sway swaybg waybar wofi lightdm lightdm-gtk-greeter grub efibootmgr os-prober arch-install-scripts mkinitcpio " + selected_kernel + " linux-firmware sudo");
            
            // Mount EFI partition
            execute_command("mount " + efi_part + " /mnt/boot/efi");
            
            // Install GRUB
            install_grub_ext4(drive);
            
            // Create new user (using new method for desktop environments)
            std::cout << COLOR_CYAN << "Setting up user account..." << COLOR_RESET << std::endl;
            create_new_user(fs_type, drive);
            
            // Enable services
            execute_command("chroot /mnt /bin/bash -c \"systemctl enable lightdm\"");
            
            // Cleanup
            execute_command("umount -R /mnt");
            std::cout << COLOR_GREEN << "Sway installation completed!" << COLOR_RESET << std::endl;
        } else if (desktop_choice == "11") {
            std::cout << COLOR_PURPLE << "Installing Hyprland (Modern Wayland Compositor)..." << COLOR_RESET << std::endl;
            
            // Kernel selection for desktop environments
            std::string selected_kernel = select_kernel();
            std::cout << COLOR_GREEN << "Selected kernel: " << selected_kernel << COLOR_RESET << std::endl;
            
            // Prepare partitions
            prepare_target_partitions(drive, "ext4");
            std::string efi_part = drive + "1";
            std::string root_part = drive + "2";
            
            // Setup filesystem
            setup_ext4_filesystem(root_part);
            
            // Install base system with Hyprland and selected kernel
            execute_command("pacstrap /mnt base hyprland waybar rofi wl-clipboard sddm grub efibootmgr os-prober arch-install-scripts mkinitcpio " + selected_kernel + " linux-firmware sudo");
            
            // Mount EFI partition
            execute_command("mount " + efi_part + " /mnt/boot/efi");
            
            // Install GRUB
            install_grub_ext4(drive);
            
            // Create new user (using new method for desktop environments)
            std::cout << COLOR_CYAN << "Setting up user account..." << COLOR_RESET << std::endl;
            create_new_user(fs_type, drive);
            
            // Enable services
            execute_command("chroot /mnt /bin/bash -c \"systemctl enable sddm\"");
            
            // Cleanup
            execute_command("umount -R /mnt");
            std::cout << COLOR_PURPLE << "Hyprland installed! Note: You may need to configure ~/.config/hypr/hyprland.conf" << COLOR_RESET << std::endl;
        } else if (desktop_choice == "12") {
            std::cout << COLOR_CYAN << "Returning to main menu..." << COLOR_RESET << std::endl;
        } else {
            std::cout << COLOR_RED << "Invalid option. Returning to main menu." << COLOR_RESET << std::endl;
        }

        // Cleanup
        std::cout << COLOR_CYAN << "Cleaning up..." << COLOR_RESET << std::endl;
        execute_command("umount -R /mnt");
    }

    // Function to install Cachyos options
    void install_cachyos_options(const std::string& fs_type, const std::string& drive) {
        std::cout << COLOR_CYAN << "Installing CachyOS..." << COLOR_RESET << std::endl;
        
        // Kernel selection for CachyOS
        std::string selected_kernel = select_kernel();
        std::cout << COLOR_GREEN << "Selected kernel: " << selected_kernel << COLOR_RESET << std::endl;
        
        // Prepare partitions
        prepare_target_partitions(drive, "ext4");
        std::string efi_part = drive + "1";
        std::string root_part = drive + "2";
        
        // Setup filesystem
        setup_ext4_filesystem(root_part);
        
        // Install base system with selected kernel
        execute_command("pacstrap /mnt base grub efibootmgr os-prober arch-install-scripts mkinitcpio " + selected_kernel + " linux-firmware sudo");
        
        // Mount EFI partition
        execute_command("mount " + efi_part + " /mnt/boot/efi");
        
        // Install GRUB
        install_grub_ext4(drive);
        
        // Create new user (using new method for desktop environments)
        std::cout << COLOR_CYAN << "Setting up user account..." << COLOR_RESET << std::endl;
        create_new_user(fs_type, drive);
        
        // Download and setup CachyOS repositories
        execute_command("curl https://mirror.cachyos.org/cachyos-repo.tar.xz -o cachyos-repo.tar.xz");
        execute_command("tar xvf cachyos-repo.tar.xz && cd cachyos-repo");
        execute_command("./cachyos-repo.sh");

        // Check if cachyosmenu.sh exists in current directory
        if (system("test -f cachyosmenu.sh") == 0) {
            std::cout << COLOR_GREEN << "Copying cachyosmenu.sh to chroot..." << COLOR_RESET << std::endl;
            execute_command("cp cachyosmenu.sh /mnt");
            execute_command("chmod +x /mnt/cachyosmenu.sh");
            
            // We need to get the username from somewhere - for now using a placeholder
            std::cout << COLOR_GREEN << "Executing cachyosmenu.sh with username..." << COLOR_RESET << std::endl;
            execute_command("chroot /mnt /bin/bash -c \"/cachyosmenu.sh $new_username\"");
            std::cout << COLOR_GREEN << "Cachyos installation completed!" << COLOR_RESET << std::endl;
        } else {
            std::cout << COLOR_RED << "Error: cachyosmenu.sh not found in current directory" << COLOR_RESET << std::endl;
            std::cout << COLOR_YELLOW << "Please ensure cachyosmenu.sh is in the same directory as this script" << COLOR_RESET << std::endl;
        }

        // Cleanup
        std::cout << COLOR_CYAN << "Cleaning up..." << COLOR_RESET << std::endl;
        execute_command("umount -R /mnt");
    }

    // Function to install claudemods distribution options
    void install_claudemods_distribution(const std::string& fs_type, const std::string& drive) {
        std::cout << COLOR_CYAN << "Installing claudemods distribution..." << COLOR_RESET << std::endl;
        
        // Kernel selection for claudemods distribution
        std::string selected_kernel = select_kernel();
        std::cout << COLOR_GREEN << "Selected kernel: " << selected_kernel << COLOR_RESET << std::endl;
        
        // Prepare partitions
        prepare_target_partitions(drive, "ext4");
        std::string efi_part = drive + "1";
        std::string root_part = drive + "2";
        
        // Setup filesystem
        setup_ext4_filesystem(root_part);
        
        // Install base system with selected kernel
        execute_command("pacstrap /mnt base grub efibootmgr os-prober arch-install-scripts mkinitcpio " + selected_kernel + " linux-firmware sudo");
        
        // Mount EFI partition
        execute_command("mount " + efi_part + " /mnt/boot/efi");
        
        // Install GRUB
        install_grub_ext4(drive);
        
        // Create new user (using new method for desktop environments)
        std::cout << COLOR_CYAN << "Setting up user account..." << COLOR_RESET << std::endl;
        create_new_user(fs_type, drive);

        // Check if claudemods-distributions.sh exists in current directory
        if (system("test -f claudemods-distributions.sh") == 0) {
            std::cout << COLOR_GREEN << "Copying claudemods-distributions.sh to chroot..." << COLOR_RESET << std::endl;
            execute_command("cp claudemods-distributions.sh /mnt");
            execute_command("chmod +x /mnt/claudemods-distributions.sh");
            std::cout << COLOR_GREEN << "Executing claudemods-distributions.sh with username..." << COLOR_RESET << std::endl;
            execute_command("chroot /mnt /bin/bash -c \"/claudemods-distributions.sh $new_username\"");
            std::cout << COLOR_GREEN << "Claudemods distribution installation completed!" << COLOR_RESET << std::endl;
        } else {
            std::cout << COLOR_RED << "Error: claudemods-distributions.sh not found in current directory" << COLOR_RESET << std::endl;
            std::cout << COLOR_YELLOW << "Please ensure claudemods-distributions.sh is in the same directory as this script" << COLOR_RESET << std::endl;
        }

        // Cleanup
        std::cout << COLOR_CYAN << "Cleaning up..." << COLOR_RESET << std::endl;
        execute_command("umount -R /mnt");
    }

    // Function to display main menu
    void main_menu(const std::string& fs_type, const std::string& drive) {
        while (true) {
            std::cout << COLOR_CYAN;
            std::cout << "╔══════════════════════════════════════╗" << std::endl;
            std::cout << "║              Main Menu               ║" << std::endl;
            std::cout << "╠══════════════════════════════════════╣" << std::endl;
            std::cout << "║ 1. Install Vanilla Arch Desktop      ║" << std::endl;
            std::cout << "║ 2. Vanilla Cachyos Options           ║" << std::endl;
            std::cout << "║ 3. Claudemods Distribution Options   ║" << std::endl;
            std::cout << "║ 4. Reboot System                     ║" << std::endl;
            std::cout << "║ 5. Exit                              ║" << std::endl;
            std::cout << "╚══════════════════════════════════════╝" << std::endl;
            std::cout << COLOR_RESET;

            std::cout << COLOR_CYAN << "Select an option (1-5): " << COLOR_RESET;
            std::string choice;
            std::getline(std::cin, choice);

            if (choice == "1") {
                install_desktop(fs_type, drive);
            } else if (choice == "2") {
                install_cachyos_options(fs_type, drive);
            } else if (choice == "3") {
                install_claudemods_distribution(fs_type, drive);
            } else if (choice == "4") {
                std::cout << COLOR_GREEN << "Rebooting system..." << COLOR_RESET << std::endl;
                execute_command("umount -R /mnt 2>/dev/null || true");
                execute_command("reboot");
            } else if (choice == "5") {
                std::cout << COLOR_GREEN << "Exiting. Goodbye!" << COLOR_RESET << std::endl;
                exit(0);
            } else {
                std::cout << COLOR_RED << "Invalid option. Please try again." << COLOR_RESET << std::endl;
            }

            std::cout << std::endl;
            std::cout << COLOR_YELLOW << "Press Enter to continue..." << COLOR_RESET;
            std::cin.ignore(); // Clear the buffer
            std::getline(std::cin, choice); // Wait for Enter
        }
    }

public:
    // Main script
    void run() {
        display_header();
        display_available_drives();

        std::string drive;
        std::cout << COLOR_CYAN << "Enter target drive (e.g., /dev/sda): " << COLOR_RESET;
        std::getline(std::cin, drive);
        if (!is_block_device(drive)) {
            std::cerr << COLOR_RED << "Error: " << drive << " is not a valid block device" << COLOR_RESET << std::endl;
            exit(1);
        }

        std::string fs_type;
        std::cout << COLOR_CYAN << "Choose filesystem type (ext4/btrfs): " << COLOR_RESET;
        std::getline(std::cin, fs_type);

        if (fs_type == "btrfs") {
            std::cout << COLOR_CYAN << "Executing btrfsrsync.sh with drive: " << drive << COLOR_RESET << std::endl;
            execute_command("./btrfsrsync.sh " + drive);
            std::cout << COLOR_GREEN << "Btrfs installation complete!" << COLOR_RESET << std::endl;
            exit(0);
        }

        // Show main menu for ext4
        main_menu(fs_type, drive);
    }
};

int main(int argc, char* argv[]) {
    ArchInstaller installer;
    installer.run();
    return 0;
}
