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
#include <unistd.h>
#include <fstream>
#include <termios.h>
#include <sys/ioctl.h>
#include "btrfsinstallerdev.h"

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
    // Store user inputs for use during installation
    std::string selected_drive;
    std::string fs_type;
    std::string selected_kernel;
    std::string new_username;
    std::string timezone;
    std::string keyboard_layout;
    std::string root_password;
    std::string user_password;
    std::string installation_type;
    std::string desktop_environment;

    // Terminal control for arrow keys
    struct termios oldt, newt;

    // Function to display header
    void display_header() {
        std::cout << COLOR_RED;
        std::cout << "░█████╗░██╗░░░░░░█████╗░██║░░░██╗██████╗░███████╗███╗░░░███╗░█████╗░██████╗░░██████╗" << std::endl;
        std::cout << "██╔══██╗██║░░░░░██╔══██╗██║░░░██║██╔══██╗██╔════╝████╗░████║██╔══██╗██╔══██╗██╔════╝" << std::endl;
        std::cout << "██║░░╚═╝██║░░░░░███████║██║░░░██║██║░░██║█████╗░░██╔████╔██║██║░░██║██║░░██║╚█████╗░" << std::endl;
        std::cout << "██║░░██╗██║░░░░░██╔══██║██║░░░██║██║░░██║██╔══╝░░██║╚██╔╝██║██║░░██║██║░░██║░╚═══██╗" << std::endl;
        std::cout << "╚█████╔╝███████╗██║░░██║╚██████╔╝██████╔╝███████╗██║░╚═╝░██║╚█████╔╝██████╔╝██████╔╝" << std::endl;
        std::cout << "░╚════╝░╚══════╝╚═╝░░░░░░╚═════╝░╚═════╝░╚══════╝╚═╝░░░░░╚═╝░╚════╝░╚═════╝░╚═════╝░" << std::endl;
        std::cout << COLOR_CYAN << "claudemods distribution installer Beta DevBranch v1.01 11-11-2025" << COLOR_RESET << std::endl;
        std::cout << COLOR_CYAN << "Supports Ext4 And Btrfs filesystems" << COLOR_RESET << std::endl;
        std::cout << std::endl;
    }

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

    // Function to setup terminal for arrow key reading
    void setup_terminal() {
        tcgetattr(STDIN_FILENO, &oldt);
        newt = oldt;
        newt.c_lflag &= ~(ICANON | ECHO);
        tcsetattr(STDIN_FILENO, TCSANOW, &newt);
    }

    // Function to restore terminal
    void restore_terminal() {
        tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
    }

    // Function to get arrow key input
    int get_arrow_key() {
        int ch = getchar();
        if (ch == 27) { // ESC
            getchar(); // Skip [
            ch = getchar(); // Actual key
            return ch;
        }
        return ch;
    }

    // Function to display menu with selection
    int show_menu(const std::vector<std::string>& options, const std::string& title, int selected = 0) {
        setup_terminal();

        while (true) {
            system("clear");
            display_header(); // Display ASCII art and custom text at all times

            std::cout << COLOR_CYAN;
            std::cout << "╔══════════════════════════════════════════════════════════════╗" << std::endl;
            std::cout << "║ " << std::left << std::setw(60) << title << "║" << std::endl;
            std::cout << "╠══════════════════════════════════════════════════════════════╣" << std::endl;

            for (int i = 0; i < options.size(); i++) {
                std::cout << "║ ";
                if (i == selected) {
                    std::cout << COLOR_GREEN << "> " << COLOR_RESET << COLOR_CYAN;
                } else {
                    std::cout << "  ";
                }
                std::cout << std::left << std::setw(58) << options[i] << "║" << std::endl;
            }

            std::cout << "╚══════════════════════════════════════════════════════════════╝" << std::endl;
            std::cout << COLOR_RESET;
            std::cout << COLOR_YELLOW << "Use ↑↓ arrows to navigate, Enter to select" << COLOR_RESET << std::endl;

            int key = get_arrow_key();
            if (key == 'A') { // Up arrow
                selected = (selected - 1 + options.size()) % options.size();
            } else if (key == 'B') { // Down arrow
                selected = (selected + 1) % options.size();
            } else if (key == 10) { // Enter
                restore_terminal();
                return selected;
            }
        }
    }

    // Function to get text input with prompt
    std::string get_input(const std::string& prompt) {
        std::cout << COLOR_CYAN << prompt << COLOR_RESET;
        std::string input;
        std::getline(std::cin, input);
        return input;
    }

    // Function to extract version number (v1.0, v1.01, etc.)
    std::string extract_version_number(const std::string& version_string) {
        size_t v_pos = version_string.find("v");
        if (v_pos != std::string::npos) {
            size_t end = v_pos + 1;
            while (end < version_string.length() &&
                (isdigit(version_string[end]) || version_string[end] == '.')) {
                end++;
                }
                return version_string.substr(v_pos, end - v_pos);
        }
        return "";
    }

    // Function to extract date for comparison (DD-MM-YYYY format)
    int extract_date_value(const std::string& version_string) {
        // Look for DD-MM-YYYY pattern
        for (size_t i = 0; i < version_string.length() - 9; i++) {
            if (isdigit(version_string[i]) && isdigit(version_string[i+1]) &&
                version_string[i+2] == '-' &&
                isdigit(version_string[i+3]) && isdigit(version_string[i+4]) &&
                version_string[i+5] == '-' &&
                isdigit(version_string[i+6]) && isdigit(version_string[i+7]) &&
                isdigit(version_string[i+8]) && isdigit(version_string[i+9])) {

                std::string day = version_string.substr(i, 2);
            std::string month = version_string.substr(i+3, 2);
            std::string year = version_string.substr(i+6, 4);

            // Convert to integer for comparison (YYYYMMDD format)
            return std::stoi(year + month + day);
                }
        }
        return 0;
    }

    // Function to check for updates automatically
    bool check_for_updates() {
        std::cout << COLOR_CYAN << "Checking for updates..." << COLOR_RESET << std::endl;

        // Paths
        std::string local_version_file = "/opt/claudemods-distribution-installer/version.txt";
        std::string remote_version_url = "https://claudemodsreloaded.co.uk/version.txt";
        std::string updater_script = "/opt/claudemods-distribution-installer/updater";

        // Download remote version using wget
        std::string download_cmd = "wget -qO- " + remote_version_url + " 2>/dev/null";
        FILE* pipe = popen(download_cmd.c_str(), "r");
        if (!pipe) {
            std::cout << COLOR_YELLOW << "Failed to check for updates. Continuing..." << COLOR_RESET << std::endl;
            return false;
        }

        char buffer[128];
        std::string remote_version = "";
        while (fgets(buffer, sizeof(buffer), pipe) != nullptr) {
            remote_version += buffer;
        }
        pclose(pipe);

        // Remove only newlines, keep spaces
        remote_version.erase(std::remove(remote_version.begin(), remote_version.end(), '\n'), remote_version.end());
        remote_version.erase(std::remove(remote_version.begin(), remote_version.end(), '\r'), remote_version.end());

        if (remote_version.empty()) {
            std::cout << COLOR_YELLOW << "Failed to retrieve remote version. Continuing..." << COLOR_RESET << std::endl;
            return false;
        }

        // Read local version
        std::string local_version = "";
        std::ifstream local_file(local_version_file);
        if (local_file.is_open()) {
            std::getline(local_file, local_version);
            local_file.close();
            // Remove only newlines, keep spaces
            local_version.erase(std::remove(local_version.begin(), local_version.end(), '\n'), local_version.end());
            local_version.erase(std::remove(local_version.begin(), local_version.end(), '\r'), local_version.end());
        } else {
            std::cout << COLOR_YELLOW << "No local version found. First run?" << COLOR_RESET << std::endl;
            local_version = "0";
        }

        // Print versions with their original formatting
        std::cout << COLOR_CYAN << "Local version: " << local_version << COLOR_RESET << std::endl;
        std::cout << COLOR_CYAN << "Remote version: " << remote_version << COLOR_RESET << std::endl;

        // Extract version numbers and dates
        std::string remote_ver = extract_version_number(remote_version);
        std::string local_ver = extract_version_number(local_version);
        int remote_date = extract_date_value(remote_version);
        int local_date = extract_date_value(local_version);

        // Compare: if version numbers are different OR dates are different
        bool is_newer = false;

        if (remote_ver != local_ver) {
            // If version numbers are different, use version comparison
            is_newer = (remote_ver > local_ver);
        } else {
            // If version numbers are the same, use date comparison
            is_newer = (remote_date > local_date);
        }

        if (is_newer) {
            std::cout << COLOR_GREEN << "New version available!" << COLOR_RESET << std::endl;

            // Ask user if they want to update
            std::cout << COLOR_CYAN << "Do you want to update? (yes/no): " << COLOR_RESET;
            std::string response;
            std::getline(std::cin, response);

            if (response == "yes" || response == "y" || response == "Y") {
                std::cout << COLOR_GREEN << "Running updater..." << COLOR_RESET << std::endl;
                std::string update_cmd = updater_script;
                int update_status = system(update_cmd.c_str());

                if (update_status == 0) {
                    std::cout << COLOR_GREEN << "Update completed successfully! Restarting..." << COLOR_RESET << std::endl;
                    return true;
                } else {
                    std::cout << COLOR_RED << "Update failed. Continuing with current version." << COLOR_RESET << std::endl;
                    return false;
                }
            } else {
                std::cout << COLOR_YELLOW << "Update cancelled. Continuing with current version." << COLOR_RESET << std::endl;
                return false;
            }
        } else {
            std::cout << COLOR_GREEN << "You are running the latest version." << COLOR_RESET << std::endl;
            return false;
        }
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

    // Function to show confirmation screen
    bool show_confirmation_screen() {
        system("clear");
        display_header(); // Display ASCII art and custom text
        std::cout << COLOR_CYAN;
        std::cout << "╔══════════════════════════════════════════════════════════════╗" << std::endl;
        std::cout << "║                  Installation Summary                       ║" << std::endl;
        std::cout << "╠══════════════════════════════════════════════════════════════╣" << std::endl;
        std::cout << "║ " << std::left << std::setw(35) << "Drive:" << selected_drive << std::setw(23) << "" << "║" << std::endl;
        std::cout << "║ " << std::left << std::setw(35) << "Filesystem:" << fs_type << std::setw(23) << "" << "║" << std::endl;
        std::cout << "║ " << std::left << std::setw(35) << "Kernel:" << selected_kernel << std::setw(23) << "" << "║" << std::endl;
        std::cout << "║ " << std::left << std::setw(35) << "Username:" << new_username << std::setw(23) << "" << "║" << std::endl;
        std::cout << "║ " << std::left << std::setw(35) << "Timezone:" << timezone << std::setw(23) << "" << "║" << std::endl;
        std::cout << "║ " << std::left << std::setw(35) << "Keyboard Layout:" << keyboard_layout << std::setw(23) << "" << "║" << std::endl;
        std::cout << "║ " << std::left << std::setw(35) << "Installation Type:" << installation_type << std::setw(23) << "" << "║" << std::endl;
        if (!desktop_environment.empty()) {
            std::cout << "║ " << std::left << std::setw(35) << "Desktop Environment:" << desktop_environment << std::setw(23) << "" << "║" << std::endl;
        }
        std::cout << "╠══════════════════════════════════════════════════════════════╣" << std::endl;
        std::cout << "║ " << COLOR_RED << "WARNING: This will erase all data on " << selected_drive << COLOR_CYAN << "   ║" << std::endl;
        std::cout << "╚══════════════════════════════════════════════════════════════╝" << std::endl;
        std::cout << COLOR_RESET;

        std::vector<std::string> options = {"Start Installation", "Go Back to Main Menu"};
        int choice = show_menu(options, "Confirm Installation");

        return (choice == 0); // Return true if "Start Installation" selected
    }

    // Function to apply timezone and keyboard settings during installation
    void apply_timezone_keyboard_settings() {
        std::cout << COLOR_CYAN << "Setting timezone to: " << timezone << COLOR_RESET << std::endl;
        execute_command("chroot /mnt /bin/bash -c \"ln -sf /usr/share/zoneinfo/" + timezone + " /etc/localtime\"");
        execute_command("chroot /mnt /bin/bash -c \"hwclock --systohc\"");

        std::cout << COLOR_CYAN << "Setting keyboard layout to: " << keyboard_layout << COLOR_RESET << std::endl;
        execute_command("chroot /mnt /bin/bash -c \"echo 'KEYMAP=" + keyboard_layout + "' > /etc/vconsole.conf\"");
        execute_command("chroot /mnt /bin/bash -c \"echo 'LANG=en_US.UTF-8' > /etc/locale.conf\"");
        execute_command("chroot /mnt /bin/bash -c \"echo 'en_US.UTF-8 UTF-8' >> /etc/locale.gen\"");
        execute_command("chroot /mnt /bin/bash -c \"locale-gen\"");
    }

    // Function to apply user credentials during installation
    void apply_user_credentials() {
        std::cout << COLOR_CYAN << "Creating user '" << new_username << "'..." << COLOR_RESET << std::endl;
        execute_command("chroot /mnt /bin/bash -c \"useradd -m -G wheel -s /bin/bash " + new_username + "\"");

        std::cout << COLOR_CYAN << "Setting root password..." << COLOR_RESET << std::endl;
        execute_command("chroot /mnt /bin/bash -c \"echo 'root:" + root_password + "' | chpasswd\"");

        std::cout << COLOR_CYAN << "Setting password for user '" << new_username << "'..." << COLOR_RESET << std::endl;
        execute_command("chroot /mnt /bin/bash -c \"echo '" + new_username + ":" + user_password + "' | chpasswd\"");

        execute_command("chroot /mnt /bin/bash -c \"echo '%wheel ALL=(ALL:ALL) ALL' | tee -a /etc/sudoers\"");
    }

    // Function to change username in the new system (for Arch TTY Grub)
    void change_username(const std::string& fs_type, const std::string& drive) {
        std::cout << COLOR_CYAN << "Mounting system for username change..." << COLOR_RESET << std::endl;

        execute_command("mount " + drive + "2 /mnt");
        execute_command("mount " + drive + "1 /mnt/boot/efi");
        execute_command("mount --bind /dev /mnt/dev");
        execute_command("mount --bind /dev/pts /mnt/dev/pts");
        execute_command("mount --bind /proc /mnt/proc");
        execute_command("mount --bind /sys /mnt/sys");
        execute_command("mount --bind /run /mnt/run");

        std::cout << COLOR_CYAN << "Changing username from 'arch' to '" + new_username + "'..." << COLOR_RESET << std::endl;

        execute_command("chroot /mnt /bin/bash -c \"usermod -l " + new_username + " cachyos\"");
        execute_command("chroot /mnt /bin/bash -c \"mv /home/cachyos /home/" + new_username + "\"");
        execute_command("chroot /mnt /bin/bash -c \"usermod -d /home/" + new_username + " " + new_username + "\"");
        execute_command("chroot /mnt /bin/bash -c \"groupmod -n " + new_username + " cachyos\"");

        std::cout << COLOR_CYAN << "Adding " + new_username + " to sudo group..." << COLOR_RESET << std::endl;
        execute_command("chroot /mnt /bin/bash -c \"gpasswd -a " + new_username + " wheel\"");

        // Apply stored passwords
        std::cout << COLOR_CYAN << "Setting root password..." << COLOR_RESET << std::endl;
        execute_command("chroot /mnt /bin/bash -c \"echo 'root:" + root_password + "' | chpasswd\"");

        std::cout << COLOR_CYAN << "Setting password for user '" + new_username + "'..." << COLOR_RESET << std::endl;
        execute_command("chroot /mnt /bin/bash -c \"echo '" + new_username + ":" + user_password + "' | chpasswd\"");

        execute_command("chroot /mnt /bin/bash -c \"echo '%wheel ALL=(ALL:ALL) ALL' | tee -a /etc/sudoers\"");

        // Apply stored timezone and keyboard settings
        apply_timezone_keyboard_settings();

        std::cout << COLOR_GREEN << "Username changed from 'arch' to '" + new_username + "'" << COLOR_RESET << std::endl;
    }

    // Function to create new user (for desktop environments)
    std::string create_new_user(const std::string& fs_type, const std::string& drive) {
        std::cout << COLOR_CYAN << "Mounting system for user creation..." << COLOR_RESET << std::endl;

        execute_command("mount " + drive + "2 /mnt");
        execute_command("mount " + drive + "1 /mnt/boot/efi");
        execute_command("mount --bind /dev /mnt/dev");
        execute_command("mount --bind /dev/pts /mnt/dev/pts");
        execute_command("mount --bind /proc /mnt/proc");
        execute_command("mount --bind /sys /mnt/sys");
        execute_command("mount --bind /run /mnt/run");

        // Apply stored user credentials
        apply_user_credentials();

        // Apply stored timezone and keyboard settings
        apply_timezone_keyboard_settings();

        std::cout << COLOR_GREEN << "User '" + new_username + "' created successfully with sudo privileges" << COLOR_RESET << std::endl;

        return new_username;
    }

    // Function to unmount all mounted partitions before reboot
    void unmount_all_partitions() {
        std::cout << COLOR_CYAN << "Unmounting all partitions..." << COLOR_RESET << std::endl;
        execute_command("umount -R /mnt 2>/dev/null || true");
    }

    // Function to prompt for reboot
    void prompt_reboot() {
        // Unmount all partitions before reboot prompt
        unmount_all_partitions();

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

    // Function to install arch tty grub (complete installation) using pacstrap
    void install_arch_tty_grub(const std::string& drive) {
        std::string fs_type = "ext4";

        std::cout << COLOR_CYAN << "Starting Arch TTY Grub installation..." << COLOR_RESET << std::endl;

        prepare_target_partitions(drive, fs_type);

        std::string efi_part = drive + "1";
        std::string root_part = drive + "2";

        setup_ext4_filesystem(root_part);

        execute_command("cp -r /opt/claudemods-distribution-installer/vconsole.conf /mnt/etc");

        execute_command("cp -r /etc/resolv.conf /mnt/etc");

        execute_command("pacstrap /mnt base " + selected_kernel + " linux-firmware grub efibootmgr os-prober sudo arch-install-scripts mkinitcpio vim nano bash-completion systemd networkmanager");

        execute_command("mount " + efi_part + " /mnt/boot/efi");

        execute_command("chroot /mnt /bin/bash -c \"systemctl enable NetworkManager\"");

        install_grub_ext4(drive);

        create_new_user(fs_type, drive);

        std::cout << COLOR_GREEN << "Arch TTY Grub installation completed successfully!" << COLOR_RESET << std::endl;

        prompt_reboot();
    }

    // Function to install desktop environments
    void install_desktop(const std::string& fs_type, const std::string& drive) {
        std::vector<std::string> desktop_options = {
            "Arch TTY Grub (Complete Installation)",
            "GNOME",
            "KDE Plasma",
            "XFCE",
            "LXQt",
            "Cinnamon",
            "MATE",
            "Budgie",
            "i3 (tiling WM)",
            "Sway (Wayland tiling)",
            "Hyprland (Wayland)",
            "Return to Main Menu"
        };

        int desktop_choice = show_menu(desktop_options, "Select Desktop Environment");

        if (desktop_choice == 11) {
            std::cout << COLOR_CYAN << "Returning to main menu..." << COLOR_RESET << std::endl;
            return;
        }

        desktop_environment = desktop_options[desktop_choice];

        if (desktop_choice == 0) {
            install_arch_tty_grub(drive);
        } else if (desktop_choice == 1) {
            std::cout << COLOR_CYAN << "Installing GNOME Desktop..." << COLOR_RESET << std::endl;

            prepare_target_partitions(drive, "ext4");
            std::string efi_part = drive + "1";
            std::string root_part = drive + "2";

            setup_ext4_filesystem(root_part);
            
            execute_command("cp -r /opt/claudemods-distribution-installer/vconsole.conf /mnt/etc");

            execute_command("cp -r /etc/resolv.conf /mnt/etc");

            execute_command("pacstrap /mnt base gnome gnome-extra gdm grub efibootmgr os-prober arch-install-scripts mkinitcpio " + selected_kernel + " linux-firmware sudo networkmanager");

            execute_command("chroot /mnt /bin/bash -c \"systemctl enable gdm\"");
            execute_command("chroot /mnt /bin/bash -c \"systemctl enable NetworkManager\"");

            execute_command("mount " + efi_part + " /mnt/boot/efi");

            install_grub_ext4(drive);

            create_new_user(fs_type, drive);

            std::cout << COLOR_GREEN << "GNOME installation completed!" << COLOR_RESET << std::endl;

            prompt_reboot();
        } else if (desktop_choice == 2) {
            std::cout << COLOR_CYAN << "Installing KDE Plasma..." << COLOR_RESET << std::endl;

            prepare_target_partitions(drive, "ext4");
            std::string efi_part = drive + "1";
            std::string root_part = drive + "2";

            setup_ext4_filesystem(root_part);
            
            execute_command("cp -r /opt/claudemods-distribution-installer/vconsole.conf /mnt/etc");

            execute_command("cp -r /etc/resolv.conf /mnt/etc");

            execute_command("pacstrap /mnt base plasma sddm dolphin konsole grub efibootmgr os-prober arch-install-scripts mkinitcpio " + selected_kernel + " linux-firmware sudo networkmanager");

            execute_command("chroot /mnt /bin/bash -c \"systemctl enable sddm\"");
            execute_command("chroot /mnt /bin/bash -c \"systemctl enable NetworkManager\"");

            execute_command("mount " + efi_part + " /mnt/boot/efi");

            install_grub_ext4(drive);

            create_new_user(fs_type, drive);

            std::cout << COLOR_GREEN << "KDE Plasma installation completed!" << COLOR_RESET << std::endl;

            prompt_reboot();
        } else if (desktop_choice == 3) {
            std::cout << COLOR_CYAN << "Installing XFCE..." << COLOR_RESET << std::endl;

            prepare_target_partitions(drive, "ext4");
            std::string efi_part = drive + "1";
            std::string root_part = drive + "2";

            setup_ext4_filesystem(root_part);
            
            execute_command("cp -r /opt/claudemods-distribution-installer/vconsole.conf /mnt/etc");

            execute_command("cp -r /etc/resolv.conf /mnt/etc");

            execute_command("pacstrap /mnt base xfce4 xfce4-goodies lightdm lightdm-gtk-greeter grub efibootmgr os-prober arch-install-scripts mkinitcpio " + selected_kernel + " linux-firmware sudo networkmanager");

            execute_command("chroot /mnt /bin/bash -c \"systemctl enable lightdm\"");
            execute_command("chroot /mnt /bin/bash -c \"systemctl enable NetworkManager\"");

            execute_command("mount " + efi_part + " /mnt/boot/efi");

            install_grub_ext4(drive);

            create_new_user(fs_type, drive);

            std::cout << COLOR_GREEN << "XFCE installation completed!" << COLOR_RESET << std::endl;

            prompt_reboot();
        } else if (desktop_choice == 4) {
            std::cout << COLOR_CYAN << "Installing LXQt..." << COLOR_RESET << std::endl;

            prepare_target_partitions(drive, "ext4");
            std::string efi_part = drive + "1";
            std::string root_part = drive + "2";

            setup_ext4_filesystem(root_part);
            
            execute_command("cp -r /opt/claudemods-distribution-installer/vconsole.conf /mnt/etc");

            execute_command("cp -r /etc/resolv.conf /mnt/etc");

            execute_command("pacstrap /mnt base lxqt sddm grub efibootmgr os-prober arch-install-scripts mkinitcpio " + selected_kernel + " linux-firmware sudo networkmanager");

            execute_command("chroot /mnt /bin/bash -c \"systemctl enable sddm\"");
            execute_command("chroot /mnt /bin/bash -c \"systemctl enable NetworkManager\"");

            execute_command("mount " + efi_part + " /mnt/boot/efi");

            install_grub_ext4(drive);

            create_new_user(fs_type, drive);

            std::cout << COLOR_GREEN << "LXQt installation completed!" << COLOR_RESET << std::endl;

            prompt_reboot();
        } else if (desktop_choice == 5) {
            std::cout << COLOR_CYAN << "Installing Cinnamon..." << COLOR_RESET << std::endl;

            prepare_target_partitions(drive, "ext4");
            std::string efi_part = drive + "1";
            std::string root_part = drive + "2";

            setup_ext4_filesystem(root_part);
            
            execute_command("cp -r /opt/claudemods-distribution-installer/vconsole.conf /mnt/etc");

            execute_command("cp -r /etc/resolv.conf /mnt/etc");

            execute_command("pacstrap /mnt base cinnamon lightdm lightdm-gtk-greeter grub efibootmgr os-prober arch-install-scripts mkinitcpio " + selected_kernel + " linux-firmware sudo networkmanager");

            execute_command("chroot /mnt /bin/bash -c \"systemctl enable lightdm\"");
            execute_command("chroot /mnt /bin/bash -c \"systemctl enable NetworkManager\"");

            execute_command("mount " + efi_part + " /mnt/boot/efi");

            install_grub_ext4(drive);

            create_new_user(fs_type, drive);

            std::cout << COLOR_GREEN << "Cinnamon installation completed!" << COLOR_RESET << std::endl;

            prompt_reboot();
        } else if (desktop_choice == 6) {
            std::cout << COLOR_CYAN << "Installing MATE..." << COLOR_RESET << std::endl;

            prepare_target_partitions(drive, "ext4");
            std::string efi_part = drive + "1";
            std::string root_part = drive + "2";

            setup_ext4_filesystem(root_part);
            
            execute_command("cp -r /opt/claudemods-distribution-installer/vconsole.conf /mnt/etc");

            execute_command("cp -r /etc/resolv.conf /mnt/etc");

            execute_command("pacstrap /mnt base mate mate-extra lightdm lightdm-gtk-greeter grub efibootmgr os-prober arch-install-scripts mkinitcpio " + selected_kernel + " linux-firmware sudo networkmanager");

            execute_command("chroot /mnt /bin/bash -c \"systemctl enable lightdm\"");
            execute_command("chroot /mnt /bin/bash -c \"systemctl enable NetworkManager\"");

            execute_command("mount " + efi_part + " /mnt/boot/efi");

            install_grub_ext4(drive);

            create_new_user(fs_type, drive);

            std::cout << COLOR_GREEN << "MATE installation completed!" << COLOR_RESET << std::endl;

            prompt_reboot();
        } else if (desktop_choice == 7) {
            std::cout << COLOR_CYAN << "Installing Budgie..." << COLOR_RESET << std::endl;

            prepare_target_partitions(drive, "ext4");
            std::string efi_part = drive + "1";
            std::string root_part = drive + "2";

            setup_ext4_filesystem(root_part);
            
            execute_command("cp -r /opt/claudemods-distribution-installer/vconsole.conf /mnt/etc");

            execute_command("cp -r /etc/resolv.conf /mnt/etc");

            execute_command("pacstrap /mnt base budgie-desktop lightdm lightdm-gtk-greeter grub efibootmgr os-prober arch-install-scripts mkinitcpio " + selected_kernel + " linux-firmware sudo networkmanager");

            execute_command("chroot /mnt /bin/bash -c \"systemctl enable lightdm\"");
            execute_command("chroot /mnt /bin/bash -c \"systemctl enable NetworkManager\"");

            execute_command("mount " + efi_part + " /mnt/boot/efi");

            install_grub_ext4(drive);

            create_new_user(fs_type, drive);

            std::cout << COLOR_GREEN << "Budgie installation completed!" << COLOR_RESET << std::endl;

            prompt_reboot();
        } else if (desktop_choice == 8) {
            std::cout << COLOR_CYAN << "Installing i3 (tiling WM)..." << COLOR_RESET << std::endl;

            prepare_target_partitions(drive, "ext4");
            std::string efi_part = drive + "1";
            std::string root_part = drive + "2";

            setup_ext4_filesystem(root_part);
            
            execute_command("cp -r /opt/claudemods-distribution-installer/vconsole.conf /mnt/etc");

            execute_command("cp -r /etc/resolv.conf /mnt/etc");

            execute_command("pacstrap /mnt base i3-wm i3status i3lock dmenu lightdm lightdm-gtk-greeter grub efibootmgr os-prober arch-install-scripts mkinitcpio " + selected_kernel + " linux-firmware sudo networkmanager");

            execute_command("chroot /mnt /bin/bash -c \"systemctl enable lightdm\"");
            execute_command("chroot /mnt /bin/bash -c \"systemctl enable NetworkManager\"");

            execute_command("mount " + efi_part + " /mnt/boot/efi");

            install_grub_ext4(drive);

            create_new_user(fs_type, drive);

            std::cout << COLOR_GREEN << "i3 installation completed!" << COLOR_RESET << std::endl;

            prompt_reboot();
        } else if (desktop_choice == 9) {
            std::cout << COLOR_CYAN << "Installing Sway (Wayland tiling)..." << COLOR_RESET << std::endl;

            prepare_target_partitions(drive, "ext4");
            std::string efi_part = drive + "1";
            std::string root_part = drive + "2";

            setup_ext4_filesystem(root_part);
            
            execute_command("cp -r /opt/claudemods-distribution-installer/vconsole.conf /mnt/etc");

            execute_command("cp -r /etc/resolv.conf /mnt/etc");

            execute_command("pacstrap /mnt base sway swaybg waybar wofi lightdm lightdm-gtk-greeter grub efibootmgr os-prober arch-install-scripts mkinitcpio " + selected_kernel + " linux-firmware sudo networkmanager");

            execute_command("chroot /mnt /bin/bash -c \"systemctl enable lightdm\"");
            execute_command("chroot /mnt /bin/bash -c \"systemctl enable NetworkManager\"");

            execute_command("mount " + efi_part + " /mnt/boot/efi");

            install_grub_ext4(drive);

            create_new_user(fs_type, drive);

            std::cout << COLOR_GREEN << "Sway installation completed!" << COLOR_RESET << std::endl;

            prompt_reboot();
        } else if (desktop_choice == 10) {
            std::cout << COLOR_PURPLE << "Installing Hyprland (Modern Wayland Compositor)..." << COLOR_RESET << std::endl;

            prepare_target_partitions(drive, "ext4");
            std::string efi_part = drive + "1";
            std::string root_part = drive + "2";

            setup_ext4_filesystem(root_part);
            
            execute_command("cp -r /opt/claudemods-distribution-installer/vconsole.conf /mnt/etc");

            execute_command("cp -r /etc/resolv.conf /mnt/etc");

            execute_command("pacstrap /mnt base hyprland waybar rofi wl-clipboard sddm grub efibootmgr os-prober arch-install-scripts mkinitcpio " + selected_kernel + " linux-firmware sudo networkmanager");

            execute_command("chroot /mnt /bin/bash -c \"systemctl enable sddm\"");
            execute_command("chroot /mnt /bin/bash -c \"systemctl enable NetworkManager\"");

            execute_command("mount " + efi_part + " /mnt/boot/efi");

            install_grub_ext4(drive);

            create_new_user(fs_type, drive);

            std::cout << COLOR_PURPLE << "Hyprland installed! Note: You may need to configure ~/.config/hypr/hyprland.conf" << COLOR_RESET << std::endl;

            prompt_reboot();
        }
    }

    // Function to install CachyOS TTY Grub
    void install_cachyos_tty_grub(const std::string& drive) {
        std::cout << COLOR_CYAN << "Installing CachyOS TTY Grub..." << COLOR_RESET << std::endl;

        prepare_target_partitions(drive, "ext4");
        std::string efi_part = drive + "1";
        std::string root_part = drive + "2";

        setup_ext4_filesystem(root_part);

        execute_cd_command("cd /mnt");
        execute_command("wget --show-progress --no-check-certificate --continue --tries=10 --timeout=30 --waitretry=5 https://claudemodsreloaded.co.uk/claudemods-rootfs-images/Cachyos-TtyGrub/rootfs.img");
        execute_command("unsquashfs -f -d /mnt /mnt/rootfs.img");
        execute_command("mount " + efi_part + " /mnt/boot/efi");

        install_grub_ext4(drive);

        change_username("ext4", drive);

        std::cout << COLOR_GREEN << "CachyOS TTY Grub installation completed!" << COLOR_RESET << std::endl;

        prompt_reboot();
    }

    // Function to install CachyOS KDE
    void install_cachyos_kde(const std::string& drive) {
        std::cout << COLOR_CYAN << "Installing CachyOS KDE..." << COLOR_RESET << std::endl;

        prepare_target_partitions(drive, "ext4");
        std::string efi_part = drive + "1";
        std::string root_part = drive + "2";

        setup_ext4_filesystem(root_part);

        execute_cd_command("cd /mnt");
        execute_command("wget --show-progress --no-check-certificate --continue --tries=10 --timeout=30 --waitretry=5 https://claudemodsreloaded.co.uk/claudemods-rootfs-images/cachyos-kde/rootfs.img");
        execute_command("unsquashfs -f -d /mnt /mnt/rootfs.img");
        execute_command("mount " + efi_part + " /mnt/boot/efi");

        install_grub_ext4(drive);

        change_username("ext4", drive);

        std::cout << COLOR_GREEN << "CachyOS KDE installation completed!" << COLOR_RESET << std::endl;

        prompt_reboot();
    }

    // Function to install CachyOS GNOME
    void install_cachyos_gnome(const std::string& drive) {
        std::cout << COLOR_CYAN << "Installing CachyOS GNOME..." << COLOR_RESET << std::endl;

        prepare_target_partitions(drive, "ext4");
        std::string efi_part = drive + "1";
        std::string root_part = drive + "2";

        setup_ext4_filesystem(root_part);

        execute_cd_command("cd /mnt");
        execute_command("wget --show-progress --no-check-certificate --continue --tries=10 --timeout=30 --waitretry=5 https://claudemodsreloaded.co.uk/claudemods-rootfs-images/Cachyos-GnomeGrub/rootfs.img");
        execute_command("unsquashfs -f -d /mnt /mnt/rootfs.img");
        execute_command("mount " + efi_part + " /mnt/boot/efi");

        install_grub_ext4(drive);

        change_username("ext4", drive);

        std::cout << COLOR_CYAN << "Setting up CachyOS..." << COLOR_RESET << std::endl;

        std::cout << COLOR_GREEN << "CachyOS GNOME installation completed!" << COLOR_RESET << std::endl;

        prompt_reboot();
    }

    // Function to display Cachyos menu
    void display_cachyos_menu(const std::string& fs_type, const std::string& drive) {
        std::vector<std::string> cachyos_options = {
            "Install CachyOS TTY Grub",
            "Install CachyOS KDE Grub",
            "Install CachyOS GNOME Grub",
            "Return to Main Menu"
        };

        int cachyos_choice = show_menu(cachyos_options, "CachyOS Options");

        if (cachyos_choice == 3) {
            std::cout << COLOR_CYAN << "Returning to main menu..." << COLOR_RESET << std::endl;
            return;
        }

        switch(cachyos_choice) {
            case 0:
                install_cachyos_tty_grub(drive);
                break;
            case 1:
                install_cachyos_kde(drive);
                break;
            case 2:
                install_cachyos_gnome(drive);
                break;
        }
    }

    // Function to install Spitfire CKGE
    void install_spitfire_ckge(const std::string& drive) {
        std::cout << COLOR_ORANGE << "Installing Spitfire CKGE Minimal..." << COLOR_RESET << std::endl;

        prepare_target_partitions(drive, "ext4");
        std::string efi_part = drive + "1";
        std::string root_part = drive + "2";

        setup_ext4_filesystem(root_part);

        // Use execute_cd_command for cd commands
        execute_cd_command("cd /mnt");
        execute_command("wget --show-progress --no-check-certificate --continue --tries=10 --timeout=30 --waitretry=5 https://claudemodsreloaded.co.uk/claudemods-desktop/desktopminimal.img");
        execute_command("unsquashfs -f -d /mnt /mnt/desktopminimal.img");
        execute_command("mv /mnt/cachyos /mnt/home/" + new_username);
        execute_command("unzip -o /opt/claudemods-distribution-installer/pacman.d.zip -d /mnt/etc");
        execute_command("unzip -o /opt/claudemods-distribution-installer/pacman.d.zip -d /etc");
        execute_command("cp -r /opt/claudemods-distribution-installer/pacman.conf /mnt/etc");
        execute_command("cp -r /opt/claudemods-distribution-installer/pacman.conf /etc");
        execute_command("cp -r /etc/resolv.conf /mnt/etc");
        execute_command("cp -r /opt/claudemods-distribution-installer/vconsole.conf /mnt/etc");
        execute_command("mkdir -p /mnt/boot");
        execute_command("mkdir -p /mnt/boot/grub");
        execute_command("touch /mnt/boot/grub/grub.cfg.new");
        execute_command("pacman -Sy");

        execute_command("pacstrap /mnt claudemods-desktop");

        execute_command("chroot /mnt /bin/bash -c \"systemctl enable sddm\"");
        execute_command("chroot /mnt /bin/bash -c \"systemctl enable NetworkManager\"");

        execute_command("mount " + efi_part + " /mnt/boot/efi");

        install_grub_ext4(drive);

        create_new_user(fs_type, drive);

        execute_command("mkdir -p /mnt/opt/Arch-Systemtool");
        execute_command("cp -r /opt/claudemods-distribution-installer/spitfire-ckge-minimal/desktop.sh /mnt/opt/Arch-Systemtool");
        execute_command("chmod +x /mnt/opt/Arch-Systemtool/desktop.sh");
        execute_command("chroot /mnt /bin/bash -c \"su - " + new_username + " -c 'cd /home/" + new_username + " && git clone https://github.com/claudemods/claudemods-distribution-installer'\"");
        execute_command("chroot /mnt /bin/bash -c \"su - " + new_username + " -c 'cd /home/" + new_username + "/claudemods-distribution-installer/installer && chmod +x dolphinfixes.sh'\"");
        execute_command("chroot /mnt /bin/bash -c \"su - " + new_username + " -c 'cd /home/" + new_username + "/claudemods-distribution-installer/installer && ./dolphinfixes.sh " + new_username + "'\"");
        execute_command("chroot /mnt /bin/bash -c \"su - " + new_username + " -c 'cd /home/" + new_username + "/claudemods-distribution-installer/installer/spitfire-ckge-minimal && chmod +x installspitfireupdated.sh'\"");
        execute_command("chroot /mnt /bin/bash -c \"su - " + new_username + " -c 'cd /home/" + new_username + "/claudemods-distribution-installer/installer/spitfire-ckge-minimal && ./installspitfireupdated.sh " + new_username + "'\"");
        execute_command("cp -r /opt/claudemods-distribution-installer/spitfire-ckge-minimal/konsolerc /mnt/home/" + new_username + "/.config/");
        std::cout << COLOR_ORANGE << "Spitfire CKGE Minimal installation completed!" << COLOR_RESET << std::endl;

        prompt_reboot();
    }

    // Function to install Apex CKGE
    void install_apex_ckge(const std::string& drive) {
        std::cout << COLOR_PURPLE << "Installing Apex CKGE Minimal..." << COLOR_RESET << std::endl;

        prepare_target_partitions(drive, "ext4");
        std::string efi_part = drive + "1";
        std::string root_part = drive + "2";

        setup_ext4_filesystem(root_part);

        execute_cd_command("cd /mnt");
        execute_command("wget --show-progress --no-check-certificate --continue --tries=3 --timeout=30 --waitretry=5 https://claudemodsreloaded.co.uk/claudemods-rootfs-images/claudemods-apex-ckge-minimal/apex.img");
        execute_command("unsquashfs -f -d /mnt /mnt/apex.img");
        execute_command("mount " + efi_part + " /mnt/boot/efi");

        install_grub_ext4(drive);

        change_username("ext4", drive);
        execute_command("cp -r /etc/resolv.conf /mnt/etc");
        execute_command("chroot /mnt /bin/bash -c \"su - " + new_username + " -c 'cd /home/" + new_username + " && git clone https://github.com/claudemods/claudemods-distribution-installer'\"");
        execute_command("chroot /mnt /bin/bash -c \"su - " + new_username + " -c 'cd /home/" + new_username + "/claudemods-distribution-installer/installer && chmod +x dolphinfixes.sh'\"");
        execute_command("chroot /mnt /bin/bash -c \"su - " + new_username + " -c 'cd /home/" + new_username + "/claudemods-distribution-installer/installer && ./dolphinfixes.sh " + new_username + "'\"");

        execute_command("chroot /mnt /bin/bash -c \"su - " + new_username + " -c 'cd /home/" + new_username + "/claudemods-distribution-installer/installer && chmod +x cleanup.sh'\"");
        execute_command("chroot /mnt /bin/bash -c \"su - " + new_username + " -c 'cd /home/" + new_username + "/claudemods-distribution-installer/installer && ./cleanup.sh'\"");

        std::cout << COLOR_PURPLE << "Apex CKGE Minimal installation completed!" << COLOR_RESET << std::endl;

        prompt_reboot();
    }

    // Function to install Spitfire CKGE Full
    void install_spitfire_ckge_full(const std::string& drive) {
        std::cout << COLOR_ORANGE << "Installing Spitfire CKGE Full..." << COLOR_RESET << std::endl;

        prepare_target_partitions(drive, "ext4");
        std::string efi_part = drive + "1";
        std::string root_part = drive + "2";

        setup_ext4_filesystem(root_part);

        // Use execute_cd_command for cd commands
        execute_cd_command("cd /mnt");
        execute_command("wget --show-progress --no-check-certificate --continue --tries=3 --timeout=30 --waitretry=5 https://claudemodsreloaded.co.uk/claudemods-rootfs-images/claudemods-apex-ckge-minimal/apex.img");
        execute_command("unsquashfs -f -d /mnt /mnt/apex.img");
        execute_command("mount " + efi_part + " /mnt/boot/efi");

        install_grub_ext4(drive);

        change_username("ext4", drive);
        execute_command("cp -r /etc/resolv.conf /mnt/etc");
        execute_command("cp -r /opt/claudemods-distribution-installer/spitfire-ckge-minimal/desktop.sh /mnt/opt/Arch-Systemtool");
        execute_command("chmod +x /mnt/opt/Arch-Systemtool/desktop.sh");
        execute_command("chroot /mnt /bin/bash -c \"su - " + new_username + " -c 'cd /home/" + new_username + " && git clone https://github.com/claudemods/claudemods-distribution-installer'\"");
        execute_command("chroot /mnt /bin/bash -c \"su - " + new_username + " -c 'cd /home/" + new_username + "/claudemods-distribution-installer/installer && chmod +x dolphinfixes.sh'\"");
        execute_command("chroot /mnt /bin/bash -c \"su - " + new_username + " -c 'cd /home/" + new_username + "/claudemods-distribution-installer/installer && ./dolphinfixes.sh " + new_username + "'\"");
        execute_command("chroot /mnt /bin/bash -c \"su - " + new_username + " -c 'cd /home/" + new_username + "/claudemods-distribution-installer/installer/spitfire-ckge-minimal && chmod +x installspitfire.sh'\"");
        execute_command("chroot /mnt /bin/bash -c \"su - " + new_username + " -c 'cd /home/" + new_username + "/claudemods-distribution-installer/installer/spitfire-ckge-minimal && ./installspitfire.sh " + new_username + "'\"");

        std::cout << COLOR_ORANGE << "Spitfire CKGE Full installation completed!" << COLOR_RESET << std::endl;

        prompt_reboot();
    }

    // Function to install Apex CKGE Full
    void install_apex_ckge_full(const std::string& drive) {
        std::cout << COLOR_PURPLE << "Installing Apex CKGE Full..." << COLOR_RESET << std::endl;

        prepare_target_partitions(drive, "ext4");
        std::string efi_part = drive + "1";
        std::string root_part = drive + "2";

        setup_ext4_filesystem(root_part);

        execute_cd_command("cd /mnt");
        execute_command("wget --show-progress --no-check-certificate --continue --tries=3 --timeout=30 --waitretry=5 https://claudemodsreloaded.co.uk/claudemods-rootfs-images/claudemods-apex-ckge-minimal/apex.img");
        execute_command("unsquashfs -f -d /mnt /mnt/apex.img");
        execute_command("mount " + efi_part + " /mnt/boot/efi");

        install_grub_ext4(drive);

        change_username("ext4", drive);
        execute_command("cp -r /etc/resolv.conf /mnt/etc");
        execute_command("chroot /mnt /bin/bash -c \"su - " + new_username + " -c 'cd /home/" + new_username + " && git clone https://github.com/claudemods/claudemods-distribution-installer'\"");
        execute_command("chroot /mnt /bin/bash -c \"su - " + new_username + " -c 'cd /home/" + new_username + "/claudemods-distribution-installer/installer && chmod +x dolphinfixes.sh'\"");
        execute_command("chroot /mnt /bin/bash -c \"su - " + new_username + " -c 'cd /home/" + new_username + "/claudemods-distribution-installer/installer && ./dolphinfixes.sh " + new_username + "'\"");

        execute_command("chroot /mnt /bin/bash -c \"su - " + new_username + " -c 'cd /home/" + new_username + "/claudemods-distribution-installer/installer && chmod +x cleanup.sh'\"");
        execute_command("chroot /mnt /bin/bash -c \"su - " + new_username + " -c 'cd /home/" + new_username + "/claudemods-distribution-installer/installer && ./cleanup.sh'\"");

        std::cout << COLOR_PURPLE << "Apex CKGE Full installation completed!" << COLOR_RESET << std::endl;

        prompt_reboot();
    }

    // Function to install Spitfire CKGE Minimal Dev
    void install_spitfire_ckge_minimal_dev(const std::string& drive) {
        std::cout << COLOR_ORANGE << "Installing Spitfire CKGE Minimal Dev..." << COLOR_RESET << std::endl;

        prepare_target_partitions(drive, "ext4");
        std::string efi_part = drive + "1";
        std::string root_part = drive + "2";

        setup_ext4_filesystem(root_part);

        // Use execute_cd_command for cd commands
        execute_cd_command("cd /mnt");
        execute_command("wget --show-progress --no-check-certificate --continue --tries=10 --timeout=30 --waitretry=5 https://claudemodsreloaded.co.uk/claudemods-rootfs-images/claudemods-apex-ckge-minimal/apex.img");
        execute_command("unsquashfs -f -d /mnt /mnt/apex.img");
        execute_command("mount " + efi_part + " /mnt/boot/efi");

        install_grub_ext4(drive);

        change_username("ext4", drive);
        execute_command("cp -r /etc/resolv.conf /mnt/etc");
        execute_command("sudo -S cp -r /opt/claudemods-distribution-installer/pacman.conf /mnt/etc");
        execute_command("cp -r /opt/claudemods-distribution-installer/spitfire-ckge-minimal/desktop.sh /mnt/opt/Arch-Systemtool");
        execute_command("chmod +x /mnt/opt/Arch-Systemtool/desktop.sh");
        execute_command("chroot /mnt /bin/bash -c \"su - " + new_username + " -c 'cd /home/" + new_username + " && git clone https://github.com/claudemods/claudemods-distribution-installer'\"");
        execute_command("chroot /mnt /bin/bash -c \"su - " + new_username + " -c 'cd /home/" + new_username + "/claudemods-distribution-installer/installer && chmod +x dolphinfixes.sh'\"");
        execute_command("chroot /mnt /bin/bash -c \"su - " + new_username + " -c 'cd /home/" + new_username + "/claudemods-distribution-installer/installer && ./dolphinfixes.sh " + new_username + "'\"");
        execute_command("chroot /mnt /bin/bash -c \"su - " + new_username + " -c 'cd /home/" + new_username + "/claudemods-distribution-installer/installer/spitfire-ckge-minimal && chmod +x installspitfire.sh'\"");
        execute_command("chroot /mnt /bin/bash -c \"su - " + new_username + " -c 'cd /home/" + new_username + "/claudemods-distribution-installer/installer/spitfire-ckge-minimal && ./installspitfire.sh " + new_username + "'\"");
        execute_command("chroot /mnt /bin/bash -c \"pacman -Sy\"");
        execute_command("chroot /mnt /bin/bash -c \"pacman -Syu --noconfirm\"");
        execute_command("chroot /mnt /bin/bash -c \"pacman -Rsc --noconfirm phonon-qt6\"");
        execute_command("chroot /mnt /bin/bash -c \"pacman -Rsc --noconfirm breeze-gtk\"");
        execute_command("chroot /mnt /bin/bash -c \"pacman -Rsc --noconfirm kcoreaddons\"");
        execute_command("chroot /mnt /bin/bash -c \"pacman -Rsc --noconfirm kconfig\"");
        execute_command("chroot /mnt /bin/bash -c \"pacman -Rsc --noconfirm karchive\"");
        execute_command("chroot /mnt /bin/bash -c \"pacman -Rsc --noconfirm raptor\"");
        execute_command("chroot /mnt /bin/bash -c \"pacman -Rsc --noconfirm poppler-glib\"");
        execute_command("chroot /mnt /bin/bash -c \"pacman -Rsc --noconfirm harfbuzz-icu\"");
        execute_command("chroot /mnt /bin/bash -c \"pacman -Rsc --noconfirm ark karchive plasma-desktop dolphin kate konsole attica knewstuff discover\"");
        execute_command("chroot /mnt /bin/bash -c \"pacman -Sy\"");
        execute_command("chroot /mnt /bin/bash -c \"pacman -S --noconfirm kdedevpackages kate\"");

        std::cout << COLOR_ORANGE << "Spitfire CKGE Minimal Dev installation completed!" << COLOR_RESET << std::endl;

        prompt_reboot();
    }

    // Function to install Apex CKGE Minimal Dev
    void install_apex_ckge_minimal_dev(const std::string& drive) {
        std::cout << COLOR_PURPLE << "Installing Apex CKGE Minimal Dev..." << COLOR_RESET << std::endl;

        prepare_target_partitions(drive, "ext4");
        std::string efi_part = drive + "1";
        std::string root_part = drive + "2";

        setup_ext4_filesystem(root_part);

        execute_cd_command("cd /mnt");
        execute_command("wget --show-progress --no-check-certificate --continue --tries=3 --timeout=30 --waitretry=5 https://claudemodsreloaded.co.uk/claudemods-rootfs-images/claudemods-apex-ckge-minimal/apex.img");
        execute_command("unsquashfs -f -d /mnt /mnt/apex.img");
        execute_command("mount " + efi_part + " /mnt/boot/efi");

        install_grub_ext4(drive);

        change_username("ext4", drive);
        execute_command("cp -r /etc/resolv.conf /mnt/etc");
        execute_command("chroot /mnt /bin/bash -c \"su - " + new_username + " -c 'cd /home/" + new_username + " && git clone https://github.com/claudemods/claudemods-distribution-installer'\"");
        execute_command("chroot /mnt /bin/bash -c \"su - " + new_username + " -c 'cd /home/" + new_username + "/claudemods-distribution-installer/installer && chmod +x dolphinfixes.sh'\"");
        execute_command("chroot /mnt /bin/bash -c \"su - " + new_username + " -c 'cd /home/" + new_username + "/claudemods-distribution-installer/installer && ./dolphinfixes.sh " + new_username + "'\"");

        execute_command("chroot /mnt /bin/bash -c \"su - " + new_username + " -c 'cd /home/" + new_username + "/claudemods-distribution-installer/installer && chmod +x cleanup.sh'\"");
        execute_command("chroot /mnt /bin/bash -c \"su - " + new_username + " -c 'cd /home/" + new_username + "/claudemods-distribution-installer/installer && ./cleanup.sh'\"");

        std::cout << COLOR_PURPLE << "Apex CKGE Minimal Dev installation completed!" << COLOR_RESET << std::endl;

        prompt_reboot();
    }

    // Function to install Spitfire CKGE Full Dev
    void install_spitfire_ckge_full_dev(const std::string& drive) {
        std::cout << COLOR_ORANGE << "Installing Spitfire CKGE Full Dev..." << COLOR_RESET << std::endl;

        prepare_target_partitions(drive, "ext4");
        std::string efi_part = drive + "1";
        std::string root_part = drive + "2";

        setup_ext4_filesystem(root_part);

        // Use execute_cd_command for cd commands
        execute_cd_command("cd /mnt");
        execute_command("wget --show-progress --no-check-certificate --continue --tries=3 --timeout=30 --waitretry=5 https://claudemodsreloaded.co.uk/claudemods-rootfs-images/claudemods-apex-ckge-minimal/apex.img");
        execute_command("unsquashfs -f -d /mnt /mnt/apex.img");
        execute_command("mount " + efi_part + " /mnt/boot/efi");

        install_grub_ext4(drive);

        change_username("ext4", drive);
        execute_command("cp -r /etc/resolv.conf /mnt/etc");
        execute_command("cp -r /opt/claudemods-distribution-installer/spitfire-ckge-minimal/desktop.sh /mnt/opt/Arch-Systemtool");
        execute_command("chmod +x /mnt/opt/Arch-Systemtool/desktop.sh");
        execute_command("chroot /mnt /bin/bash -c \"su - " + new_username + " -c 'cd /home/" + new_username + " && git clone https://github.com/claudemods/claudemods-distribution-installer'\"");
        execute_command("chroot /mnt /bin/bash -c \"su - " + new_username + " -c 'cd /home/" + new_username + "/claudemods-distribution-installer/installer && chmod +x dolphinfixes.sh'\"");
        execute_command("chroot /mnt /bin/bash -c \"su - " + new_username + " -c 'cd /home/" + new_username + "/claudemods-distribution-installer/installer && ./dolphinfixes.sh " + new_username + "'\"");
        execute_command("chroot /mnt /bin/bash -c \"su - " + new_username + " -c 'cd /home/" + new_username + "/claudemods-distribution-installer/installer/spitfire-ckge-minimal && chmod +x installspitfire.sh'\"");
        execute_command("chroot /mnt /bin/bash -c \"su - " + new_username + " -c 'cd /home/" + new_username + "/claudemods-distribution-installer/installer/spitfire-ckge-minimal && ./installspitfire.sh " + new_username + "'\"");

        std::cout << COLOR_ORANGE << "Spitfire CKGE Full Dev installation completed!" << COLOR_RESET << std::endl;

        prompt_reboot();
    }

    // Function to install Apex CKGE Full Dev
    void install_apex_ckge_full_dev(const std::string& drive) {
        std::cout << COLOR_PURPLE << "Installing Apex CKGE Full Dev..." << COLOR_RESET << std::endl;

        prepare_target_partitions(drive, "ext4");
        std::string efi_part = drive + "1";
        std::string root_part = drive + "2";

        setup_ext4_filesystem(root_part);

        execute_cd_command("cd /mnt");
        execute_command("wget --show-progress --no-check-certificate --continue --tries=3 --timeout=30 --waitretry=5 https://claudemodsreloaded.co.uk/claudemods-rootfs-images/claudemods-apex-ckge-minimal/apex.img");
        execute_command("unsquashfs -f -d /mnt /mnt/apex.img");
        execute_command("mount " + efi_part + " /mnt/boot/efi");

        install_grub_ext4(drive);

        change_username("ext4", drive);
        execute_command("cp -r /etc/resolv.conf /mnt/etc");
        execute_command("chroot /mnt /bin/bash -c \"su - " + new_username + " -c 'cd /home/" + new_username + " && git clone https://github.com/claudemods/claudemods-distribution-installer'\"");
        execute_command("chroot /mnt /bin/bash -c \"su - " + new_username + " -c 'cd /home/" + new_username + "/claudemods-distribution-installer/installer && chmod +x dolphinfixes.sh'\"");
        execute_command("chroot /mnt /bin/bash -c \"su - " + new_username + " -c 'cd /home/" + new_username + "/claudemods-distribution-installer/installer && ./dolphinfixes.sh " + new_username + "'\"");

        execute_command("chroot /mnt /bin/bash -c \"su - " + new_username + " -c 'cd /home/" + new_username + "/claudemods-distribution-installer/installer && chmod +x cleanup.sh'\"");
        execute_command("chroot /mnt /bin/bash -c \"su - " + new_username + " -c 'cd /home/" + new_username + "/claudemods-distribution-installer/installer && ./cleanup.sh'\"");

        std::cout << COLOR_PURPLE << "Apex CKGE Full Dev installation completed!" << COLOR_RESET << std::endl;

        prompt_reboot();
    }

    // Function to display claudemods Distribution menu
    void display_claudemods_menu(const std::string& fs_type, const std::string& drive) {
        std::vector<std::string> claudemods_options = {
            "Install Spitfire CKGE Minimal",
            "Install Spitfire CKGE Full",
            "Install Spitfire CKGE Minimal Dev",
            "Install Spitfire CKGE Full Dev",
            "Install Apex CKGE Minimal",
            "Install Apex CKGE Full",
            "Install Apex CKGE Minimal Dev",
            "Install Apex CKGE Full Dev",
            "Return to Main Menu"
        };

        int claudemods_choice = show_menu(claudemods_options, "claudemods Distribution Options");

        if (claudemods_choice == 8) {
            std::cout << COLOR_CYAN << "Returning to main menu..." << COLOR_RESET << std::endl;
            return;
        }

        switch(claudemods_choice) {
            case 0: install_spitfire_ckge(drive); break;
            case 1: install_spitfire_ckge_full(drive); break;
            case 2: install_spitfire_ckge_minimal_dev(drive); break;
            case 3: install_spitfire_ckge_full_dev(drive); break;
            case 4: install_apex_ckge(drive); break;
            case 5: install_apex_ckge_full(drive); break;
            case 6: install_apex_ckge_minimal_dev(drive); break;
            case 7: install_apex_ckge_full_dev(drive); break;
        }
    }

    // Function to display main menu with arrow keys - MODIFIED to remember position
    void main_menu() {
        // Initialize default values
        if (selected_drive.empty()) selected_drive = "Not set";
        if (fs_type.empty()) fs_type = "Not set";
        if (selected_kernel.empty()) selected_kernel = "Not set";
        if (new_username.empty()) new_username = "Not set";
        if (timezone.empty()) timezone = "Not set";
        if (keyboard_layout.empty()) keyboard_layout = "Not set";
        if (installation_type.empty()) installation_type = "Not set";
        desktop_environment = "Not set";

        int current_selection = 0; // Track current menu position

        while (true) {
            std::vector<std::string> main_options = {
                "Drive: " + selected_drive,
                "Filesystem: " + fs_type,
                "Kernel: " + selected_kernel,
                "Username: " + new_username,
                "Timezone: " + timezone,
                "Keyboard: " + keyboard_layout,
                "Installation Type: " + installation_type,
                "Desktop Environment: " + desktop_environment,
                "Confirm and Install",
                "Reboot System",
                "Exit"
            };

            int choice = show_menu(main_options, "Arch Linux Installer - Main Menu", current_selection);

            // Update current selection to remember position
            current_selection = choice;

            switch(choice) {
                case 0: // Drive
                    display_available_drives();
                    selected_drive = get_input("Enter target drive (e.g., /dev/sda): ");
                    if (!is_block_device(selected_drive)) {
                        std::cerr << COLOR_RED << "Error: " << selected_drive << " is not a valid block device" << COLOR_RESET << std::endl;
                        selected_drive = "Invalid - try again";
                    }
                    break;

                case 1: // Filesystem
                {
                    std::vector<std::string> fs_options = {"ext4", "btrfs"};
                    int fs_choice = show_menu(fs_options, "Select Filesystem Type");
                    fs_type = fs_options[fs_choice];

                    // In the filesystem selection case (around line 1251):
                    if (fs_type == "btrfs") {
                        std::cout << COLOR_CYAN << "Starting Btrfs installer with drive: " << selected_drive << COLOR_RESET << std::endl;

                        // Create command line arguments for BtrfsInstaller
                        int argc = 2;
                        char arg1[] = "btrfsinstaller";
                        char* arg2 = const_cast<char*>(selected_drive.c_str());
                        char* argv[] = {arg1, arg2, nullptr};

                        // Create and run BtrfsInstaller with correct arguments
                        BtrfsInstaller btrfs_installer;
                        btrfs_installer.run(argc, argv);

                        // BtrfsInstaller will handle everything - when it returns, we're done
                        return;
                    }
                }
                break;

                case 2: // Kernel
                {
                    std::vector<std::string> kernel_options = {
                        "linux (Standard)",
                        "linux-lts (Long Term Support)",
                        "linux-zen (Tuned for desktop performance)",
                        "linux-hardened (Security-focused)"
                    };
                    int kernel_choice = show_menu(kernel_options, "Select Kernel");

                    switch(kernel_choice) {
                        case 0: selected_kernel = "linux"; break;
                        case 1: selected_kernel = "linux-lts"; break;
                        case 2: selected_kernel = "linux-zen"; break;
                        case 3: selected_kernel = "linux-hardened"; break;
                    }
                }
                break;

                        case 3: // Username
                            new_username = get_input("Enter new username: ");
                            root_password = get_input("Enter root password: ");
                            user_password = get_input("Enter password for user '" + new_username + "': ");
                            break;

                        case 4: // Timezone
                        {
                            std::vector<std::string> timezone_options = {
                                "America/New_York (US English)",
                                "Europe/London (UK English)",
                                "Europe/Berlin (German)",
                                "Europe/Paris (French)",
                                "Europe/Madrid (Spanish)",
                                "Europe/Rome (Italian)",
                                "Asia/Tokyo (Japanese)",
                                "Other (manual entry)"
                            };

                            int timezone_choice = show_menu(timezone_options, "Select Timezone");

                            switch(timezone_choice) {
                                case 0: timezone = "America/New_York"; break;
                                case 1: timezone = "Europe/London"; break;
                                case 2: timezone = "Europe/Berlin"; break;
                                case 3: timezone = "Europe/Paris"; break;
                                case 4: timezone = "Europe/Madrid"; break;
                                case 5: timezone = "Europe/Rome"; break;
                                case 6: timezone = "Asia/Tokyo"; break;
                                case 7: timezone = get_input("Enter timezone (e.g., Europe/Berlin): "); break;
                            }
                        }
                        break;

                                case 5: // Keyboard
                                {
                                    std::vector<std::string> keyboard_options = {
                                        "us (US English)",
                                        "uk (UK English)",
                                        "de (German)",
                                        "fr (French)",
                                        "es (Spanish)",
                                        "it (Italian)",
                                        "jp (Japanese)",
                                        "Other (manual entry)"
                                    };

                                    int keyboard_choice = show_menu(keyboard_options, "Select Keyboard Layout");

                                    switch(keyboard_choice) {
                                        case 0: keyboard_layout = "us"; break;
                                        case 1: keyboard_layout = "uk"; break;
                                        case 2: keyboard_layout = "de"; break;
                                        case 3: keyboard_layout = "fr"; break;
                                        case 4: keyboard_layout = "es"; break;
                                        case 5: keyboard_layout = "it"; break;
                                        case 6: keyboard_layout = "jp"; break;
                                        case 7: keyboard_layout = get_input("Enter keyboard layout (e.g., br, ru, pt): "); break;
                                    }
                                }
                                break;

                                        case 6: // Installation Type
                                        {
                                            std::vector<std::string> install_options = {
                                                "Vanilla Arch Desktop",
                                                "CachyOS",
                                                "claudemods Distribution"
                                            };

                                            int install_choice = show_menu(install_options, "Select Installation Type");

                                            switch(install_choice) {
                                                case 0:
                                                    installation_type = "Vanilla Arch";
                                                    // Show desktop environment selection for Vanilla Arch
                                                    {
                                                        std::vector<std::string> desktop_options = {
                                                            "Arch TTY Grub (Complete Installation)",
                                                            "GNOME",
                                                            "KDE Plasma",
                                                            "XFCE",
                                                            "LXQt",
                                                            "Cinnamon",
                                                            "MATE",
                                                            "Budgie",
                                                            "i3 (tiling WM)",
                                                            "Sway (Wayland tiling)",
                                                            "Hyprland (Wayland)"
                                                        };

                                                        int desktop_choice = show_menu(desktop_options, "Select Desktop Environment");
                                                        desktop_environment = desktop_options[desktop_choice];
                                                    }
                                                    break;
                                                case 1:
                                                    installation_type = "CachyOS";
                                                    // Show CachyOS options
                                                    {
                                                        std::vector<std::string> cachyos_options = {
                                                            "CachyOS TTY Grub",
                                                            "CachyOS KDE Grub",
                                                            "CachyOS GNOME Grub"
                                                        };

                                                        int cachyos_choice = show_menu(cachyos_options, "Select CachyOS Variant");
                                                        desktop_environment = cachyos_options[cachyos_choice];
                                                    }
                                                    break;
                                                case 2:
                                                    installation_type = "claudemods";
                                                    // Show claudemods options
                                                    {
                                                        std::vector<std::string> claudemods_options = {
                                                            "Spitfire CKGE Minimal",
                                                            "Spitfire CKGE Full",
                                                            "Spitfire CKGE Minimal Dev",
                                                            "Spitfire CKGE Full Dev",
                                                            "Apex CKGE Minimal",
                                                            "Apex CKGE Full",
                                                            "Apex CKGE Minimal Dev",
                                                            "Apex CKGE Full Dev"
                                                        };

                                                        int claudemods_choice = show_menu(claudemods_options, "Select claudemods Variant");
                                                        desktop_environment = claudemods_options[claudemods_choice];
                                                    }
                                                    break;
                                            }
                                        }
                                        break;

                                                case 7: // Desktop Environment
                                                    // Only allow changing desktop environment if Vanilla Arch is selected
                                                    if (installation_type == "Vanilla Arch") {
                                                        std::vector<std::string> desktop_options = {
                                                            "Arch TTY Grub (Complete Installation)",
                                                            "GNOME",
                                                            "KDE Plasma",
                                                            "XFCE",
                                                            "LXQt",
                                                            "Cinnamon",
                                                            "MATE",
                                                            "Budgie",
                                                            "i3 (tiling WM)",
                                                            "Sway (Wayland tiling)",
                                                            "Hyprland (Wayland)"
                                                        };

                                                        int desktop_choice = show_menu(desktop_options, "Select Desktop Environment");
                                                        desktop_environment = desktop_options[desktop_choice];
                                                    } else {
                                                        std::cout << COLOR_YELLOW << "Desktop environment is determined by the installation type selection." << COLOR_RESET << std::endl;
                                                        std::cout << COLOR_YELLOW << "Change 'Installation Type' to modify desktop environment." << COLOR_RESET << std::endl;
                                                        // Small delay to show message
                                                        system("sleep 2");
                                                    }
                                                    break;

                                                case 8: // Confirm and Install
                                                    if (show_confirmation_screen()) {
                                                        start_installation();
                                                    }
                                                    break;

                                                case 9: // Reboot
                                                    std::cout << COLOR_GREEN << "Rebooting system..." << COLOR_RESET << std::endl;
                                                    execute_command("sudo reboot");
                                                    break;

                                                case 10: // Exit
                                                    std::cout << COLOR_GREEN << "Exiting. Goodbye!" << COLOR_RESET << std::endl;
                                                    exit(0);
                                                    break;
            }
        }
    }

    // Function to start the actual installation based on selections
    void start_installation() {
        std::cout << COLOR_GREEN << "Starting installation with selected options..." << COLOR_RESET << std::endl;

        // Check if all required fields are set
        if (selected_drive == "Not set" || selected_drive.empty()) {
            std::cerr << COLOR_RED << "Error: Drive not selected" << COLOR_RESET << std::endl;
            return;
        }

        if (installation_type == "Vanilla Arch") {
            if (desktop_environment.find("Arch TTY Grub") != std::string::npos) {
                install_arch_tty_grub(selected_drive);
            } else if (desktop_environment.find("GNOME") != std::string::npos) {
                install_desktop("ext4", selected_drive);
            } else {
                install_desktop("ext4", selected_drive);
            }
        } else if (installation_type == "CachyOS") {
            if (desktop_environment.find("TTY Grub") != std::string::npos) {
                install_cachyos_tty_grub(selected_drive);
            } else if (desktop_environment.find("KDE") != std::string::npos) {
                install_cachyos_kde(selected_drive);
            } else if (desktop_environment.find("GNOME") != std::string::npos) {
                install_cachyos_gnome(selected_drive);
            }
        } else if (installation_type == "claudemods") {
            // Spitfire installations
            if (desktop_environment == "Spitfire CKGE Minimal") {
                install_spitfire_ckge(selected_drive);
            } else if (desktop_environment == "Spitfire CKGE Full") {
                install_spitfire_ckge_full(selected_drive);
            } else if (desktop_environment == "Spitfire CKGE Minimal Dev") {
                install_spitfire_ckge_minimal_dev(selected_drive);
            } else if (desktop_environment == "Spitfire CKGE Full Dev") {
                install_spitfire_ckge_full_dev(selected_drive);
            }
            // Apex installations
            else if (desktop_environment == "Apex CKGE Minimal") {
                install_apex_ckge(selected_drive);
            } else if (desktop_environment == "Apex CKGE Full") {
                install_apex_ckge_full(selected_drive);
            } else if (desktop_environment == "Apex CKGE Minimal Dev") {
                install_apex_ckge_minimal_dev(selected_drive);
            } else if (desktop_environment == "Apex CKGE Full Dev") {
                install_apex_ckge_full_dev(selected_drive);
            }
        }
    } // Add this closing brace for start_installation()

public:
    // Main script
    void run() {
        // Check for updates first
        if (check_for_updates()) {
            // If update was successful and returned true, exit so the updated version can run
            exit(0);
        }

        display_header();

        // Show main menu with arrow key navigation
        main_menu();
    }
}; // Add this closing brace for the class

int main() {
    ArchInstaller installer;
    installer.run();
    return 0;
}
