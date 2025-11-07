#ifndef BTRFSINSTALLER_H
#define BTRFSINSTALLER_H

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

class BtrfsInstaller {
private:
    // Store user inputs for use during installation
    std::string selected_drive;
    std::string fs_type;
    std::string selected_kernel;
    std::string new_username;
    std::string timezone;
    std::string keyboard_layout;
    std::string root_password;
    std::string user_password;

    // Function to execute commands with error handling
    int execute_command(const std::string& cmd) {
        std::cout << "\033[38;2;0;255;255m";
        std::string full_cmd = "sudo " + cmd;
        int status = system(full_cmd.c_str());
        std::cout << "\033[0m";
        if (status != 0) {
            std::cerr << "\033[31m" << "Error executing: " << full_cmd << "\033[0m" << std::endl;
            exit(1);
        }
        return status;
    }

// Special function for CD commands only
    int execute_cd_command(const std::string& cmd) {
        if (cmd.find("cd ") == 0) {
            std::string path = cmd.substr(3);
            if (chdir(path.c_str()) == 0) {
                std::cout << COLOR_CYAN << "Changed directory to: " << path << COLOR_RESET << std::endl;
                return 0;
            } else {
                std::cerr << COLOR_RED << "Error changing directory to: " << path << COLOR_RESET << std::endl;
                return -1;
            }
        }
        return execute_command(cmd); // Fall back to original for non-cd commands
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
        std::cout << "\033[33m";
        std::cout << "╔══════════════════════════════════════════════════════════════╗" << std::endl;
        std::cout << "║                    Available Drives                         ║" << std::endl;
        std::cout << "╠══════════════════════════════════════════════════════════════╣" << std::endl;
        std::cout << "\033[0m";

        std::cout << "\033[38;2;0;255;255m";
        system("sudo lsblk -o NAME,SIZE,TYPE,MOUNTPOINT,FSTYPE,MODEL | grep -v \"loop\"");

        std::cout << "\033[33m";
        std::cout << "╚══════════════════════════════════════════════════════════════╝" << std::endl;
        std::cout << "\033[0m" << std::endl;
    }

    // Function to display header
    void display_header() {
        std::cout << "\033[31m";
        std::cout << "░█████╗░██╗░░░░░░█████╗░██║░░░██╗██████╗░███████╗███╗░░░███╗░█████╗░██████╗░░██████╗" << std::endl;
        std::cout << "██╔══██╗██║░░░░░██╔══██╗██║░░░██║██╔══██╗██╔════╝████╗░████║██╔══██╗██╔══██╗██╔════╝" << std::endl;
        std::cout << "██║░░╚═╝██║░░░░░███████║██║░░░██║██║░░██║█████╗░░██╔████╔██║██║░░██║██║░░██║╚█████╗░" << std::endl;
        std::cout << "██║░░██╗██║░░░░░██╔══██║██║░░░██║██║░░██║██╔══╝░░██║╚██╔╝██║██║░░██║██║░░██║░╚═══██╗" << std::endl;
        std::cout << "╚█████╔╝███████╗██║░░██║╚██████╔╝██████╔╝███████╗██║░╚═╝░██║╚█████╔╝██████╔╝██████╔╝" << std::endl;
        std::cout << "░╚════╝░╚══════╝╚═╝░░░░░░╚═════╝░╚═════╝░╚══════╝╚═╝░░░░░╚═╝░╚════╝░╚═════╝░╚═════╝░" << std::endl;
        std::cout << "\033[38;2;0;255;255m" << "claudemods Distribution Installer Btrfs v1.0 06-11-2025" << "\033[0m" << std::endl;
        std::cout << "\033[38;2;0;255;255m" << "Supports Btrfs (with Zstd compression 22) filesystem" << "\033[0m" << std::endl;
        std::cout << std::endl;
    }

    // Function to prepare target partitions for Btrfs
    void prepare_target_partitions(const std::string& drive) {
        execute_command("umount -f " + drive + "* 2>/dev/null || true");
        execute_command("wipefs -a " + drive);
        execute_command("parted -s " + drive + " mklabel gpt");
        execute_command("parted -s " + drive + " mkpart primary fat32 1MiB 551MiB");
        execute_command("parted -s " + drive + " mkpart primary btrfs 551MiB 100%");
        execute_command("parted -s " + drive + " set 1 esp on");
        execute_command("partprobe " + drive);

        // Sleep for 2 seconds
        system("sleep 2");

        std::string efi_part = drive + "1";
        std::string root_part = drive + "2";

        if (!is_block_device(efi_part) || !is_block_device(root_part)) {
            std::cerr << "\033[31m" << "Error: Failed to create partitions" << "\033[0m" << std::endl;
            exit(1);
        }

        execute_command("mkfs.vfat -F32 " + efi_part);
        execute_command("mkfs.btrfs -f -L ROOT " + root_part);
    }

    // Function to setup Btrfs subvolumes
    void setup_btrfs_subvolumes(const std::string& root_part) {
        execute_command("mount " + root_part + " /mnt");
        execute_command("btrfs subvolume create /mnt/@");
        execute_command("btrfs subvolume create /mnt/@home");
        execute_command("btrfs subvolume create /mnt/@root");
        execute_command("btrfs subvolume create /mnt/@srv");
        execute_command("btrfs subvolume create /mnt/@cache");
        execute_command("btrfs subvolume create /mnt/@tmp");
        execute_command("btrfs subvolume create /mnt/@log");
        execute_command("mkdir -p /mnt/@/var/lib");
        execute_command("btrfs subvolume create /mnt/@/var/lib/portables");
        execute_command("btrfs subvolume create /mnt/@/var/lib/machines");
        execute_command("umount /mnt");

        execute_command("mount -o subvol=@,compress=zstd:22,compress-force=zstd:22 " + root_part + " /mnt");
        execute_command("mkdir -p /mnt/{home,root,srv,tmp,var/{cache,log},var/lib/{portables,machines},boot/efi}");

        execute_command("mount -o subvol=@home,compress=zstd:22,compress-force=zstd:22 " + root_part + " /mnt/home");
        execute_command("mount -o subvol=@root,compress=zstd:22,compress-force=zstd:22 " + root_part + " /mnt/root");
        execute_command("mount -o subvol=@srv,compress=zstd:22,compress-force=zstd:22 " + root_part + " /mnt/srv");
        execute_command("mount -o subvol=@cache,compress=zstd:22,compress-force=zstd:22 " + root_part + " /mnt/var/cache");
        execute_command("mount -o subvol=@tmp,compress=zstd:22,compress-force=zstd:22 " + root_part + " /mnt/tmp");
        execute_command("mount -o subvol=@log,compress=zstd:22,compress-force=zstd:22 " + root_part + " /mnt/var/log");
        execute_command("mount -o subvol=@/var/lib/portables,compress=zstd:22,compress-force=zstd:22 " + root_part + " /mnt/var/lib/portables");
        execute_command("mount -o subvol=@/var/lib/machines,compress=zstd:22,compress-force=zstd:22 " + root_part + " /mnt/var/lib/machines");
    }

