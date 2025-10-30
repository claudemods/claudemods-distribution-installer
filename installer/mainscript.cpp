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

        std::cout << COLOR_CYAN;
        system("sudo lsblk -o NAME,SIZE,TYPE,MOUNTPOINT,FSTYPE,MODEL | grep -v \"loop\"");
        
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
        std::cout << COLOR_CYAN << "claudemods distribution installer v1.0 29-10-2025" << COLOR_RESET << std::endl;
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

    // Function to setup timezone and keyboard after username
    void setup_timezone_keyboard(const std::string& fs_type, const std::string& drive) {
        // Timezone setup
        std::cout << COLOR_CYAN;
        std::cout << "╔══════════════════════════════════════════════════════════════╗" << std::endl;
        std::cout << "║                      Timezone Setup                          ║" << std::endl;
        std::cout << "╠══════════════════════════════════════════════════════════════╣" << std::endl;
        std::cout << "║  1. Europe/London (UK)                                      ║" << std::endl;
        std::cout << "║  2. America/New_York (Eastern)                              ║" << std::endl;
        std::cout << "║  3. America/Los_Angeles (Pacific)                           ║" << std::endl;
        std::cout << "║  4. Europe/Paris (Central European)                         ║" << std::endl;
        std::cout << "║  5. Asia/Tokyo (Japan)                                      ║" << std::endl;
        std::cout << "║  6. Australia/Sydney                                        ║" << std::endl;
        std::cout << "║  7. Other (manual entry)                                    ║" << std::endl;
        std::cout << "╚══════════════════════════════════════════════════════════════╝" << std::endl;
        std::cout << COLOR_RESET;

        std::string timezone_choice;
        std::string timezone;

        std::cout << COLOR_CYAN << "Select timezone (1-7): " << COLOR_RESET;
        std::getline(std::cin, timezone_choice);

        if (timezone_choice == "1") {
            timezone = "Europe/London";
        } else if (timezone_choice == "2") {
            timezone = "America/New_York";
        } else if (timezone_choice == "3") {
            timezone = "America/Los_Angeles";
        } else if (timezone_choice == "4") {
            timezone = "Europe/Paris";
        } else if (timezone_choice == "5") {
            timezone = "Asia/Tokyo";
        } else if (timezone_choice == "6") {
            timezone = "Australia/Sydney";
        } else if (timezone_choice == "7") {
            std::cout << COLOR_CYAN << "Enter timezone (e.g., Europe/Berlin): " << COLOR_RESET;
            std::getline(std::cin, timezone);
        } else {
            std::cout << COLOR_RED << "Invalid selection. Using default: Europe/London" << COLOR_RESET << std::endl;
            timezone = "Europe/London";
        }

        std::cout << COLOR_CYAN << "Setting timezone to: " << timezone << COLOR_RESET << std::endl;
        execute_command("chroot /mnt /bin/bash -c \"ln -sf /usr/share/zoneinfo/" + timezone + " /etc/localtime\"");
        execute_command("chroot /mnt /bin/bash -c \"hwclock --systohc\"");

        // Keyboard layout setup
        std::cout << COLOR_CYAN;
        std::cout << "╔══════════════════════════════════════════════════════════════╗" << std::endl;
        std::cout << "║                    Keyboard Layout Setup                     ║" << std::endl;
        std::cout << "╠══════════════════════════════════════════════════════════════╣" << std::endl;
        std::cout << "║  1. us (US English)                                         ║" << std::endl;
        std::cout << "║  2. uk (UK English)                                         ║" << std::endl;
        std::cout << "║  3. de (German)                                             ║" << std::endl;
        std::cout << "║  4. fr (French)                                             ║" << std::endl;
        std::cout << "║  5. es (Spanish)                                            ║" << std::endl;
        std::cout << "║  6. it (Italian)                                            ║" << std::endl;
        std::cout << "║  7. jp (Japanese)                                           ║" << std::endl;
        std::cout << "║  8. Other (manual entry)                                    ║" << std::endl;
        std::cout << "╚══════════════════════════════════════════════════════════════╝" << std::endl;
        std::cout << COLOR_RESET;

        std::string keyboard_choice;
        std::string keyboard_layout;

        std::cout << COLOR_CYAN << "Select keyboard layout (1-8): " << COLOR_RESET;
        std::getline(std::cin, keyboard_choice);

        if (keyboard_choice == "1") {
            keyboard_layout = "us";
        } else if (keyboard_choice == "2") {
            keyboard_layout = "uk";
        } else if (keyboard_choice == "3") {
            keyboard_layout = "de";
        } else if (keyboard_choice == "4") {
            keyboard_layout = "fr";
        } else if (keyboard_choice == "5") {
            keyboard_layout = "es";
        } else if (keyboard_choice == "6") {
            keyboard_layout = "it";
        } else if (keyboard_choice == "7") {
            keyboard_layout = "jp";
        } else if (keyboard_choice == "8") {
            std::cout << COLOR_CYAN << "Enter keyboard layout (e.g., br, ru, pt): " << COLOR_RESET;
            std::getline(std::cin, keyboard_layout);
        } else {
            std::cout << COLOR_RED << "Invalid selection. Using default: us" << COLOR_RESET << std::endl;
            keyboard_layout = "us";
        }

        std::cout << COLOR_CYAN << "Setting keyboard layout to: " << keyboard_layout << COLOR_RESET << std::endl;
        execute_command("chroot /mnt /bin/bash -c \"echo 'KEYMAP=" + keyboard_layout + "' > /etc/vconsole.conf\"");
        execute_command("chroot /mnt /bin/bash -c \"echo 'LANG=en_US.UTF-8' > /etc/locale.conf\"");
        execute_command("chroot /mnt /bin/bash -c \"echo 'en_US.UTF-8 UTF-8' >> /etc/locale.gen\"");
        execute_command("chroot /mnt /bin/bash -c \"locale-gen\"");
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

        execute_command("chroot /mnt /bin/bash -c \"usermod -l " + new_username + " arch\"");
        execute_command("chroot /mnt /bin/bash -c \"mv /home/arch /home/" + new_username + "\"");
        execute_command("chroot /mnt /bin/bash -c \"usermod -d /home/" + new_username + " " + new_username + "\"");
        execute_command("chroot /mnt /bin/bash -c \"groupmod -n " + new_username + " arch\"");

        std::cout << COLOR_CYAN << "Adding " << new_username << " to sudo group..." << COLOR_RESET << std::endl;
        execute_command("chroot /mnt /bin/bash -c \"gpasswd -a " + new_username + " wheel\"");

        std::cout << COLOR_CYAN << "Setting password for user 'root'..." << COLOR_RESET << std::endl;
        execute_command("chroot /mnt /bin/bash -c \"passwd root\"");
        
        std::cout << COLOR_CYAN << "Setting password for user '" << new_username << "'..." << COLOR_RESET << std::endl;
        execute_command("chroot /mnt /bin/bash -c \"passwd " + new_username + "\"");
        
        execute_command("chroot /mnt /bin/bash -c \"echo '%wheel ALL=(ALL:ALL) ALL' | tee -a /etc/sudoers\"");

        // TIMEZONE AND KEYBOARD SETUP AFTER USERNAME
        setup_timezone_keyboard(fs_type, drive);

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

        execute_command("chroot /mnt /bin/bash -c \"useradd -m -G wheel -s /bin/bash " + new_username + "\"");
        
        std::cout << COLOR_CYAN << "Setting password for user 'root'..." << COLOR_RESET << std::endl;
        execute_command("chroot /mnt /bin/bash -c \"passwd root\"");
        
        std::cout << COLOR_CYAN << "Setting password for user '" << new_username << "'..." << COLOR_RESET << std::endl;
        execute_command("chroot /mnt /bin/bash -c \"passwd " + new_username + "\"");

        execute_command("chroot /mnt /bin/bash -c \"echo '%wheel ALL=(ALL:ALL) ALL' | tee -a /etc/sudoers\"");

        // TIMEZONE AND KEYBOARD SETUP AFTER USERNAME
        setup_timezone_keyboard(fs_type, drive);

        execute_command("umount -R /mnt");

        std::cout << COLOR_GREEN << "User '" << new_username << "' created successfully with sudo privileges" << COLOR_RESET << std::endl;
        
        return new_username;
    }

    // Function to prompt for reboot
    void prompt_reboot() {
        std::cout << COLOR_CYAN << "Installation completed successfully! Would you like to reboot now? (yes/no): " << COLOR_RESET;
        std::string reboot_choice;
        std::getline(std::cin, reboot_choice);
        
        if (reboot_choice == "yes" || reboot_choice == "y" || reboot_choice == "Y") {
            std::cout << COLOR_GREEN << "Rebooting system..." << COLOR_RESET << std::endl;
            execute_command("sudo reboot");
        } else {
            std::cout << COLOR_YELLOW << "You can reboot manually later using: sudo reboot" << COLOR_RESET << std::endl;
        }
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
        
        std::string selected_kernel = select_kernel();
        std::cout << COLOR_GREEN << "Selected kernel: " << selected_kernel << COLOR_RESET << std::endl;
        
        prepare_target_partitions(drive, fs_type);
        
        std::string efi_part = drive + "1";
        std::string root_part = drive + "2";
        
        setup_ext4_filesystem(root_part);
        
        execute_command("pacstrap /mnt base " + selected_kernel + " linux-firmware grub efibootmgr os-prober sudo arch-install-scripts mkinitcpio vim nano bash-completion networkmanager");
        
        execute_command("mount " + efi_part + " /mnt/boot/efi");
        
        install_grub_ext4(drive);
        
        std::string new_username = create_new_user(fs_type, drive);
        
        std::cout << COLOR_GREEN << "Arch TTY Grub installation completed successfully!" << COLOR_RESET << std::endl;
        
        prompt_reboot();
    }

    // Function to install desktop environments
    void install_desktop(const std::string& fs_type, const std::string& drive) {
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
            install_arch_tty_grub(drive);
        } else if (desktop_choice == "2") {
            std::cout << COLOR_CYAN << "Installing GNOME Desktop..." << COLOR_RESET << std::endl;
            
            std::string selected_kernel = select_kernel();
            std::cout << COLOR_GREEN << "Selected kernel: " << selected_kernel << COLOR_RESET << std::endl;
            
            prepare_target_partitions(drive, "ext4");
            std::string efi_part = drive + "1";
            std::string root_part = drive + "2";
            
            setup_ext4_filesystem(root_part);
            
            execute_command("pacstrap /mnt base gnome gnome-extra gdm grub efibootmgr os-prober arch-install-scripts mkinitcpio " + selected_kernel + " linux-firmware sudo networkmanager");
            
            execute_command("chroot /mnt /bin/bash -c \"systemctl enable gdm\"");
            execute_command("chroot /mnt /bin/bash -c \"systemctl enable NetworkManager\"");
            
            execute_command("mount " + efi_part + " /mnt/boot/efi");
            
            install_grub_ext4(drive);
            
            std::string new_username = create_new_user(fs_type, drive);
            
            std::cout << COLOR_GREEN << "GNOME installation completed!" << COLOR_RESET << std::endl;
            
            prompt_reboot();
        } else if (desktop_choice == "3") {
            std::cout << COLOR_CYAN << "Installing KDE Plasma..." << COLOR_RESET << std::endl;
            
            std::string selected_kernel = select_kernel();
            std::cout << COLOR_GREEN << "Selected kernel: " << selected_kernel << COLOR_RESET << std::endl;
            
            prepare_target_partitions(drive, "ext4");
            std::string efi_part = drive + "1";
            std::string root_part = drive + "2";
            
            setup_ext4_filesystem(root_part);
            
            execute_command("pacstrap /mnt base plasma sddm dolphin konsole grub efibootmgr os-prober arch-install-scripts mkinitcpio " + selected_kernel + " linux-firmware sudo networkmanager");
            
            execute_command("chroot /mnt /bin/bash -c \"systemctl enable sddm\"");
            execute_command("chroot /mnt /bin/bash -c \"systemctl enable NetworkManager\"");
            
            execute_command("mount " + efi_part + " /mnt/boot/efi");
            
            install_grub_ext4(drive);
            
            std::string new_username = create_new_user(fs_type, drive);
            
            std::cout << COLOR_GREEN << "KDE Plasma installation completed!" << COLOR_RESET << std::endl;
            
            prompt_reboot();
        } else if (desktop_choice == "4") {
            std::cout << COLOR_CYAN << "Installing XFCE..." << COLOR_RESET << std::endl;
            
            std::string selected_kernel = select_kernel();
            std::cout << COLOR_GREEN << "Selected kernel: " << selected_kernel << COLOR_RESET << std::endl;
            
            prepare_target_partitions(drive, "ext4");
            std::string efi_part = drive + "1";
            std::string root_part = drive + "2";
            
            setup_ext4_filesystem(root_part);
            
            execute_command("pacstrap /mnt base xfce4 xfce4-goodies lightdm lightdm-gtk-greeter grub efibootmgr os-prober arch-install-scripts mkinitcpio " + selected_kernel + " linux-firmware sudo networkmanager");
            
            execute_command("chroot /mnt /bin/bash -c \"systemctl enable lightdm\"");
            execute_command("chroot /mnt /bin/bash -c \"systemctl enable NetworkManager\"");
            
            execute_command("mount " + efi_part + " /mnt/boot/efi");
            
            install_grub_ext4(drive);
            
            std::string new_username = create_new_user(fs_type, drive);
            
            std::cout << COLOR_GREEN << "XFCE installation completed!" << COLOR_RESET << std::endl;
            
            prompt_reboot();
        } else if (desktop_choice == "5") {
            std::cout << COLOR_CYAN << "Installing LXQt..." << COLOR_RESET << std::endl;
            
            std::string selected_kernel = select_kernel();
            std::cout << COLOR_GREEN << "Selected kernel: " << selected_kernel << COLOR_RESET << std::endl;
            
            prepare_target_partitions(drive, "ext4");
            std::string efi_part = drive + "1";
            std::string root_part = drive + "2";
            
            setup_ext4_filesystem(root_part);
            
            execute_command("pacstrap /mnt base lxqt sddm grub efibootmgr os-prober arch-install-scripts mkinitcpio " + selected_kernel + " linux-firmware sudo networkmanager");
            
            execute_command("chroot /mnt /bin/bash -c \"systemctl enable sddm\"");
            execute_command("chroot /mnt /bin/bash -c \"systemctl enable NetworkManager\"");
            
            execute_command("mount " + efi_part + " /mnt/boot/efi");
            
            install_grub_ext4(drive);
            
            std::string new_username = create_new_user(fs_type, drive);
            
            std::cout << COLOR_GREEN << "LXQt installation completed!" << COLOR_RESET << std::endl;
            
            prompt_reboot();
        } else if (desktop_choice == "6") {
            std::cout << COLOR_CYAN << "Installing Cinnamon..." << COLOR_RESET << std::endl;
            
            std::string selected_kernel = select_kernel();
            std::cout << COLOR_GREEN << "Selected kernel: " << selected_kernel << COLOR_RESET << std::endl;
            
            prepare_target_partitions(drive, "ext4");
            std::string efi_part = drive + "1";
            std::string root_part = drive + "2";
            
            setup_ext4_filesystem(root_part);
            
            execute_command("pacstrap /mnt base cinnamon lightdm lightdm-gtk-greeter grub efibootmgr os-prober arch-install-scripts mkinitcpio " + selected_kernel + " linux-firmware sudo networkmanager");
            
            execute_command("chroot /mnt /bin/bash -c \"systemctl enable lightdm\"");
            execute_command("chroot /mnt /bin/bash -c \"systemctl enable NetworkManager\"");
            
            execute_command("mount " + efi_part + " /mnt/boot/efi");
            
            install_grub_ext4(drive);
            
            std::string new_username = create_new_user(fs_type, drive);
            
            std::cout << COLOR_GREEN << "Cinnamon installation completed!" << COLOR_RESET << std::endl;
            
            prompt_reboot();
        } else if (desktop_choice == "7") {
            std::cout << COLOR_CYAN << "Installing MATE..." << COLOR_RESET << std::endl;
            
            std::string selected_kernel = select_kernel();
            std::cout << COLOR_GREEN << "Selected kernel: " << selected_kernel << COLOR_RESET << std::endl;
            
            prepare_target_partitions(drive, "ext4");
            std::string efi_part = drive + "1";
            std::string root_part = drive + "2";
            
            setup_ext4_filesystem(root_part);
            
            execute_command("pacstrap /mnt base mate mate-extra lightdm lightdm-gtk-greeter grub efibootmgr os-prober arch-install-scripts mkinitcpio " + selected_kernel + " linux-firmware sudo networkmanager");
            
            execute_command("chroot /mnt /bin/bash -c \"systemctl enable lightdm\"");
            execute_command("chroot /mnt /bin/bash -c \"systemctl enable NetworkManager\"");
            
            
            execute_command("mount " + efi_part + " /mnt/boot/efi");
            
            install_grub_ext4(drive);
            
            std::string new_username = create_new_user(fs_type, drive);
            
            std::cout << COLOR_GREEN << "MATE installation completed!" << COLOR_RESET << std::endl;
            
            prompt_reboot();
        } else if (desktop_choice == "8") {
            std::cout << COLOR_CYAN << "Installing Budgie..." << COLOR_RESET << std::endl;
            
            std::string selected_kernel = select_kernel();
            std::cout << COLOR_GREEN << "Selected kernel: " << selected_kernel << COLOR_RESET << std::endl;
            
            prepare_target_partitions(drive, "ext4");
            std::string efi_part = drive + "1";
            std::string root_part = drive + "2";
            
            setup_ext4_filesystem(root_part);
            
            execute_command("pacstrap /mnt base budgie-desktop lightdm lightdm-gtk-greeter grub efibootmgr os-prober arch-install-scripts mkinitcpio " + selected_kernel + " linux-firmware sudo networkmanager");
            
            execute_command("chroot /mnt /bin/bash -c \"systemctl enable lightdm\"");
            execute_command("chroot /mnt /bin/bash -c \"systemctl enable NetworkManager\"");
            
            execute_command("mount " + efi_part + " /mnt/boot/efi");
            
            install_grub_ext4(drive);
            
            std::string new_username = create_new_user(fs_type, drive);
            
            std::cout << COLOR_GREEN << "Budgie installation completed!" << COLOR_RESET << std::endl;
            
            prompt_reboot();
        } else if (desktop_choice == "9") {
            std::cout << COLOR_CYAN << "Installing i3 (tiling WM)..." << COLOR_RESET << std::endl;
            
            std::string selected_kernel = select_kernel();
            std::cout << COLOR_GREEN << "Selected kernel: " << selected_kernel << COLOR_RESET << std::endl;
            
            prepare_target_partitions(drive, "ext4");
            std::string efi_part = drive + "1";
            std::string root_part = drive + "2";
            
            setup_ext4_filesystem(root_part);
            
            execute_command("pacstrap /mnt base i3-wm i3status i3lock dmenu lightdm lightdm-gtk-greeter grub efibootmgr os-prober arch-install-scripts mkinitcpio " + selected_kernel + " linux-firmware sudo networkmanager");
            
            execute_command("chroot /mnt /bin/bash -c \"systemctl enable lightdm\"");
            execute_command("chroot /mnt /bin/bash -c \"systemctl enable NetworkManager\"");
            
            execute_command("mount " + efi_part + " /mnt/boot/efi");
            
            install_grub_ext4(drive);
            
            std::string new_username = create_new_user(fs_type, drive);
            
            std::cout << COLOR_GREEN << "i3 installation completed!" << COLOR_RESET << std::endl;
            
            prompt_reboot();
        } else if (desktop_choice == "10") {
            std::cout << COLOR_CYAN << "Installing Sway (Wayland tiling)..." << COLOR_RESET << std::endl;
            
            std::string selected_kernel = select_kernel();
            std::cout << COLOR_GREEN << "Selected kernel: " << selected_kernel << COLOR_RESET << std::endl;
            
            prepare_target_partitions(drive, "ext4");
            std::string efi_part = drive + "1";
            std::string root_part = drive + "2";
            
            setup_ext4_filesystem(root_part);
            
            execute_command("pacstrap /mnt base sway swaybg waybar wofi lightdm lightdm-gtk-greeter grub efibootmgr os-prober arch-install-scripts mkinitcpio " + selected_kernel + " linux-firmware sudo networkmanager");
            
            execute_command("chroot /mnt /bin/bash -c \"systemctl enable lightdm\"");
            execute_command("chroot /mnt /bin/bash -c \"systemctl enable NetworkManager\"");
            
            execute_command("mount " + efi_part + " /mnt/boot/efi");
            
            install_grub_ext4(drive);
            
            std::string new_username = create_new_user(fs_type, drive);
            
            std::cout << COLOR_GREEN << "Sway installation completed!" << COLOR_RESET << std::endl;
            
            prompt_reboot();
        } else if (desktop_choice == "11") {
            std::cout << COLOR_PURPLE << "Installing Hyprland (Modern Wayland Compositor)..." << COLOR_RESET << std::endl;
            
            std::string selected_kernel = select_kernel();
            std::cout << COLOR_GREEN << "Selected kernel: " << selected_kernel << COLOR_RESET << std::endl;
            
            prepare_target_partitions(drive, "ext4");
            std::string efi_part = drive + "1";
            std::string root_part = drive + "2";
            
            setup_ext4_filesystem(root_part);
            
            execute_command("pacstrap /mnt base hyprland waybar rofi wl-clipboard sddm grub efibootmgr os-prober arch-install-scripts mkinitcpio " + selected_kernel + " linux-firmware sudo networkmanager");
            
            execute_command("chroot /mnt /bin/bash -c \"systemctl enable sddm\"");
            execute_command("chroot /mnt /bin/bash -c \"systemctl enable NetworkManager\"");
            
            execute_command("mount " + efi_part + " /mnt/boot/efi");
            
            install_grub_ext4(drive);
            
            std::string new_username = create_new_user(fs_type, drive);
            
            std::cout << COLOR_PURPLE << "Hyprland installed! Note: You may need to configure ~/.config/hypr/hyprland.conf" << COLOR_RESET << std::endl;
            
            prompt_reboot();
        } else if (desktop_choice == "12") {
            std::cout << COLOR_CYAN << "Returning to main menu..." << COLOR_RESET << std::endl;
        } else {
            std::cout << COLOR_RED << "Invalid option. Returning to main menu." << COLOR_RESET << std::endl;
        }
    }

    // Function to install CachyOS TTY Grub
    void install_cachyos_tty_grub(const std::string& drive) {
        std::cout << COLOR_CYAN << "Installing CachyOS TTY Grub..." << COLOR_RESET << std::endl;
        
        std::string selected_kernel = select_kernel();
        std::cout << COLOR_GREEN << "Selected kernel: " << selected_kernel << COLOR_RESET << std::endl;
        
        prepare_target_partitions(drive, "ext4");
        std::string efi_part = drive + "1";
        std::string root_part = drive + "2";
        
        setup_ext4_filesystem(root_part);
        
        execute_command("pacstrap /mnt base grub efibootmgr os-prober arch-install-scripts mkinitcpio " + selected_kernel + " linux-firmware sudo networkmanager");
        
        execute_command("mount " + efi_part + " /mnt/boot/efi");
        
        install_grub_ext4(drive);
        
        std::string new_username = create_new_user("ext4", drive);
        
        execute_command("umount -R /mnt");

        std::cout << COLOR_GREEN << "CachyOS TTY Grub installation completed!" << COLOR_RESET << std::endl;
        
        prompt_reboot();
    }

    // Function to install CachyOS KDE
    void install_cachyos_kde(const std::string& drive) {
        std::cout << COLOR_CYAN << "Installing CachyOS KDE..." << COLOR_RESET << std::endl;
        
        std::string selected_kernel = select_kernel();
        std::cout << COLOR_GREEN << "Selected kernel: " << selected_kernel << COLOR_RESET << std::endl;
        
        prepare_target_partitions(drive, "ext4");
        std::string efi_part = drive + "1";
        std::string root_part = drive + "2";
        
        setup_ext4_filesystem(root_part);
        
        execute_command("pacstrap /mnt base plasma sddm dolphin konsole grub efibootmgr os-prober arch-install-scripts mkinitcpio " + selected_kernel + " linux-firmware sudo networkmanager");
        
        execute_command("chroot /mnt /bin/bash -c \"systemctl enable sddm\"");
        execute_command("chroot /mnt /bin/bash -c \"systemctl enable NetworkManager\"");
        
        execute_command("mount " + efi_part + " /mnt/boot/efi");
        
        install_grub_ext4(drive);
        
        std::string new_username = create_new_user("ext4", drive);
        
        std::cout << COLOR_CYAN << "Setting up CachyOS..." << COLOR_RESET << std::endl;
        execute_command("mkdir /mnt/home/" + new_username + "/.config/autostart");
        execute_command("cp -r /opt/claudemods-distribution-installer /mnt/opt");
        execute_command("cp -r /opt/claudemods-distribution-installer/install-fullkde-grub/cachyoskdebgrub.desktop /mnt/home/" + new_username + "/.config/autostart");
        
        execute_command("umount -R /mnt");

        std::cout << COLOR_GREEN << "CachyOS KDE Part 1 installation completed!" << COLOR_RESET << std::endl;
        std::cout << COLOR_GREEN << " For CachyOS KDE Part 2 installation Please Reboot And login To Run Next Script!" << COLOR_RESET << std::endl;
        
        prompt_reboot();
    }

    // Function to install CachyOS GNOME
    void install_cachyos_gnome(const std::string& drive) {
        std::cout << COLOR_CYAN << "Installing CachyOS GNOME..." << COLOR_RESET << std::endl;
        
        std::string selected_kernel = select_kernel();
        std::cout << COLOR_GREEN << "Selected kernel: " << selected_kernel << COLOR_RESET << std::endl;
        
        prepare_target_partitions(drive, "ext4");
        std::string efi_part = drive + "1";
        std::string root_part = drive + "2";
        
        setup_ext4_filesystem(root_part);
        
        execute_command("pacstrap /mnt base gnome gnome-extra gdm grub efibootmgr os-prober arch-install-scripts mkinitcpio " + selected_kernel + " linux-firmware sudo networkmanager");
        
        execute_command("chroot /mnt /bin/bash -c \"systemctl enable gdm\"");
        execute_command("chroot /mnt /bin/bash -c \"systemctl enable NetworkManager\"");
        
        execute_command("mount " + efi_part + " /mnt/boot/efi");
        
        install_grub_ext4(drive);
        
        std::string new_username = create_new_user("ext4", drive);
        
        std::cout << COLOR_CYAN << "Setting up CachyOS..." << COLOR_RESET << std::endl;
        
        execute_command("umount -R /mnt");

        std::cout << COLOR_GREEN << "CachyOS GNOME installation completed!" << COLOR_RESET << std::endl;
        
        prompt_reboot();
    }

    // Function to display Cachyos menu
    void display_cachyos_menu(const std::string& fs_type, const std::string& drive) {
        while (true) {
            std::cout << COLOR_CYAN;
            std::cout << "╔══════════════════════════════════════════════════════════════╗" << std::endl;
            std::cout << "║                    CachyOS Options                          ║" << std::endl;
            std::cout << "╠══════════════════════════════════════════════════════════════╣" << std::endl;
            std::cout << "║  1. Install CachyOS TTY Grub                               ║" << std::endl;
            std::cout << "║  2. Install CachyOS KDE Grub                               ║" << std::endl;
            std::cout << "║  3. Install CachyOS GNOME Grub                             ║" << std::endl;
            std::cout << "║  4. Return to Main Menu                                    ║" << std::endl;
            std::cout << "╚══════════════════════════════════════════════════════════════╝" << std::endl;
            std::cout << COLOR_RESET;

            std::cout << COLOR_CYAN << "Select CachyOS option (1-4): " << COLOR_RESET;
            std::string cachyos_choice;
            std::getline(std::cin, cachyos_choice);

            if (cachyos_choice == "1") {
                install_cachyos_tty_grub(drive);
            } else if (cachyos_choice == "2") {
                install_cachyos_kde(drive);
            } else if (cachyos_choice == "3") {
                install_cachyos_gnome(drive);
            } else if (cachyos_choice == "4") {
                std::cout << COLOR_CYAN << "Returning to main menu..." << COLOR_RESET << std::endl;
                break;
            } else {
                std::cout << COLOR_RED << "Invalid option. Please try again." << COLOR_RESET << std::endl;
            }
        }
    }

    // Function to install Spitfire CKGE
    void install_spitfire_ckge(const std::string& drive) {
        std::cout << COLOR_ORANGE << "Installing Spitfire CKGE..." << COLOR_RESET << std::endl;
        
        std::string selected_kernel = select_kernel();
        std::cout << COLOR_GREEN << "Selected kernel: " << selected_kernel << COLOR_RESET << std::endl;
        
        prepare_target_partitions(drive, "ext4");
        std::string efi_part = drive + "1";
        std::string root_part = drive + "2";
        
        setup_ext4_filesystem(root_part);
        
        execute_command("pacstrap /mnt base plasma sddm dolphin konsole grub efibootmgr os-prober arch-install-scripts mkinitcpio " + selected_kernel + " linux-firmware sudo networkmanager");
        
        execute_command("chroot /mnt /bin/bash -c \"systemctl enable sddm\"");
        execute_command("chroot /mnt /bin/bash -c \"systemctl enable NetworkManager\"");
        
        execute_command("mount " + efi_part + " /mnt/boot/efi");
        
        install_grub_ext4(drive);
        
        std::string new_username = create_new_user("ext4", drive);
        
        std::cout << COLOR_ORANGE << "Setting up Spitfire CKGE repositories..." << COLOR_RESET << std::endl;
        
        
        execute_command("umount -R /mnt");

        std::cout << COLOR_ORANGE << "Spitfire CKGE installation completed!" << COLOR_RESET << std::endl;
        
        prompt_reboot();
    }

    // Function to install Apex CKGE
    void install_apex_ckge(const std::string& drive) {
        std::cout << COLOR_PURPLE << "Installing Apex CKGE..." << COLOR_RESET << std::endl;
        
        std::string selected_kernel = select_kernel();
        std::cout << COLOR_GREEN << "Selected kernel: " << selected_kernel << COLOR_RESET << std::endl;
        
        prepare_target_partitions(drive, "ext4");
        std::string efi_part = drive + "1";
        std::string root_part = drive + "2";
        
        setup_ext4_filesystem(root_part);
        
        execute_command("pacstrap /mnt base plasma sddm dolphin konsole grub efibootmgr os-prober arch-install-scripts mkinitcpio " + selected_kernel + " linux-firmware sudo networkmanager");
        
        execute_command("chroot /mnt /bin/bash -c \"systemctl enable sddm\"");
        execute_command("chroot /mnt /bin/bash -c \"systemctl enable NetworkManager\"");
        
        execute_command("mount " + efi_part + " /mnt/boot/efi");
        
        install_grub_ext4(drive);
        
        std::string new_username = create_new_user("ext4", drive);
        
        std::cout << COLOR_PURPLE << "Setting up Apex CKGE repositories..." << COLOR_RESET << std::endl;
      
        
        execute_command("umount -R /mnt");

        std::cout << COLOR_PURPLE << "Apex CKGE installation completed!" << COLOR_RESET << std::endl;
        
        prompt_reboot();
    }

    // Function to display Claudemods Distribution menu
    void display_claudemods_menu(const std::string& fs_type, const std::string& drive) {
        while (true) {
            std::cout << COLOR_CYAN;
            std::cout << "╔══════════════════════════════════════════════════════════════╗" << std::endl;
            std::cout << "║               Claudemods Distribution Options               ║" << std::endl;
            std::cout << "╠══════════════════════════════════════════════════════════════╣" << std::endl;
            std::cout << "║  1. Install Spitfire CKGE                                  ║" << std::endl;
            std::cout << "║  2. Install Apex CKGE                                      ║" << std::endl;
            std::cout << "║  3. Return to Main Menu                                    ║" << std::endl;
            std::cout << "╚══════════════════════════════════════════════════════════════╝" << std::endl;
            std::cout << COLOR_RESET;

            std::cout << COLOR_CYAN << "Select Claudemods option (1-3): " << COLOR_RESET;
            std::string claudemods_choice;
            std::getline(std::cin, claudemods_choice);

            if (claudemods_choice == "1") {
                install_spitfire_ckge(drive);
            } else if (claudemods_choice == "2") {
                install_apex_ckge(drive);
            } else if (claudemods_choice == "3") {
                std::cout << COLOR_CYAN << "Returning to main menu..." << COLOR_RESET << std::endl;
                break;
            } else {
                std::cout << COLOR_RED << "Invalid option. Please try again." << COLOR_RESET << std::endl;
            }
        }
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
                display_cachyos_menu(fs_type, drive);
            } else if (choice == "3") {
                display_claudemods_menu(fs_type, drive);
            } else if (choice == "4") {
                std::cout << COLOR_GREEN << "Rebooting system..." << COLOR_RESET << std::endl;
                execute_command("sudo reboot");
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