    // Function to install GRUB for Btrfs
    void install_grub_btrfs(const std::string& drive) {
        execute_command("mount --bind /dev /mnt/dev");
        execute_command("mount --bind /dev/pts /mnt/dev/pts");
        execute_command("mount --bind /proc /mnt/proc");
        execute_command("mount --bind /sys /mnt/sys");
        execute_command("mount --bind /run /mnt/run");

        execute_command("chroot /mnt /bin/bash -c \"mount -t efivarfs efivarfs /sys/firmware/efi/efivars\"");
        execute_command("chroot /mnt /bin/bash -c \"genfstab -U / >> /etc/fstab\"");
        execute_command("chroot /mnt /bin/bash -c \"grub-install --target=x86_64-efi --efi-directory=/boot/efi --bootloader-id=GRUB --recheck\"");
        execute_command("chroot /mnt /bin/bash -c \"grub-mkconfig -o /boot/grub/grub.cfg\"");
        execute_command("chroot /mnt /bin/bash -c \"/opt/btrfsfstabcompressed.sh\"");
        execute_command("chroot /mnt /bin/bash -c \"mkinitcpio -P\"");
    }

    // Function to get drive selection (Step 1) - Modified to accept command line argument
    void get_drive_selection(int argc, char* argv[]) {
        // Check if drive was provided as command line argument
        if (argc >= 2) {
            selected_drive = argv[1];
            std::cout << "\033[32m" << "Using drive from command line: " << selected_drive << "\033[0m" << std::endl;

            if (!is_block_device(selected_drive)) {
                std::cerr << "\033[31m" << "Error: " << selected_drive << " is not a valid block device" << "\033[0m" << std::endl;
                exit(1);
            }
        } else {
            // Interactive drive selection
            display_available_drives();
            std::cout << "\033[38;2;0;255;255m" << "Enter target drive (e.g., /dev/sda): " << "\033[0m";
            std::getline(std::cin, selected_drive);
            if (!is_block_device(selected_drive)) {
                std::cerr << "\033[31m" << "Error: " << selected_drive << " is not a valid block device" << "\033[0m" << std::endl;
                exit(1);
            }
        }
    }

    // Function to get filesystem selection (Step 2) - Modified for Btrfs focus
    void get_filesystem_selection() {
        std::cout << "\033[38;2;0;255;255m" << "Filesystem type (btrfs): " << "\033[0m";
        std::getline(std::cin, fs_type);
        if (fs_type.empty()) {
            fs_type = "btrfs";
        }
        std::cout << "\033[32m" << "Using Btrfs filesystem with Zstd compression" << "\033[0m" << std::endl;
    }

    // Function to select kernel (Step 3)
    void get_kernel_selection() {
        while (true) {
            std::cout << "\033[38;2;0;255;255m";
            std::cout << "╔══════════════════════════════════════════════════════════════╗" << std::endl;
            std::cout << "║                      Select Kernel                          ║" << std::endl;
            std::cout << "╠══════════════════════════════════════════════════════════════╣" << std::endl;
            std::cout << "║  1. linux (Standard)                                        ║" << std::endl;
            std::cout << "║  2. linux-lts (Long Term Support)                           ║" << std::endl;
            std::cout << "║  3. linux-zen (Tuned for desktop performance)               ║" << std::endl;
            std::cout << "║  4. linux-hardened (Security-focused)                       ║" << std::endl;
            std::cout << "║  5. Return to Main Menu                                     ║" << std::endl;
            std::cout << "╚══════════════════════════════════════════════════════════════╝" << std::endl;
            std::cout << "\033[0m";

            std::cout << "\033[38;2;0;255;255m" << "Select kernel (1-5): " << "\033[0m";
            std::string kernel_choice;
            std::getline(std::cin, kernel_choice);

            if (kernel_choice == "1") {
                selected_kernel = "linux";
                break;
            } else if (kernel_choice == "2") {
                selected_kernel = "linux-lts";
                break;
            } else if (kernel_choice == "3") {
                selected_kernel = "linux-zen";
                break;
            } else if (kernel_choice == "4") {
                selected_kernel = "linux-hardened";
                break;
            } else if (kernel_choice == "5") {
                std::cout << "\033[38;2;0;255;255m" << "Returning to main menu..." << "\033[0m" << std::endl;
                break;
            } else {
                std::cout << "\033[31m" << "Invalid selection. Please enter a number between 1-5." << "\033[0m" << std::endl;
            }
        }
        std::cout << "\033[32m" << "Selected kernel: " << selected_kernel << "\033[0m" << std::endl;
    }

    // Function to get new user credentials (Step 4)
    void get_new_user_credentials() {
        std::cout << "\033[38;2;0;255;255m";
        std::cout << "╔══════════════════════════════════════════════════════════════╗" << std::endl;
        std::cout << "║                    User Configuration                        ║" << std::endl;
        std::cout << "╠══════════════════════════════════════════════════════════════╣" << std::endl;
        std::cout << "║  Please enter the following user details:                   ║" << std::endl;
        std::cout << "╚══════════════════════════════════════════════════════════════╝" << std::endl;
        std::cout << "\033[0m";

        // Get username
        std::cout << "\033[38;2;0;255;255m" << "Enter new username: " << "\033[0m";
        std::getline(std::cin, new_username);

        // Get root password
        std::cout << "\033[38;2;0;255;255m" << "Enter root password: " << "\033[0m";
        std::getline(std::cin, root_password);

        // Get user password
        std::cout << "\033[38;2;0;255;255m" << "Enter password for user '" << new_username << "': " << "\033[0m";
        std::getline(std::cin, user_password);

        std::cout << "\033[32m" << "User credentials stored successfully!" << "\033[0m" << std::endl;
    }

    // Function to setup timezone and keyboard (Step 5)
    void get_timezone_keyboard_settings() {
        // Timezone setup
        std::cout << "\033[38;2;0;255;255m";
        std::cout << "╔══════════════════════════════════════════════════════════════╗" << std::endl;
        std::cout << "║                      Timezone Setup                          ║" << std::endl;
        std::cout << "╠══════════════════════════════════════════════════════════════╣" << std::endl;
        std::cout << "║  1. America/New_York (US English)                           ║" << std::endl;
        std::cout << "║  2. Europe/London (UK English)                              ║" << std::endl;
        std::cout << "║  3. Europe/Berlin (German)                                  ║" << std::endl;
        std::cout << "║  4. Europe/Paris (French)                                   ║" << std::endl;
        std::cout << "║  5. Europe/Madrid (Spanish)                                 ║" << std::endl;
        std::cout << "║  6. Europe/Rome (Italian)                                   ║" << std::endl;
        std::cout << "║  7. Asia/Tokyo (Japanese)                                   ║" << std::endl;
        std::cout << "║  8. Other (manual entry)                                    ║" << std::endl;
        std::cout << "╚══════════════════════════════════════════════════════════════╝" << std::endl;
        std::cout << "\033[0m";

        std::string timezone_choice;
        std::cout << "\033[38;2;0;255;255m" << "Select timezone (1-8): " << "\033[0m";
        std::getline(std::cin, timezone_choice);

        if (timezone_choice == "1") {
            timezone = "America/New_York";
        } else if (timezone_choice == "2") {
            timezone = "Europe/London";
        } else if (timezone_choice == "3") {
            timezone = "Europe/Berlin";
        } else if (timezone_choice == "4") {
            timezone = "Europe/Paris";
        } else if (timezone_choice == "5") {
            timezone = "Europe/Madrid";
        } else if (timezone_choice == "6") {
            timezone = "Europe/Rome";
        } else if (timezone_choice == "7") {
            timezone = "Asia/Tokyo";
        } else if (timezone_choice == "8") {
            std::cout << "\033[38;2;0;255;255m" << "Enter timezone (e.g., Europe/Berlin): " << "\033[0m";
            std::getline(std::cin, timezone);
        } else {
            std::cout << "\033[31m" << "Invalid selection. Using default: America/New_York" << "\033[0m" << std::endl;
            timezone = "America/New_York";
        }

        // Keyboard layout setup
        std::cout << "\033[38;2;0;255;255m";
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
        std::cout << "\033[0m";

        std::string keyboard_choice;
        std::cout << "\033[38;2;0;255;255m" << "Select keyboard layout (1-8): " << "\033[0m";
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
            std::cout << "\033[38;2;0;255;255m" << "Enter keyboard layout (e.g., br, ru, pt): " << "\033[0m";
            std::getline(std::cin, keyboard_layout);
        } else {
            std::cout << "\033[31m" << "Invalid selection. Using default: us" << "\033[0m" << std::endl;
            keyboard_layout = "us";
        }

        std::cout << "\033[32m" << "Timezone: " << timezone << ", Keyboard: " << keyboard_layout << "\033[0m" << std::endl;
    }

    // Function to apply timezone and keyboard settings during installation
    void apply_timezone_keyboard_settings() {
        std::cout << "\033[38;2;0;255;255m" << "Setting timezone to: " << timezone << "\033[0m" << std::endl;
        execute_command("chroot /mnt /bin/bash -c \"ln -sf /usr/share/zoneinfo/" + timezone + " /etc/localtime\"");
        execute_command("chroot /mnt /bin/bash -c \"hwclock --systohc\"");

        std::cout << "\033[38;2;0;255;255m" << "Setting keyboard layout to: " << keyboard_layout << "\033[0m" << std::endl;
        execute_command("chroot /mnt /bin/bash -c \"echo 'KEYMAP=" + keyboard_layout + "' > /etc/vconsole.conf\"");
        execute_command("chroot /mnt /bin/bash -c \"echo 'LANG=en_US.UTF-8' > /etc/locale.conf\"");
        execute_command("chroot /mnt /bin/bash -c \"echo 'en_US.UTF-8 UTF-8' >> /etc/locale.gen\"");
        execute_command("chroot /mnt /bin/bash -c \"locale-gen\"");
    }

    // Function to apply user credentials during installation
    void apply_user_credentials() {
        std::cout << "\033[38;2;0;255;255m" << "Creating user '" << new_username << "'..." << "\033[0m" << std::endl;
        execute_command("chroot /mnt /bin/bash -c \"useradd -m -G wheel -s /bin/bash " + new_username + "\"");

        // Set passwords using stored credentials
        std::cout << "\033[38;2;0;255;255m" << "Setting root password..." << "\033[0m" << std::endl;
        execute_command("chroot /mnt /bin/bash -c \"echo 'root:" + root_password + "' | chpasswd\"");

        std::cout << "\033[38;2;0;255;255m" << "Setting password for user '" << new_username << "'..." << "\033[0m" << std::endl;
        execute_command("chroot /mnt /bin/bash -c \"echo '" + new_username + ":" + user_password + "' | chpasswd\"");

        execute_command("chroot /mnt /bin/bash -c \"echo '%wheel ALL=(ALL:ALL) ALL' | tee -a /etc/sudoers\"");
    }

    // Function to mount all Btrfs subvolumes for user operations
    void mount_all_btrfs_subvolumes(const std::string& drive) {
        std::string root_part = drive + "2";
        std::string efi_part = drive + "1";

        execute_command("mount -o subvol=@,compress=zstd:22,compress-force=zstd:22 " + root_part + " /mnt");
        execute_command("mount " + efi_part + " /mnt/boot/efi");
        execute_command("mount -o subvol=@home,compress=zstd:22,compress-force=zstd:22 " + root_part + " /mnt/home");
        execute_command("mount -o subvol=@root,compress=zstd:22,compress-force=zstd:22 " + root_part + " /mnt/root");
        execute_command("mount -o subvol=@srv,compress=zstd:22,compress-force=zstd:22 " + root_part + " /mnt/srv");
        execute_command("mount -o subvol=@cache,compress=zstd:22,compress-force=zstd:22 " + root_part + " /mnt/var/cache");
        execute_command("mount -o subvol=@tmp,compress=zstd:22,compress-force=zstd:22 " + root_part + " /mnt/tmp");
        execute_command("mount -o subvol=@log,compress=zstd:22,compress-force=zstd:22 " + root_part + " /mnt/var/log");
        execute_command("mount -o subvol=@/var/lib/portables,compress=zstd:22,compress-force=zstd:22 " + root_part + " /mnt/var/lib/portables");
        execute_command("mount -o subvol=@/var/lib/machines,compress=zstd:22,compress-force=zstd:22 " + root_part + " /mnt/var/lib/machines");
        execute_command("mount --bind /dev /mnt/dev");
        execute_command("mount --bind /dev/pts /mnt/dev/pts");
        execute_command("mount --bind /proc /mnt/proc");
        execute_command("mount --bind /sys /mnt/sys");
        execute_command("mount --bind /run /mnt/run");
    }

    // Function to change username in the new system (for Arch TTY Grub)
    void change_username(const std::string& fs_type, const std::string& drive) {
        std::cout << "\033[38;2;0;255;255m" << "Mounting system for username change..." << "\033[0m" << std::endl;

        mount_all_btrfs_subvolumes(drive);

        std::cout << "\033[38;2;0;255;255m" << "Changing username from 'arch' to '" + new_username + "'..." << "\033[0m" << std::endl;

        execute_command("chroot /mnt /bin/bash -c \"usermod -l " + new_username + " cachyos\"");
        execute_command("chroot /mnt /bin/bash -c \"mv /home/cachyos /home/" + new_username + "\"");
        execute_command("chroot /mnt /bin/bash -c \"usermod -d /home/" + new_username + " " + new_username + "\"");
        execute_command("chroot /mnt /bin/bash -c \"groupmod -n " + new_username + " cachyos\"");

        std::cout << "\033[38;2;0;255;255m" << "Adding " + new_username + " to sudo group..." << "\033[0m" << std::endl;
        execute_command("chroot /mnt /bin/bash -c \"gpasswd -a " + new_username + " wheel\"");

        // Apply stored passwords
        std::cout << "\033[38;2;0;255;255m" << "Setting root password..." << "\033[0m" << std::endl;
        execute_command("chroot /mnt /bin/bash -c \"echo 'root:" + root_password + "' | chpasswd\"");

        std::cout << "\033[38;2;0;255;255m" << "Setting password for user '" + new_username + "'..." << "\033[0m" << std::endl;
        execute_command("chroot /mnt /bin/bash -c \"echo '" + new_username + ":" + user_password + "' | chpasswd\"");

        execute_command("chroot /mnt /bin/bash -c \"echo '%wheel ALL=(ALL:ALL) ALL' | tee -a /etc/sudoers\"");

        // Apply stored timezone and keyboard settings
        apply_timezone_keyboard_settings();

        std::cout << "\033[32m" << "Username changed from 'arch' to '" + new_username + "'" << "\033[0m" << std::endl;
    }

    // Function to create new user (for desktop environments)
    std::string create_new_user(const std::string& fs_type, const std::string& drive) {
        std::cout << "\033[38;2;0;255;255m" << "Mounting system for user creation..." << "\033[0m" << std::endl;

        mount_all_btrfs_subvolumes(drive);

        // Apply stored user credentials
        apply_user_credentials();

        // Apply stored timezone and keyboard settings
        apply_timezone_keyboard_settings();

        std::cout << "\033[32m" << "User '" + new_username + "' created successfully with sudo privileges" << "\033[0m" << std::endl;

        return new_username;
    }

    // Function to unmount all mounted partitions before reboot
    void unmount_all_partitions() {
        std::cout << "\033[38;2;0;255;255m" << "Unmounting all partitions..." << "\033[0m" << std::endl;
        execute_command("umount -R /mnt 2>/dev/null || true");
    }

    // Function to prompt for reboot
    void prompt_reboot() {
        // Unmount all partitions before reboot prompt
        unmount_all_partitions();

        std::cout << "\033[38;2;0;255;255m" << "Installation completed successfully! Would you like to reboot now? (yes/no): " << "\033[0m";
        std::string reboot_choice;
        std::getline(std::cin, reboot_choice);

        if (reboot_choice == "yes" || reboot_choice == "y" || reboot_choice == "Y") {
            std::cout << "\033[32m" << "Rebooting system..." << "\033[0m" << std::endl;
            execute_command("sudo reboot");
        } else {
            std::cout << "\033[33m" << "You can reboot manually later using: sudo reboot" << "\033[0m" << std::endl;
        }
    }

    // Function to install arch tty grub (complete installation) using pacstrap
    void install_arch_tty_grub(const std::string& drive) {
        std::string fs_type = "btrfs";

        std::cout << "\033[38;2;0;255;255m" << "Starting Arch TTY Grub installation..." << "\033[0m" << std::endl;

        prepare_target_partitions(drive);

        std::string efi_part = drive + "1";
        std::string root_part = drive + "2";

        setup_btrfs_subvolumes(root_part);

        execute_command("pacstrap /mnt base " + selected_kernel + " linux-firmware grub efibootmgr os-prober sudo vim nano bash-completion networkmanager");

        // Copy btrfsfstabcompressed.sh to the new system
        execute_command("cp btrfsfstabcompressed.sh /mnt/opt");
        execute_command("chmod +x /mnt/opt/btrfsfstabcompressed.sh");

        execute_command("mount " + efi_part + " /mnt/boot/efi");

        execute_command("chroot /mnt /bin/bash -c \"systemctl enable NetworkManager\"");

        install_grub_btrfs(drive);

        create_new_user(fs_type, drive);

        std::cout << "\033[32m" << "Arch TTY Grub installation completed successfully!" << "\033[0m" << std::endl;

        prompt_reboot();
    }

    // Function to install desktop environments
    void install_desktop(const std::string& fs_type, const std::string& drive) {
        std::cout << "\033[38;2;0;255;255m";
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
        std::cout << "\033[0m";

        std::cout << "\033[38;2;0;255;255m" << "Select desktop environment (1-12): " << "\033[0m";
        std::string desktop_choice;
        std::getline(std::cin, desktop_choice);

        if (desktop_choice == "1") {
            install_arch_tty_grub(drive);
        } else if (desktop_choice == "2") {
            std::cout << "\033[38;2;0;255;255m" << "Installing GNOME Desktop..." << "\033[0m" << std::endl;

            prepare_target_partitions(drive);
            std::string efi_part = drive + "1";
            std::string root_part = drive + "2";

            setup_btrfs_subvolumes(root_part);

            execute_command("pacstrap /mnt base gnome gnome-extra gdm grub efibootmgr os-prober arch-install-scripts mkinitcpio " + selected_kernel + " linux-firmware sudo networkmanager");

            // Copy btrfsfstabcompressed.sh to the new system
            execute_command("cp btrfsfstabcompressed.sh /mnt/opt");
            execute_command("chmod +x /mnt/opt/btrfsfstabcompressed.sh");

            execute_command("mount " + efi_part + " /mnt/boot/efi");

            install_grub_btrfs(drive);

            create_new_user(fs_type, drive);

            execute_command("chroot /mnt /bin/bash -c \"systemctl enable gdm\"");
            execute_command("chroot /mnt /bin/bash -c \"systemctl enable NetworkManager\"");

            std::cout << "\033[32m" << "GNOME installation completed!" << "\033[0m" << std::endl;

            prompt_reboot();
        } else if (desktop_choice == "3") {
            std::cout << "\033[38;2;0;255;255m" << "Installing KDE Plasma..." << "\033[0m" << std::endl;

            prepare_target_partitions(drive);
            std::string efi_part = drive + "1";
            std::string root_part = drive + "2";

            setup_btrfs_subvolumes(root_part);

            execute_command("pacstrap /mnt base plasma sddm dolphin konsole grub efibootmgr os-prober arch-install-scripts mkinitcpio " + selected_kernel + " linux-firmware sudo networkmanager");

            // Copy btrfsfstabcompressed.sh to the new system
            execute_command("cp btrfsfstabcompressed.sh /mnt/opt");
            execute_command("chmod +x /mnt/opt/btrfsfstabcompressed.sh");

            execute_command("mount " + efi_part + " /mnt/boot/efi");

            install_grub_btrfs(drive);

            create_new_user(fs_type, drive);

            execute_command("chroot /mnt /bin/bash -c \"systemctl enable sddm\"");
            execute_command("chroot /mnt /bin/bash -c \"systemctl enable NetworkManager\"");

            std::cout << "\033[32m" << "KDE Plasma installation completed!" << "\033[0m" << std::endl;

            prompt_reboot();
        } else if (desktop_choice == "4") {
            std::cout << "\033[38;2;0;255;255m" << "Installing XFCE..." << "\033[0m" << std::endl;

            prepare_target_partitions(drive);
            std::string efi_part = drive + "1";
            std::string root_part = drive + "2";

            setup_btrfs_subvolumes(root_part);

            execute_command("pacstrap /mnt base xfce4 xfce4-goodies lightdm lightdm-gtk-greeter grub efibootmgr os-prober arch-install-scripts mkinitcpio " + selected_kernel + " linux-firmware sudo networkmanager");

            // Copy btrfsfstabcompressed.sh to the new system
            execute_command("cp btrfsfstabcompressed.sh /mnt/opt");
            execute_command("chmod +x /mnt/opt/btrfsfstabcompressed.sh");

            execute_command("mount " + efi_part + " /mnt/boot/efi");

            install_grub_btrfs(drive);

            create_new_user(fs_type, drive);

            execute_command("chroot /mnt /bin/bash -c \"systemctl enable lightdm\"");
            execute_command("chroot /mnt /bin/bash -c \"systemctl enable NetworkManager\"");

            std::cout << "\033[32m" << "XFCE installation completed!" << "\033[0m" << std::endl;

            prompt_reboot();
        } else if (desktop_choice == "5") {
            std::cout << "\033[38;2;0;255;255m" << "Installing LXQt..." << "\033[0m" << std::endl;

            prepare_target_partitions(drive);
            std::string efi_part = drive + "1";
            std::string root_part = drive + "2";

            setup_btrfs_subvolumes(root_part);

            execute_command("pacstrap /mnt base lxqt sddm grub efibootmgr os-prober arch-install-scripts mkinitcpio " + selected_kernel + " linux-firmware sudo networkmanager");

            // Copy btrfsfstabcompressed.sh to the new system
            execute_command("cp btrfsfstabcompressed.sh /mnt/opt");
            execute_command("chmod +x /mnt/opt/btrfsfstabcompressed.sh");

            execute_command("mount " + efi_part + " /mnt/boot/efi");

            install_grub_btrfs(drive);

            create_new_user(fs_type, drive);

            execute_command("chroot /mnt /bin/bash -c \"systemctl enable sddm\"");
            execute_command("chroot /mnt /bin/bash -c \"systemctl enable NetworkManager\"");

            std::cout << "\033[32m" << "LXQt installation completed!" << "\033[0m" << std::endl;

            prompt_reboot();
        } else if (desktop_choice == "6") {
            std::cout << "\033[38;2;0;255;255m" << "Installing Cinnamon..." << "\033[0m" << std::endl;

            prepare_target_partitions(drive);
            std::string efi_part = drive + "1";
            std::string root_part = drive + "2";

            setup_btrfs_subvolumes(root_part);

            execute_command("pacstrap /mnt base cinnamon lightdm lightdm-gtk-greeter grub efibootmgr os-prober arch-install-scripts mkinitcpio " + selected_kernel + " linux-firmware sudo networkmanager");

            // Copy btrfsfstabcompressed.sh to the new system
            execute_command("cp btrfsfstabcompressed.sh /mnt/opt");
            execute_command("chmod +x /mnt/opt/btrfsfstabcompressed.sh");

            execute_command("mount " + efi_part + " /mnt/boot/efi");

            install_grub_btrfs(drive);

            create_new_user(fs_type, drive);

            execute_command("chroot /mnt /bin/bash -c \"systemctl enable lightdm\"");
            execute_command("chroot /mnt /bin/bash -c \"systemctl enable NetworkManager\"");

            std::cout << "\033[32m" << "Cinnamon installation completed!" << "\033[0m" << std::endl;

            prompt_reboot();
        } else if (desktop_choice == "7") {
            std::cout << "\033[38;2;0;255;255m" << "Installing MATE..." << "\033[0m" << std::endl;

            prepare_target_partitions(drive);
            std::string efi_part = drive + "1";
            std::string root_part = drive + "2";

            setup_btrfs_subvolumes(root_part);

            execute_command("pacstrap /mnt base mate mate-extra lightdm lightdm-gtk-greeter grub efibootmgr os-prober arch-install-scripts mkinitcpio " + selected_kernel + " linux-firmware sudo networkmanager");

            // Copy btrfsfstabcompressed.sh to the new system
            execute_command("cp btrfsfstabcompressed.sh /mnt/opt");
            execute_command("chmod +x /mnt/opt/btrfsfstabcompressed.sh");

            execute_command("mount " + efi_part + " /mnt/boot/efi");

            install_grub_btrfs(drive);

            create_new_user(fs_type, drive);

            execute_command("chroot /mnt /bin/bash -c \"systemctl enable lightdm\"");
            execute_command("chroot /mnt /bin/bash -c \"systemctl enable NetworkManager\"");

            std::cout << "\033[32m" << "MATE installation completed!" << "\033[0m" << std::endl;

            prompt_reboot();
        } else if (desktop_choice == "8") {
            std::cout << "\033[38;2;0;255;255m" << "Installing Budgie..." << "\033[0m" << std::endl;

            prepare_target_partitions(drive);
            std::string efi_part = drive + "1";
            std::string root_part = drive + "2";

            setup_btrfs_subvolumes(root_part);

            execute_command("pacstrap /mnt base budgie-desktop lightdm lightdm-gtk-greeter grub efibootmgr os-prober arch-install-scripts mkinitcpio " + selected_kernel + " linux-firmware sudo networkmanager");

            // Copy btrfsfstabcompressed.sh to the new system
            execute_command("cp btrfsfstabcompressed.sh /mnt/opt");
            execute_command("chmod +x /mnt/opt/btrfsfstabcompressed.sh");

            execute_command("mount " + efi_part + " /mnt/boot/efi");

            install_grub_btrfs(drive);

            create_new_user(fs_type, drive);

            execute_command("chroot /mnt /bin/bash -c \"systemctl enable lightdm\"");
            execute_command("chroot /mnt /bin/bash -c \"systemctl enable NetworkManager\"");

            std::cout << "\033[32m" << "Budgie installation completed!" << "\033[0m" << std::endl;

            prompt_reboot();
        } else if (desktop_choice == "9") {
            std::cout << "\033[38;2;0;255;255m" << "Installing i3 (tiling WM)..." << "\033[0m" << std::endl;

            prepare_target_partitions(drive);
            std::string efi_part = drive + "1";
            std::string root_part = drive + "2";

            setup_btrfs_subvolumes(root_part);

            execute_command("pacstrap /mnt base i3-wm i3status i3lock dmenu lightdm lightdm-gtk-greeter grub efibootmgr os-prober arch-install-scripts mkinitcpio " + selected_kernel + " linux-firmware sudo networkmanager");

            // Copy btrfsfstabcompressed.sh to the new system
            execute_command("cp btrfsfstabcompressed.sh /mnt/opt");
            execute_command("chmod +x /mnt/opt/btrfsfstabcompressed.sh");

            execute_command("mount " + efi_part + " /mnt/boot/efi");

            install_grub_btrfs(drive);

            create_new_user(fs_type, drive);

            execute_command("chroot /mnt /bin/bash -c \"systemctl enable lightdm\"");
            execute_command("chroot /mnt /bin/bash -c \"systemctl enable NetworkManager\"");

            std::cout << "\033[32m" << "i3 installation completed!" << "\033[0m" << std::endl;

            prompt_reboot();
        } else if (desktop_choice == "10") {
            std::cout << "\033[38;2;0;255;255m" << "Installing Sway (Wayland tiling)..." << "\033[0m" << std::endl;

            prepare_target_partitions(drive);
            std::string efi_part = drive + "1";
            std::string root_part = drive + "2";

            setup_btrfs_subvolumes(root_part);

            execute_command("pacstrap /mnt base sway swaybg waybar wofi lightdm lightdm-gtk-greeter grub efibootmgr os-prober arch-install-scripts mkinitcpio " + selected_kernel + " linux-firmware sudo networkmanager");

            // Copy btrfsfstabcompressed.sh to the new system
            execute_command("cp btrfsfstabcompressed.sh /mnt/opt");
            execute_command("chmod +x /mnt/opt/btrfsfstabcompressed.sh");

            execute_command("mount " + efi_part + " /mnt/boot/efi");

            install_grub_btrfs(drive);

            create_new_user(fs_type, drive);

            execute_command("chroot /mnt /bin/bash -c \"systemctl enable lightdm\"");
            execute_command("chroot /mnt /bin/bash -c \"systemctl enable NetworkManager\"");

            std::cout << "\033[32m" << "Sway installation completed!" << "\033[0m" << std::endl;

            prompt_reboot();
        } else if (desktop_choice == "11") {
            std::cout << "\033[38;5;93m" << "Installing Hyprland (Modern Wayland Compositor)..." << "\033[0m" << std::endl;

            prepare_target_partitions(drive);
            std::string efi_part = drive + "1";
            std::string root_part = drive + "2";

            setup_btrfs_subvolumes(root_part);

            execute_command("pacstrap /mnt base hyprland waybar rofi wl-clipboard sddm grub efibootmgr os-prober arch-install-scripts mkinitcpio " + selected_kernel + " linux-firmware sudo networkmanager");

            // Copy btrfsfstabcompressed.sh to the new system
            execute_command("cp btrfsfstabcompressed.sh /mnt/opt");
            execute_command("chmod +x /mnt/opt/btrfsfstabcompressed.sh");

            execute_command("mount " + efi_part + " /mnt/boot/efi");

            install_grub_btrfs(drive);

            create_new_user(fs_type, drive);

            execute_command("chroot /mnt /bin/bash -c \"systemctl enable sddm\"");
            execute_command("chroot /mnt /bin/bash -c \"systemctl enable NetworkManager\"");

            std::cout << "\033[38;5;93m" << "Hyprland installed! Note: You may need to configure ~/.config/hypr/hyprland.conf" << "\033[0m" << std::endl;

            prompt_reboot();
        } else if (desktop_choice == "12") {
            std::cout << "\033[38;2;0;255;255m" << "Returning to main menu..." << "\033[0m" << std::endl;
        } else {
            std::cout << "\033[31m" << "Invalid option. Returning to main menu." << "\033[0m" << std::endl;
        }
    }

    // Function to install CachyOS TTY Grub
    void install_cachyos_tty_grub(const std::string& drive) {
        std::cout << "\033[38;2;0;255;255m" << "Installing CachyOS TTY Grub..." << "\033[0m" << std::endl;

        prepare_target_partitions(drive);
        std::string efi_part = drive + "1";
        std::string root_part = drive + "2";

        setup_btrfs_subvolumes(root_part);

        execute_command("pacstrap /mnt base grub efibootmgr os-prober arch-install-scripts mkinitcpio " + selected_kernel + " linux-firmware sudo networkmanager");

        // Copy btrfsfstabcompressed.sh to the new system
        execute_command("cp btrfsfstabcompressed.sh /mnt/opt");
        execute_command("chmod +x /mnt/opt/btrfsfstabcompressed.sh");

        execute_command("mount " + efi_part + " /mnt/boot/efi");

        execute_command("chroot /mnt /bin/bash -c \"systemctl enable NetworkManager\"");

        install_grub_btrfs(drive);

        create_new_user("btrfs", drive);

        std::cout << "\033[32m" << "CachyOS TTY Grub installation completed!" << "\033[0m" << std::endl;

        prompt_reboot();
    }

    // Function to install CachyOS KDE
    void install_cachyos_kde(const std::string& drive) {
        std::cout << "\033[38;2;0;255;255m" << "Installing CachyOS KDE..." << "\033[0m" << std::endl;

        prepare_target_partitions(drive);
        std::string efi_part = drive + "1";
        std::string root_part = drive + "2";

        setup_btrfs_subvolumes(root_part);

        execute_command("pacstrap /mnt base plasma sddm dolphin konsole grub efibootmgr os-prober arch-install-scripts mkinitcpio " + selected_kernel + " linux-firmware sudo networkmanager");

        execute_command("chroot /mnt /bin/bash -c \"systemctl enable sddm\"");
        execute_command("chroot /mnt /bin/bash -c \"systemctl enable NetworkManager\"");

        // Copy btrfsfstabcompressed.sh to the new system
        execute_command("cp btrfsfstabcompressed.sh /mnt/opt");
        execute_command("chmod +x /mnt/opt/btrfsfstabcompressed.sh");

        execute_command("mount " + efi_part + " /mnt/boot/efi");

        install_grub_btrfs(drive);

        create_new_user("btrfs", drive);

        std::cout << "\033[38;2;0;255;255m" << "Setting up CachyOS..." << "\033[0m" << std::endl;
        execute_command("mkdir /mnt/home/" + new_username + "/.config");
        execute_command("mkdir /mnt/home/" + new_username + "/.config/autostart");
        execute_command("cp -r /opt/claudemods-distribution-installer /mnt/opt");
        execute_command("cp -r /opt/claudemods-distribution-installer/install-fullkde-grub/cachyoskdegrub.desktop /mnt/home/" + new_username + "/.config/autostart");
        execute_command("chroot /mnt chown " + new_username + new_username + " /home/" + new_username + "/.config");
        execute_command("chroot /mnt chown " + new_username + new_username + " /home/" + new_username + "/.config/autostart");
        execute_command("chroot /mnt chown " + new_username + new_username + " /mnt/home/" + new_username + "/.config/autostart/cachyoskdegrub.desktop");
        execute_command("chmod +x /mnt/home/" + new_username + "/.config/autostart/cachyoskdegrub.desktop");
        execute_command("chmod +x /opt/claudemods-distribution-installer/install-fullkde-grub/*");

        std::cout << "\033[32m" << "CachyOS KDE Part 1 installation completed!" << "\033[0m" << std::endl;
        std::cout << "\033[32m" << " For CachyOS KDE Part 2 installation Please Reboot And login To Run Next Script!" << "\033[0m" << std::endl;

        prompt_reboot();
    }

    // Function to install CachyOS GNOME
    void install_cachyos_gnome(const std::string& drive) {
        std::cout << "\033[38;2;0;255;255m" << "Installing CachyOS GNOME..." << "\033[0m" << std::endl;

        prepare_target_partitions(drive);
        std::string efi_part = drive + "1";
        std::string root_part = drive + "2";

        setup_btrfs_subvolumes(root_part);

        execute_command("pacstrap /mnt base gnome gnome-extra gdm grub efibootmgr os-prober arch-install-scripts mkinitcpio " + selected_kernel + " linux-firmware sudo networkmanager");

        execute_command("chroot /mnt /bin/bash -c \"systemctl enable gdm\"");
        execute_command("chroot /mnt /bin/bash -c \"systemctl enable NetworkManager\"");

        // Copy btrfsfstabcompressed.sh to the new system
        execute_command("cp btrfsfstabcompressed.sh /mnt/opt");
        execute_command("chmod +x /mnt/opt/btrfsfstabcompressed.sh");

        execute_command("mount " + efi_part + " /mnt/boot/efi");

        install_grub_btrfs(drive);

        create_new_user("btrfs", drive);

        std::cout << "\033[38;2;0;255;255m" << "Setting up CachyOS..." << "\033[0m" << std::endl;

        std::cout << "\033[32m" << "CachyOS GNOME installation completed!" << "\033[0m" << std::endl;

        prompt_reboot();
    }

    // Function to display Cachyos menu
    void display_cachyos_menu(const std::string& fs_type, const std::string& drive) {
        while (true) {
            std::cout << "\033[38;2;0;255;255m";
            std::cout << "╔══════════════════════════════════════════════════════════════╗" << std::endl;
            std::cout << "║                    CachyOS Options                          ║" << std::endl;
            std::cout << "╠══════════════════════════════════════════════════════════════╣" << std::endl;
            std::cout << "║  1. Install CachyOS TTY Grub                               ║" << std::endl;
            std::cout << "║  2. Install CachyOS KDE Grub                               ║" << std::endl;
            std::cout << "║  3. Install CachyOS GNOME Grub                             ║" << std::endl;
            std::cout << "║  4. Return to Main Menu                                    ║" << std::endl;
            std::cout << "╚══════════════════════════════════════════════════════════════╝" << std::endl;
            std::cout << "\033[0m";

            std::cout << "\033[38;2;0;255;255m" << "Select CachyOS option (1-4): " << "\033[0m";
            std::string cachyos_choice;
            std::getline(std::cin, cachyos_choice);

            if (cachyos_choice == "1") {
                install_cachyos_tty_grub(drive);
            } else if (cachyos_choice == "2") {
                install_cachyos_kde(drive);
            } else if (cachyos_choice == "3") {
                install_cachyos_gnome(drive);
            } else if (cachyos_choice == "4") {
                std::cout << "\033[38;2;0;255;255m" << "Returning to main menu..." << "\033[0m" << std::endl;
                break;
            } else {
                std::cout << "\033[31m" << "Invalid option. Please try again." << "\033[0m" << std::endl;
            }
        }
    }

    // Function to install Spitfire CKGE
    void install_spitfire_ckge(const std::string& drive) {
        std::cout << COLOR_ORANGE << "Installing Spitfire CKGE..." << COLOR_RESET << std::endl;

        prepare_target_partitions(drive, "btrfs");
        std::string efi_part = drive + "1";
        std::string root_part = drive + "2";

        setup_btrfs_filesystem(root_part);

        // Use execute_cd_command for cd commands
        execute_cd_command("cd /mnt");
        execute_command("wget --show-progress --no-check-certificate --continue --tries=3 --timeout=30 --waitretry=5 https://claudemodsreloaded.co.uk/claudemods-v1.img");
        execute_command("unsquashfs -f -d /mnt /mnt/claudemods-v1.img");
        execute_command("mount " + efi_part + " /mnt/boot/efi");

        install_grub_btrfs(drive);

        change_username("ext4", drive);
        execute_command("cp -r /etc/resolv.conf /mnt/etc");
        execute_command("cp -r /opt/claudemods-distribution-installer/spitfire-ckge-minimal/desktop.sh /mnt/opt/Arch-Systemtool");
        execute_command("chmod +x /mnt/opt/Arch-Systemtool/desktop.sh");
        execute_command("chroot /mnt /bin/bash -c \"su - " + new_username + " -c 'cd /home/" + new_username + " && git clone https://github.com/claudemods/claudemods-distribution-installer'\"");
        execute_command("chroot /mnt /bin/bash -c \"su - " + new_username + " -c 'cd /home/" + new_username + "/claudemods-distribution-installer/installer && chmod +x dolphinfixes.sh'\"");
        execute_command("chroot /mnt /bin/bash -c \"su - " + new_username + " -c 'cd /home/" + new_username + "/claudemods-distribution-installer/installer && ./dolphinfixes.sh " + new_username + "'\"");
        execute_command("chroot /mnt /bin/bash -c \"su - " + new_username + " -c 'cd /home/" + new_username + "/claudemods-distribution-installer/installer/spitfire-ckge-minimal && chmod +x installspitfire.sh'\"");
        execute_command("chroot /mnt /bin/bash -c \"su - " + new_username + " -c 'cd /home/" + new_username + "/claudemods-distribution-installer/installer/spitfire-ckge-minimal && ./installspitfire.sh " + new_username + "'\"");

        std::cout << COLOR_ORANGE << "Spitfire CKGE installation completed!" << COLOR_RESET << std::endl;

        prompt_reboot();
    }

    // Function to install Apex CKGE
    void install_apex_ckge(const std::string& drive) {
        std::cout << COLOR_PURPLE << "Installing Apex CKGE..." << COLOR_RESET << std::endl;

        prepare_target_partitions(drive, "btrfs");
        std::string efi_part = drive + "1";
        std::string root_part = drive + "2";

        setup_btrfs_filesystem(root_part);

        execute_cd_command("cd /mnt");
        execute_command("wget --show-progress --no-check-certificate --continue --tries=3 --timeout=30 --waitretry=5 https://claudemodsreloaded.co.uk/claudemods-v1.img");
        execute_command("unsquashfs -f -d /mnt /mnt/claudemods-v1.img");
        execute_command("mount " + efi_part + " /mnt/boot/efi");

        install_grub_btrfs(drive);

        change_username("btrfs", drive);
        execute_command("cp -r /etc/resolv.conf /mnt/etc");
        execute_command("chroot /mnt /bin/bash -c \"su - " + new_username + " -c 'cd /home/" + new_username + " && git clone https://github.com/claudemods/claudemods-distribution-installer'\"");
        execute_command("chroot /mnt /bin/bash -c \"su - " + new_username + " -c 'cd /home/" + new_username + "/claudemods-distribution-installer/installer && chmod +x dolphinfixes.sh'\"");
        execute_command("chroot /mnt /bin/bash -c \"su - " + new_username + " -c 'cd /home/" + new_username + "/claudemods-distribution-installer/installer && ./dolphinfixes.sh " + new_username + "'\"");


        execute_command("chroot /mnt /bin/bash -c \"su - " + new_username + " -c 'cd /home/" + new_username + "/claudemods-distribution-installer/installer && chmod +x cleanup.sh'\"");
        execute_command("chroot /mnt /bin/bash -c \"su - " + new_username + " -c 'cd /home/" + new_username + "/claudemods-distribution-installer/installer && ./cleanup.sh'\"");

        std::cout << COLOR_PURPLE << "Apex CKGE installation completed!" << COLOR_RESET << std::endl;

        prompt_reboot();
    }

    // Function to display Claudemods Distribution menu
    void display_claudemods_menu(const std::string& fs_type, const std::string& drive) {
        while (true) {
            std::cout << "\033[38;2;0;255;255m";
            std::cout << "╔══════════════════════════════════════════════════════════════╗" << std::endl;
            std::cout << "║               Claudemods Distribution Options               ║" << std::endl;
            std::cout << "╠══════════════════════════════════════════════════════════════╣" << std::endl;
            std::cout << "║  1. Install Spitfire CKGE                                  ║" << std::endl;
            std::cout << "║  2. Install Apex CKGE                                      ║" << std::endl;
            std::cout << "║  3. Return to Main Menu                                    ║" << std::endl;
            std::cout << "╚══════════════════════════════════════════════════════════════╝" << std::endl;
            std::cout << "\033[0m";

            std::cout << "\033[38;2;0;255;255m" << "Select Claudemods option (1-3): " << "\033[0m";
            std::string claudemods_choice;
            std::getline(std::cin, claudemods_choice);

            if (claudemods_choice == "1") {
                install_spitfire_ckge(drive);
            } else if (claudemods_choice == "2") {
                install_apex_ckge(drive);
            } else if (claudemods_choice == "3") {
                std::cout << "\033[38;2;0;255;255m" << "Returning to main menu..." << "\033[0m" << std::endl;
                break;
            } else {
                std::cout << "\033[31m" << "Invalid option. Please try again." << "\033[0m" << std::endl;
            }
        }
    }

    // Function to display main menu
    void main_menu() {
        while (true) {
            std::cout << "\033[38;2;0;255;255m";
            std::cout << "╔══════════════════════════════════════╗" << std::endl;
            std::cout << "║              Main Menu               ║" << std::endl;
            std::cout << "╠══════════════════════════════════════╣" << std::endl;
            std::cout << "║ 1. Install Vanilla Arch Desktop      ║" << std::endl;
            std::cout << "║ 2. Vanilla Cachyos Options           ║" << std::endl;
            std::cout << "║ 3. Claudemods Distribution Options   ║" << std::endl;
            std::cout << "║ 4. Reboot System                     ║" << std::endl;
            std::cout << "║ 5. Exit                              ║" << std::endl;
            std::cout << "╚══════════════════════════════════════╝" << std::endl;
            std::cout << "\033[0m";

            std::cout << "\033[38;2;0;255;255m" << "Select an option (1-5): " << "\033[0m";
            std::string choice;
            std::getline(std::cin, choice);

            if (choice == "1") {
                install_desktop(fs_type, selected_drive);
            } else if (choice == "2") {
                display_cachyos_menu(fs_type, selected_drive);
            } else if (choice == "3") {
                display_claudemods_menu(fs_type, selected_drive);
            } else if (choice == "4") {
                std::cout << "\033[32m" << "Rebooting system..." << "\033[0m" << std::endl;
                execute_command("sudo reboot");
            } else if (choice == "5") {
                std::cout << "\033[32m" << "Exiting. Goodbye!" << "\033[0m" << std::endl;
                exit(0);
            } else {
                std::cout << "\033[31m" << "Invalid option. Please try again." << "\033[0m" << std::endl;
            }

            std::cout << std::endl;
            std::cout << "\033[33m" << "Press Enter to continue..." << "\033[0m";
            std::cin.ignore(); // Clear the buffer
            std::getline(std::cin, choice); // Wait for Enter
        }
    }

public:
    // Main script
    void run(int argc, char* argv[]) {
        display_header();

        // Step 1: Drive selection (now accepts command line argument)
        get_drive_selection(argc, argv);

        // Step 2: Filesystem selection - Btrfs only
        fs_type = "btrfs";
        std::cout << "\033[32m" << "Using Btrfs filesystem with Zstd compression" << "\033[0m" << std::endl;

        // Step 3: Kernel selection
        get_kernel_selection();

        // Step 4: User credentials
        get_new_user_credentials();

        // Step 5: Timezone and keyboard
        get_timezone_keyboard_settings();

        // Show main menu for Btrfs
        main_menu();
    }
};

#endif // BTRFSINSTALLER_H
