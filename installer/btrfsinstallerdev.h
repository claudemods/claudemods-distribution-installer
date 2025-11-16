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
#include <unistd.h>
#include <termios.h>

class BtrfsInstaller {
private:
    // Color constants
    const std::string COLOR_RED = "\033[31m";
    const std::string COLOR_GREEN = "\033[32m";
    const std::string COLOR_YELLOW = "\033[33m";
    const std::string COLOR_CYAN = "\033[38;2;0;255;255m";
    const std::string COLOR_ORANGE = "\033[38;5;214m";
    const std::string COLOR_PURPLE = "\033[38;5;93m";
    const std::string COLOR_RESET = "\033[0m";

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
        std::cout << COLOR_CYAN << "claudemods Distribution Installer Btrfs v1.01 11-11-2025" << COLOR_RESET << std::endl;
        std::cout << COLOR_CYAN << "Supports Btrfs (with Zstd compression 22) filesystem" << COLOR_RESET << std::endl;
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
        std::cout << COLOR_RESET;

        std::cout << COLOR_YELLOW;
        std::cout << "╚══════════════════════════════════════════════════════════════╝" << std::endl;
        std::cout << COLOR_RESET << std::endl;
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
            std::cerr << COLOR_RED << "Error: Failed to create partitions" << COLOR_RESET << std::endl;
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
        execute_command("mkdir -p /mnt/{home,root,etc,srv,tmp,var/{cache,log},var/lib/{portables,machines},boot/efi}");

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
        execute_command("cp -r /opt/claudemods-distribution-installer/btrfsfstabcompressed.sh /mnt/opt");
        execute_command("chmod +x /mnt/opt/btrfsfstabcompressed.sh");
        execute_command("chroot /mnt /bin/bash -c \"mount -t efivarfs efivarfs /sys/firmware/efi/efivars\"");
        execute_command("chroot /mnt /bin/bash -c \"genfstab -U / >> /etc/fstab\"");
        execute_command("chroot /mnt /bin/bash -c \"grub-install --target=x86_64-efi --efi-directory=/boot/efi --bootloader-id=GRUB --recheck\"");
        execute_command("chroot /mnt /bin/bash -c \"grub-mkconfig -o /boot/grub/grub.cfg\"");
        execute_command("chroot /mnt /bin/bash -c \"/opt/btrfsfstabcompressed.sh\"");
        execute_command("chroot /mnt /bin/bash -c \"mkinitcpio -P\"");
    }

    // Function to get drive selection - Modified to accept command line argument
    void get_drive_selection(int argc, char* argv[]) {
        // Check if drive was provided as command line argument
        if (argc >= 2) {
            selected_drive = argv[1];
            std::cout << COLOR_GREEN << "Using drive from command line: " << selected_drive << COLOR_RESET << std::endl;

            if (!is_block_device(selected_drive)) {
                std::cerr << COLOR_RED << "Error: " << selected_drive << " is not a valid block device" << COLOR_RESET << std::endl;
                exit(1);
            }
        } else {
            // Interactive drive selection
            display_available_drives();
            selected_drive = get_input("Enter target drive (e.g., /dev/sda): ");
            if (!is_block_device(selected_drive)) {
                std::cerr << COLOR_RED << "Error: " << selected_drive << " is not a valid block device" << COLOR_RESET << std::endl;
                exit(1);
            }
        }
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
        std::cout << "║ " << std::left << std::setw(35) << "Filesystem:" << "btrfs" << std::setw(23) << "" << "║" << std::endl;
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
        std::cout << COLOR_CYAN << "Mounting system for username change..." << COLOR_RESET << std::endl;

        mount_all_btrfs_subvolumes(drive);

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

        mount_all_btrfs_subvolumes(drive);

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
        std::string fs_type = "btrfs";

        std::cout << COLOR_CYAN << "Starting Arch TTY Grub installation..." << COLOR_RESET << std::endl;

        prepare_target_partitions(drive);

        std::string efi_part = drive + "1";
        std::string root_part = drive + "2";

        setup_btrfs_subvolumes(root_part);

        execute_command("cp -r /opt/claudemods-distribution-installer/vconsole.conf /mnt/etc");

        execute_command("cp -r /etc/resolv.conf /mnt/etc");

        execute_command("pacstrap /mnt base " + selected_kernel + " linux-firmware grub efibootmgr os-prober sudo vim nano bash-completion networkmanager");

        // Copy /opt/claudemods-distribution-installer/btrfsfstabcompressed.sh to the new system
        execute_command("cp -r /opt/claudemods-distribution-installer/btrfsfstabcompressed.sh /mnt/opt");
        execute_command("chmod +x /mnt/opt/btrfsfstabcompressed.sh");

        execute_command("mount " + efi_part + " /mnt/boot/efi");

        execute_command("chroot /mnt /bin/bash -c \"systemctl enable NetworkManager\"");

        install_grub_btrfs(drive);

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

            prepare_target_partitions(drive);
            std::string efi_part = drive + "1";
            std::string root_part = drive + "2";

            setup_btrfs_subvolumes(root_part);

            execute_command("cp -r /opt/claudemods-distribution-installer/vconsole.conf /mnt/etc");

            execute_command("cp -r /etc/resolv.conf /mnt/etc");

            execute_command("pacstrap /mnt base gnome gnome-extra gdm grub efibootmgr os-prober arch-install-scripts mkinitcpio " + selected_kernel + " linux-firmware sudo networkmanager");

            // Copy /opt/claudemods-distribution-installer/btrfsfstabcompressed.sh to the new system
            execute_command("cp -r /opt/claudemods-distribution-installer/btrfsfstabcompressed.sh /mnt/opt");
            execute_command("chmod +x /mnt/opt/btrfsfstabcompressed.sh");

            execute_command("mount " + efi_part + " /mnt/boot/efi");

            install_grub_btrfs(drive);

            create_new_user(fs_type, drive);

            execute_command("chroot /mnt /bin/bash -c \"systemctl enable gdm\"");
            execute_command("chroot /mnt /bin/bash -c \"systemctl enable NetworkManager\"");

            std::cout << COLOR_GREEN << "GNOME installation completed!" << COLOR_RESET << std::endl;

            prompt_reboot();
        } else if (desktop_choice == 2) {
            std::cout << COLOR_CYAN << "Installing KDE Plasma..." << COLOR_RESET << std::endl;

            prepare_target_partitions(drive);
            std::string efi_part = drive + "1";
            std::string root_part = drive + "2";

            setup_btrfs_subvolumes(root_part);

            execute_command("cp -r /opt/claudemods-distribution-installer/vconsole.conf /mnt/etc");

            execute_command("cp -r /etc/resolv.conf /mnt/etc");

            execute_command("pacstrap /mnt base plasma sddm dolphin konsole grub efibootmgr os-prober arch-install-scripts mkinitcpio " + selected_kernel + " linux-firmware sudo networkmanager");

            // Copy /opt/claudemods-distribution-installer/btrfsfstabcompressed.sh to the new system
            execute_command("cp -r /opt/claudemods-distribution-installer/btrfsfstabcompressed.sh /mnt/opt");
            execute_command("chmod +x /mnt/opt/btrfsfstabcompressed.sh");

            execute_command("mount " + efi_part + " /mnt/boot/efi");

            install_grub_btrfs(drive);

            create_new_user(fs_type, drive);

            execute_command("chroot /mnt /bin/bash -c \"systemctl enable sddm\"");
            execute_command("chroot /mnt /bin/bash -c \"systemctl enable NetworkManager\"");

            std::cout << COLOR_GREEN << "KDE Plasma installation completed!" << COLOR_RESET << std::endl;

            prompt_reboot();
        } else if (desktop_choice == 3) {
            std::cout << COLOR_CYAN << "Installing XFCE..." << COLOR_RESET << std::endl;

            prepare_target_partitions(drive);
            std::string efi_part = drive + "1";
            std::string root_part = drive + "2";

            setup_btrfs_subvolumes(root_part);

            execute_command("cp -r /opt/claudemods-distribution-installer/vconsole.conf /mnt/etc");

            execute_command("cp -r /etc/resolv.conf /mnt/etc");

            execute_command("pacstrap /mnt base xfce4 xfce4-goodies lightdm lightdm-gtk-greeter grub efibootmgr os-prober arch-install-scripts mkinitcpio " + selected_kernel + " linux-firmware sudo networkmanager");

            // Copy /opt/claudemods-distribution-installer/btrfsfstabcompressed.sh to the new system
            execute_command("cp -r /opt/claudemods-distribution-installer/btrfsfstabcompressed.sh /mnt/opt");
            execute_command("chmod +x /mnt/opt/btrfsfstabcompressed.sh");

            execute_command("mount " + efi_part + " /mnt/boot/efi");

            install_grub_btrfs(drive);

            create_new_user(fs_type, drive);

            execute_command("chroot /mnt /bin/bash -c \"systemctl enable lightdm\"");
            execute_command("chroot /mnt /bin/bash -c \"systemctl enable NetworkManager\"");

            std::cout << COLOR_GREEN << "XFCE installation completed!" << COLOR_RESET << std::endl;

            prompt_reboot();
        } else if (desktop_choice == 4) {
            std::cout << COLOR_CYAN << "Installing LXQt..." << COLOR_RESET << std::endl;

            prepare_target_partitions(drive);
            std::string efi_part = drive + "1";
            std::string root_part = drive + "2";

            setup_btrfs_subvolumes(root_part);

            execute_command("cp -r /opt/claudemods-distribution-installer/vconsole.conf /mnt/etc");

            execute_command("cp -r /etc/resolv.conf /mnt/etc");

            execute_command("pacstrap /mnt base lxqt sddm grub efibootmgr os-prober arch-install-scripts mkinitcpio " + selected_kernel + " linux-firmware sudo networkmanager");

            // Copy /opt/claudemods-distribution-installer/btrfsfstabcompressed.sh to the new system
            execute_command("cp -r /opt/claudemods-distribution-installer/btrfsfstabcompressed.sh /mnt/opt");
            execute_command("chmod +x /mnt/opt/btrfsfstabcompressed.sh");

            execute_command("mount " + efi_part + " /mnt/boot/efi");

            install_grub_btrfs(drive);

            create_new_user(fs_type, drive);

            execute_command("chroot /mnt /bin/bash -c \"systemctl enable sddm\"");
            execute_command("chroot /mnt /bin/bash -c \"systemctl enable NetworkManager\"");

            std::cout << COLOR_GREEN << "LXQt installation completed!" << COLOR_RESET << std::endl;

            prompt_reboot();
        } else if (desktop_choice == 5) {
            std::cout << COLOR_CYAN << "Installing Cinnamon..." << COLOR_RESET << std::endl;

            prepare_target_partitions(drive);
            std::string efi_part = drive + "1";
            std::string root_part = drive + "2";

            setup_btrfs_subvolumes(root_part);

            execute_command("cp -r /opt/claudemods-distribution-installer/vconsole.conf /mnt/etc");

            execute_command("cp -r /etc/resolv.conf /mnt/etc");

            execute_command("pacstrap /mnt base cinnamon lightdm lightdm-gtk-greeter grub efibootmgr os-prober arch-install-scripts mkinitcpio " + selected_kernel + " linux-firmware sudo networkmanager");

            // Copy /opt/claudemods-distribution-installer/btrfsfstabcompressed.sh to the new system
            execute_command("cp -r /opt/claudemods-distribution-installer/btrfsfstabcompressed.sh /mnt/opt");
            execute_command("chmod +x /mnt/opt/btrfsfstabcompressed.sh");

            execute_command("mount " + efi_part + " /mnt/boot/efi");

            install_grub_btrfs(drive);

            create_new_user(fs_type, drive);

            execute_command("chroot /mnt /bin/bash -c \"systemctl enable lightdm\"");
            execute_command("chroot /mnt /bin/bash -c \"systemctl enable NetworkManager\"");

            std::cout << COLOR_GREEN << "Cinnamon installation completed!" << COLOR_RESET << std::endl;

            prompt_reboot();
        } else if (desktop_choice == 6) {
            std::cout << COLOR_CYAN << "Installing MATE..." << COLOR_RESET << std::endl;

            prepare_target_partitions(drive);
            std::string efi_part = drive + "1";
            std::string root_part = drive + "2";

            setup_btrfs_subvolumes(root_part);

            execute_command("cp -r /opt/claudemods-distribution-installer/vconsole.conf /mnt/etc");

            execute_command("cp -r /etc/resolv.conf /mnt/etc");

            execute_command("pacstrap /mnt base mate mate-extra lightdm lightdm-gtk-greeter grub efibootmgr os-prober arch-install-scripts mkinitcpio " + selected_kernel + " linux-firmware sudo networkmanager");

            // Copy /opt/claudemods-distribution-installer/btrfsfstabcompressed.sh to the new system
            execute_command("cp -r /opt/claudemods-distribution-installer/btrfsfstabcompressed.sh /mnt/opt");
            execute_command("chmod +x /mnt/opt/btrfsfstabcompressed.sh");

            execute_command("mount " + efi_part + " /mnt/boot/efi");

            install_grub_btrfs(drive);

            create_new_user(fs_type, drive);

            execute_command("chroot /mnt /bin/bash -c \"systemctl enable lightdm\"");
            execute_command("chroot /mnt /bin/bash -c \"systemctl enable NetworkManager\"");

            std::cout << COLOR_GREEN << "MATE installation completed!" << COLOR_RESET << std::endl;

            prompt_reboot();
        } else if (desktop_choice == 7) {
            std::cout << COLOR_CYAN << "Installing Budgie..." << COLOR_RESET << std::endl;

            prepare_target_partitions(drive);
            std::string efi_part = drive + "1";
            std::string root_part = drive + "2";

            setup_btrfs_subvolumes(root_part);

            execute_command("cp -r /opt/claudemods-distribution-installer/vconsole.conf /mnt/etc");

            execute_command("cp -r /etc/resolv.conf /mnt/etc");

            execute_command("pacstrap /mnt base budgie-desktop lightdm lightdm-gtk-greeter grub efibootmgr os-prober arch-install-scripts mkinitcpio " + selected_kernel + " linux-firmware sudo networkmanager");

            // Copy /opt/claudemods-distribution-installer/btrfsfstabcompressed.sh to the new system
            execute_command("cp -r /opt/claudemods-distribution-installer/btrfsfstabcompressed.sh /mnt/opt");
            execute_command("chmod +x /mnt/opt/btrfsfstabcompressed.sh");

            execute_command("mount " + efi_part + " /mnt/boot/efi");

            install_grub_btrfs(drive);

            create_new_user(fs_type, drive);

            execute_command("chroot /mnt /bin/bash -c \"systemctl enable lightdm\"");
            execute_command("chroot /mnt /bin/bash -c \"systemctl enable NetworkManager\"");

            std::cout << COLOR_GREEN << "Budgie installation completed!" << COLOR_RESET << std::endl;

            prompt_reboot();
        } else if (desktop_choice == 8) {
            std::cout << COLOR_CYAN << "Installing i3 (tiling WM)..." << COLOR_RESET << std::endl;

            prepare_target_partitions(drive);
            std::string efi_part = drive + "1";
            std::string root_part = drive + "2";

            setup_btrfs_subvolumes(root_part);

            execute_command("cp -r /opt/claudemods-distribution-installer/vconsole.conf /mnt/etc");

            execute_command("cp -r /etc/resolv.conf /mnt/etc");

            execute_command("pacstrap /mnt base i3-wm i3status i3lock dmenu lightdm lightdm-gtk-greeter grub efibootmgr os-prober arch-install-scripts mkinitcpio " + selected_kernel + " linux-firmware sudo networkmanager");

            // Copy /opt/claudemods-distribution-installer/btrfsfstabcompressed.sh to the new system
            execute_command("cp -r /opt/claudemods-distribution-installer/btrfsfstabcompressed.sh /mnt/opt");
            execute_command("chmod +x /mnt/opt/btrfsfstabcompressed.sh");

            execute_command("mount " + efi_part + " /mnt/boot/efi");

            install_grub_btrfs(drive);

            create_new_user(fs_type, drive);

            execute_command("chroot /mnt /bin/bash -c \"systemctl enable lightdm\"");
            execute_command("chroot /mnt /bin/bash -c \"systemctl enable NetworkManager\"");

            std::cout << COLOR_GREEN << "i3 installation completed!" << COLOR_RESET << std::endl;

            prompt_reboot();
        } else if (desktop_choice == 9) {
            std::cout << COLOR_CYAN << "Installing Sway (Wayland tiling)..." << COLOR_RESET << std::endl;

            prepare_target_partitions(drive);
            std::string efi_part = drive + "1";
            std::string root_part = drive + "2";

            setup_btrfs_subvolumes(root_part);

            execute_command("cp -r /opt/claudemods-distribution-installer/vconsole.conf /mnt/etc");

            execute_command("cp -r /etc/resolv.conf /mnt/etc");

            execute_command("pacstrap /mnt base sway swaybg waybar wofi lightdm lightdm-gtk-greeter grub efibootmgr os-prober arch-install-scripts mkinitcpio " + selected_kernel + " linux-firmware sudo networkmanager");

            // Copy /opt/claudemods-distribution-installer/btrfsfstabcompressed.sh to the new system
            execute_command("cp -r /opt/claudemods-distribution-installer/btrfsfstabcompressed.sh /mnt/opt");
            execute_command("chmod +x /mnt/opt/btrfsfstabcompressed.sh");

            execute_command("mount " + efi_part + " /mnt/boot/efi");

            install_grub_btrfs(drive);

            create_new_user(fs_type, drive);

            execute_command("chroot /mnt /bin/bash -c \"systemctl enable lightdm\"");
            execute_command("chroot /mnt /bin/bash -c \"systemctl enable NetworkManager\"");

            std::cout << COLOR_GREEN << "Sway installation completed!" << COLOR_RESET << std::endl;

            prompt_reboot();
        } else if (desktop_choice == 10) {
            std::cout << COLOR_PURPLE << "Installing Hyprland (Modern Wayland Compositor)..." << COLOR_RESET << std::endl;

            prepare_target_partitions(drive);
            std::string efi_part = drive + "1";
            std::string root_part = drive + "2";

            setup_btrfs_subvolumes(root_part);

            execute_command("cp -r /opt/claudemods-distribution-installer/vconsole.conf /mnt/etc");

            execute_command("cp -r /etc/resolv.conf /mnt/etc");

            execute_command("pacstrap /mnt base hyprland waybar rofi wl-clipboard sddm grub efibootmgr os-prober arch-install-scripts mkinitcpio " + selected_kernel + " linux-firmware sudo networkmanager");

            // Copy /opt/claudemods-distribution-installer/btrfsfstabcompressed.sh to the new system
            execute_command("cp -r /opt/claudemods-distribution-installer/btrfsfstabcompressed.sh /mnt/opt");
            execute_command("chmod +x /mnt/opt/btrfsfstabcompressed.sh");

            execute_command("mount " + efi_part + " /mnt/boot/efi");

            install_grub_btrfs(drive);

            create_new_user(fs_type, drive);

            execute_command("chroot /mnt /bin/bash -c \"systemctl enable sddm\"");
            execute_command("chroot /mnt /bin/bash -c \"systemctl enable NetworkManager\"");

            std::cout << COLOR_PURPLE << "Hyprland installed! Note: You may need to configure ~/.config/hypr/hyprland.conf" << COLOR_RESET << std::endl;

            prompt_reboot();
        }
    }

    // Function to install CachyOS TTY Grub
    void install_cachyos_tty_grub(const std::string& drive) {
        std::cout << COLOR_CYAN << "Installing CachyOS TTY Grub..." << COLOR_RESET << std::endl;

        prepare_target_partitions(drive);
        std::string efi_part = drive + "1";
        std::string root_part = drive + "2";

        setup_btrfs_subvolumes(root_part);

        execute_cd_command("cd /mnt");
        execute_command("wget --show-progress --no-check-certificate --continue --tries=3 --timeout=30 --waitretry=5 https://claudemodsreloaded.co.uk/claudemods-rootfs-images/Cachyos-TtyGrub/rootfs.img");
        execute_command("unsquashfs -f -d /mnt /mnt/rootfs.img");
        execute_command("mount " + efi_part + " /mnt/boot/efi");

        install_grub_btrfs(drive);

        create_new_user(fs_type, drive);

        std::cout << COLOR_GREEN << "CachyOS TTY Grub installation completed!" << COLOR_RESET << std::endl;

        prompt_reboot();
    }

    // Function to install CachyOS KDE
    void install_cachyos_kde(const std::string& drive) {
        std::cout << COLOR_CYAN << "Installing CachyOS KDE..." << COLOR_RESET << std::endl;

        prepare_target_partitions(drive);
        std::string efi_part = drive + "1";
        std::string root_part = drive + "2";

        setup_btrfs_subvolumes(root_part);

        execute_cd_command("cd /mnt");
        execute_command("wget --show-progress --no-check-certificate --continue --tries=3 --timeout=30 --waitretry=5 https://claudemodsreloaded.co.uk/claudemods-rootfs-images/cachyos-kde/rootfs.img");
        execute_command("unsquashfs -f -d /mnt /mnt/rootfs.img");
        execute_command("mount " + efi_part + " /mnt/boot/efi");

        install_grub_btrfs(drive);

        create_new_user(fs_type, drive);

        std::cout << COLOR_GREEN << "CachyOS KDE Part 1 installation completed!" << COLOR_RESET << std::endl;
        std::cout << COLOR_GREEN << " For CachyOS KDE Part 2 installation Please Reboot And login To Run Next Script!" << COLOR_RESET << std::endl;

        prompt_reboot();
    }

    // Function to install CachyOS GNOME
    void install_cachyos_gnome(const std::string& drive) {
        std::cout << COLOR_CYAN << "Installing CachyOS GNOME..." << COLOR_RESET << std::endl;

        prepare_target_partitions(drive);
        std::string efi_part = drive + "1";
        std::string root_part = drive + "2";

        setup_btrfs_subvolumes(root_part);

        execute_cd_command("cd /mnt");
        execute_command("wget --show-progress --no-check-certificate --continue --tries=3 --timeout=30 --waitretry=5 https://claudemodsreloaded.co.uk/claudemods-rootfs-images/Cachyos-GnomeGrub/rootfs.img");
        execute_command("unsquashfs -f -d /mnt /mnt/rootfs.img");
        execute_command("mount " + efi_part + " /mnt/boot/efi");

        install_grub_btrfs(drive);

        change_username("btrfs", drive);

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

    // NEW CLAUDEMODS INSTALLATION FUNCTIONS - UPDATED FROM .CPP FILE

    // Function to install Spitfire CKGE
    void install_spitfire_ckge(const std::string& drive) {
        std::cout << COLOR_ORANGE << "Installing Spitfire CKGE Minimal..." << COLOR_RESET << std::endl;

        prepare_target_partitions(drive);
        std::string efi_part = drive + "1";
        std::string root_part = drive + "2";

        setup_btrfs_subvolumes(root_part);

        execute_command("cp -r /opt/claudemods-distribution-installer/vconsole.conf /mnt/etc");

        execute_command("cp -r /etc/resolv.conf /mnt/etc");
        execute_command("unzip -o /opt/claudemods-distribution-installer/pacman.d.zip -d /mnt/etc");
        execute_command("unzip -o /opt/claudemods-distribution-installer/pacman.d.zip -d /etc");
        execute_command("cp -r /opt/claudemods-distribution-installer/pacman.conf /mnt/etc");
        execute_command("cp -r /opt/claudemods-distribution-installer/pacman.conf /etc");

        execute_command("pacman -Sy");
        execute_command("pacstrap /mnt claudemods-desktop");
        execute_command("mkdir -p /mnt/boot");
        execute_command("mkdir -p /mnt/boot/grub");
        execute_command("mount " + efi_part + " /mnt/boot/efi");
        execute_command("touch /mnt/boot/grub/grub.cfg.new");

        execute_command("chroot /mnt /bin/bash -c \"systemctl enable sddm\"");
        execute_command("chroot /mnt /bin/bash -c \"systemctl enable NetworkManager\"");

        install_grub_btrfs(drive);

        std::cout << COLOR_CYAN << "setting up Custom grub and plymouth" << COLOR_RESET << std::endl;
        execute_command("cp -r /opt/claudemods-distribution-installer/spitfire-ckge-minimal/grub /mnt/etc/default");
        execute_command("cp -r /opt/claudemods-distribution-installer/spitfire-ckge-minimal/grub.cfg /mnt/boot/grub");
        execute_command("cp -r /opt/claudemods-distribution-installer/spitfire-ckge-minimal/cachyos /mnt/usr/share/grub/themes");
        execute_command("chroot /mnt /bin/bash -c \"grub-mkconfig -o /boot/grub/grub.cfg\"");
        execute_command("cp -r /opt/claudemods-distribution-installer/spitfire-ckge-minimal/cachyos-bootanimation /mnt/usr/share/plymouth/themes/");
        execute_command("cp -r /opt/claudemods-distribution-installer/spitfire-ckge-minimal/term.sh /mnt/usr/local/bin");
        execute_command("chroot /mnt /bin/bash -c \"chmod +x /usr/local/bin/term.sh\"");
        execute_command("cp -r /opt/claudemods-distribution-installer/spitfire-ckge-minimal/term.service /mnt/etc/systemd/system/");
        execute_command("chroot /mnt /bin/bash -c \"systemctl enable term.service >/dev/null 2>&1\"");
        execute_command("chroot /mnt /bin/bash -c \"plymouth-set-default-theme -R cachyos-bootanimation\"");

        std::cout << COLOR_CYAN << "setting up newuser" << COLOR_RESET << std::endl;
        create_new_user(fs_type, drive);

        execute_command("chmod +x /mnt/home/" + new_username + "/.config/fish/config.fish");
        execute_command("chroot /mnt /bin/bash -c \"chmod +x /usr/share/fish/config.fish\"");

        std::cout << COLOR_CYAN << "setting up Spitfire theme and tweaks" << COLOR_RESET << std::endl;
        execute_cd_command("cd /mnt");
        execute_command("wget --show-progress --no-check-certificate --continue --tries=10 --timeout=30 --waitretry=5 https://claudemodsreloaded.co.uk/claudemods-desktop/spitfire-minimal.zip");
        execute_command("wget --show-progress --no-check-certificate --continue --tries=10 --timeout=30 --waitretry=5 https://claudemodsreloaded.co.uk/arch-systemtool/Arch-Systemtool.zip");
        execute_command("unzip -o /mnt/Arch-Systemtool.zip -d /mnt/opt");
        execute_command("unzip -o /mnt/spitfire-minimal.zip -d /mnt/home/" + new_username + "/");
        execute_command("mkdir -p /mnt/etc/sddm.conf.d");
        execute_command("cp -r /opt/claudemods-distribution-installer/spitfire-ckge-minimal/kde_settings.conf /mnt/etc/sddm.conf.d");
        execute_command("cp -r /opt/claudemods-distribution-installer/spitfire-ckge-minimal/tweaksspitfire.sh /mnt/opt");
        execute_command("chmod +x /mnt/opt/tweaksspitfire.sh");
        execute_command("chroot /mnt /bin/bash -c \"su - " + new_username + " -c 'cd /opt && ./tweaksspitfire.sh " + new_username + "'\"");
        execute_command("cp -r /opt/claudemods-distribution-installer/spitfire-ckge-minimal/konsolerc /mnt/home/" + new_username + "/.config/");
        execute_command("cp -r /opt/claudemods-distribution-installer/spitfire-ckge-minimal/SpitFireLogin /mnt/usr/share/sddm/themes");
        execute_command("cp -r /opt/claudemods-distribution-installer/spitfire-ckge-minimal/claudemods-cyan.colorscheme /mnt/home/" + new_username + "/.local/share/konsole");
        execute_command("cp -r /opt/claudemods-distribution-installer/spitfire-ckge-minimal/claudemods-cyan.profile /mnt/home/" + new_username + "/.local/share/konsole");
        execute_command("rm -rf /mnt/Arch-Systemtool.zip");
        execute_command("rm -rf /mnt/spitfire-minimal.zip");
        execute_command("rm -rf /mnt/opt/tweaksspitfire.sh");

        // Fix user-places.xbel with actual home folder name
        std::cout << COLOR_CYAN << "Fixing user-places.xbel with actual home folder name..." << COLOR_RESET << std::endl;
        std::string cmd = "ls -1 /mnt/home | grep -v '^\\.' | head -1";
        FILE* pipe = popen(cmd.c_str(), "r");
        if (pipe) {
            char buffer[128];
            std::string home_folder;
            if (fgets(buffer, sizeof(buffer), pipe) != nullptr) {
                home_folder = buffer;
                home_folder.erase(std::remove(home_folder.begin(), home_folder.end(), '\n'), home_folder.end());
            }
            pclose(pipe);

            if (!home_folder.empty()) {
                std::string user_places_file = "/mnt/home/" + home_folder + "/.local/share/user-places.xbel";
                std::string sed_cmd = "sed -i 's/spitfire/" + home_folder + "/g' " + user_places_file;
                execute_command(sed_cmd);
                std::cout << COLOR_GREEN << "Updated user-places.xbel: replaced 'spitfire' with '" << home_folder << "'" << COLOR_RESET << std::endl;
            }
        }

        std::cout << COLOR_ORANGE << "Spitfire CKGE Minimal installation completed!" << COLOR_RESET << std::endl;
        prompt_reboot();
    }

    // Function to install Apex CKGE
    void install_apex_ckge(const std::string& drive) {
        std::cout << COLOR_PURPLE << "Installing Apex CKGE Minimal..." << COLOR_RESET << std::endl;

        prepare_target_partitions(drive);
        std::string efi_part = drive + "1";
        std::string root_part = drive + "2";

        setup_btrfs_subvolumes(root_part);

        execute_command("cp -r /opt/claudemods-distribution-installer/vconsole.conf /mnt/etc");

        execute_command("cp -r /etc/resolv.conf /mnt/etc");
        execute_command("unzip -o /opt/claudemods-distribution-installer/pacman.d.zip -d /mnt/etc");
        execute_command("unzip -o /opt/claudemods-distribution-installer/pacman.d.zip -d /etc");
        execute_command("cp -r /opt/claudemods-distribution-installer/pacman.conf /mnt/etc");
        execute_command("cp -r /opt/claudemods-distribution-installer/pacman.conf /etc");

        execute_command("pacman -Sy");
        execute_command("pacstrap /mnt claudemods-desktop");
        execute_command("mkdir -p /mnt/boot");
        execute_command("mkdir -p /mnt/boot/grub");
        execute_command("mount " + efi_part + " /mnt/boot/efi");
        execute_command("touch /mnt/boot/grub/grub.cfg.new");

        execute_command("chroot /mnt /bin/bash -c \"systemctl enable sddm\"");
        execute_command("chroot /mnt /bin/bash -c \"systemctl enable NetworkManager\"");

        install_grub_btrfs(drive);

        std::cout << COLOR_CYAN << "setting up Custom grub and plymouth" << COLOR_RESET << std::endl;
        execute_command("cp -r /opt/claudemods-distribution-installer/apex-ckge-minimal/grub /mnt/etc/default");
        execute_command("cp -r /opt/claudemods-distribution-installer/apex-ckge-minimal/grub.cfg /mnt/boot/grub");
        execute_command("cp -r /opt/claudemods-distribution-installer/apex-ckge-minimal/cachyos /mnt/usr/share/grub/themes");
        execute_command("chroot /mnt /bin/bash -c \"grub-mkconfig -o /boot/grub/grub.cfg\"");
        execute_command("cp -r /opt/claudemods-distribution-installer/spitfire-ckge-minimal/cachyos-bootanimation /mnt/usr/share/plymouth/themes/");
        execute_command("cp -r /opt/claudemods-distribution-installer/apex-ckge-minimal/term.sh /mnt/usr/local/bin");
        execute_command("chroot /mnt /bin/bash -c \"chmod +x /usr/local/bin/term.sh\"");
        execute_command("cp -r /opt/claudemods-distribution-installer/apex-ckge-minimal/term.service /mnt/etc/systemd/system/");
        execute_command("chroot /mnt /bin/bash -c \"systemctl enable term.service >/dev/null 2>&1\"");
        execute_command("chroot /mnt /bin/bash -c \"plymouth-set-default-theme -R cachyos-bootanimation\"");

        std::cout << COLOR_CYAN << "setting up newuser" << COLOR_RESET << std::endl;
        create_new_user(fs_type, drive);

        execute_command("chmod +x /mnt/home/" + new_username + "/.config/fish/config.fish");
        execute_command("chroot /mnt /bin/bash -c \"chmod +x /usr/share/fish/config.fish\"");

        std::cout << COLOR_CYAN << "setting up Apex theme and tweaks" << COLOR_RESET << std::endl;
        execute_cd_command("cd /mnt");
        execute_command("wget --show-progress --no-check-certificate --continue --tries=10 --timeout=30 --waitretry=5 https://claudemodsreloaded.co.uk/claudemods-desktop/apex-minimal.zip");
        execute_command("wget --show-progress --no-check-certificate --continue --tries=10 --timeout=30 --waitretry=5 https://claudemodsreloaded.co.uk/arch-systemtool/Arch-Systemtool.zip");
        execute_command("unzip -o /mnt/Arch-Systemtool.zip -d /mnt/opt");
        execute_command("unzip -o /mnt/apex-minimal.zip -d /mnt/home/" + new_username + "/");
        execute_command("mkdir -p /mnt/etc/sddm.conf.d");
        execute_command("cp -r /opt/claudemods-distribution-installer/apex-ckge-minimal/kde_settings.conf /mnt/etc/sddm.conf.d");
        execute_command("cp -r /opt/claudemods-distribution-installer/apex-ckge-minimal/tweaksapex.sh /mnt/opt");
        execute_command("chmod +x /mnt/opt/tweaksapex.sh");
        execute_command("chroot /mnt /bin/bash -c \"su - " + new_username + " -c 'cd /opt && ./tweaksapex.sh " + new_username + "'\"");
        execute_command("cp -r /opt/claudemods-distribution-installer/apex-ckge-minimal/konsolerc /mnt/home/" + new_username + "/.config/");
        execute_command("cp -r /opt/claudemods-distribution-installer/apex-ckge-minimal/ApexLogin2 /mnt/usr/share/sddm/themes");
        execute_command("cp -r /opt/claudemods-distribution-installer/apex-ckge-minimal/claudemods-cyan.colorscheme /mnt/home/" + new_username + "/.local/share/konsole");
        execute_command("cp -r /opt/claudemods-distribution-installer/apex-ckge-minimal/claudemods-cyan.profile /mnt/home/" + new_username + "/.local/share/konsole");
        execute_command("rm -rf /mnt/Arch-Systemtool.zip");
        execute_command("rm -rf /mnt/apex-minimal.zip");
        execute_command("rm -rf /mnt/opt/tweaksapex.sh");

        // Fix user-places.xbel with actual home folder name
        std::cout << COLOR_CYAN << "Fixing user-places.xbel with actual home folder name..." << COLOR_RESET << std::endl;
        std::string cmd = "ls -1 /mnt/home | grep -v '^\\.' | head -1";
        FILE* pipe = popen(cmd.c_str(), "r");
        if (pipe) {
            char buffer[128];
            std::string home_folder;
            if (fgets(buffer, sizeof(buffer), pipe) != nullptr) {
                home_folder = buffer;
                home_folder.erase(std::remove(home_folder.begin(), home_folder.end(), '\n'), home_folder.end());
            }
            pclose(pipe);

            if (!home_folder.empty()) {
                std::string user_places_file = "/mnt/home/" + home_folder + "/.local/share/user-places.xbel";
                std::string sed_cmd = "sed -i 's/apex/" + home_folder + "/g' " + user_places_file;
                execute_command(sed_cmd);
                std::cout << COLOR_GREEN << "Updated user-places.xbel: replaced 'apex' with '" << home_folder << "'" << COLOR_RESET << std::endl;
            }
        }

        std::cout << COLOR_PURPLE << "Apex CKGE Minimal installation completed!" << COLOR_RESET << std::endl;
        prompt_reboot();
    }

    // Function to install Spitfire CKGE Full
    void install_spitfire_ckge_full(const std::string& drive) {
        std::cout << COLOR_ORANGE << "Installing Spitfire CKGE Full..." << COLOR_RESET << std::endl;

        prepare_target_partitions(drive);
        std::string efi_part = drive + "1";
        std::string root_part = drive + "2";

        setup_btrfs_subvolumes(root_part);

        execute_command("cp -r /opt/claudemods-distribution-installer/vconsole.conf /mnt/etc");

        execute_command("cp -r /etc/resolv.conf /mnt/etc");
        execute_command("unzip -o /opt/claudemods-distribution-installer/pacman.d.zip -d /mnt/etc");
        execute_command("unzip -o /opt/claudemods-distribution-installer/pacman.d.zip -d /etc");
        execute_command("cp -r /opt/claudemods-distribution-installer/pacman.conf /mnt/etc");
        execute_command("cp -r /opt/claudemods-distribution-installer/pacman.conf /etc");

        execute_command("pacman -Sy");
        execute_command("pacstrap /mnt claudemods-desktop-full");
        execute_command("mkdir -p /mnt/boot");
        execute_command("mkdir -p /mnt/boot/grub");
        execute_command("mount " + efi_part + " /mnt/boot/efi");
        execute_command("touch /mnt/boot/grub/grub.cfg.new");

        execute_command("chroot /mnt /bin/bash -c \"systemctl enable sddm\"");
        execute_command("chroot /mnt /bin/bash -c \"systemctl enable NetworkManager\"");

        install_grub_btrfs(drive);

        std::cout << COLOR_CYAN << "setting up Custom grub and plymouth" << COLOR_RESET << std::endl;
        execute_command("cp -r /opt/claudemods-distribution-installer/spitfire-ckge-minimal/grub /mnt/etc/default");
        execute_command("cp -r /opt/claudemods-distribution-installer/spitfire-ckge-minimal/grub.cfg /mnt/boot/grub");
        execute_command("cp -r /opt/claudemods-distribution-installer/spitfire-ckge-minimal/cachyos /mnt/usr/share/grub/themes");
        execute_command("chroot /mnt /bin/bash -c \"grub-mkconfig -o /boot/grub/grub.cfg\"");
        execute_command("cp -r /opt/claudemods-distribution-installer/spitfire-ckge-minimal/cachyos-bootanimation /mnt/usr/share/plymouth/themes/");
        execute_command("cp -r /opt/claudemods-distribution-installer/spitfire-ckge-minimal/termfull.sh /mnt/usr/local/bin");
        execute_command("chroot /mnt /bin/bash -c \"chmod +x /usr/local/bin/termfull.sh\"");
        execute_command("cp -r /opt/claudemods-distribution-installer/spitfire-ckge-minimal/termfull.service /mnt/etc/systemd/system/");
        execute_command("chroot /mnt /bin/bash -c \"systemctl enable termfull.service >/dev/null 2>&1\"");
        execute_command("chroot /mnt /bin/bash -c \"plymouth-set-default-theme -R cachyos-bootanimation\"");

        std::cout << COLOR_CYAN << "setting up newuser" << COLOR_RESET << std::endl;
        create_new_user(fs_type, drive);

        execute_command("chmod +x /mnt/home/" + new_username + "/.config/fish/config.fish");
        execute_command("chroot /mnt /bin/bash -c \"chmod +x /usr/share/fish/config.fish\"");

        std::cout << COLOR_CYAN << "setting up Spitfire theme and tweaks" << COLOR_RESET << std::endl;
        execute_cd_command("cd /mnt");
        execute_command("wget --show-progress --no-check-certificate --continue --tries=10 --timeout=30 --waitretry=5 https://claudemodsreloaded.co.uk/claudemods-desktop/spitfire-full.zip");
        execute_command("wget --show-progress --no-check-certificate --continue --tries=10 --timeout=30 --waitretry=5 https://claudemodsreloaded.co.uk/arch-systemtool/Arch-Systemtool.zip");
        execute_command("unzip -o /mnt/Arch-Systemtool.zip -d /mnt/opt");
        execute_command("unzip -o /mnt/spitfire-full.zip -d /mnt/home/" + new_username + "/");
        execute_command("mkdir -p /mnt/etc/sddm.conf.d");
        execute_command("cp -r /opt/claudemods-distribution-installer/spitfire-ckge-minimal/kde_settings.conf /mnt/etc/sddm.conf.d");
        execute_command("cp -r /opt/claudemods-distribution-installer/spitfire-ckge-minimal/tweaksspitfire.sh /mnt/opt");
        execute_command("chmod +x /mnt/opt/tweaksspitfire.sh");
        execute_command("chroot /mnt /bin/bash -c \"su - " + new_username + " -c 'cd /opt && ./tweaksspitfire.sh " + new_username + "'\"");
        execute_command("cp -r /opt/claudemods-distribution-installer/spitfire-ckge-minimal/konsolerc /mnt/home/" + new_username + "/.config/");
        execute_command("cp -r /opt/claudemods-distribution-installer/spitfire-ckge-minimal/SpitFireLogin /mnt/usr/share/sddm/themes");
        execute_command("cp -r /opt/claudemods-distribution-installer/spitfire-ckge-minimal/claudemods-cyan.colorscheme /mnt/home/" + new_username + "/.local/share/konsole");
        execute_command("cp -r /opt/claudemods-distribution-installer/spitfire-ckge-minimal/claudemods-cyan.profile /mnt/home/" + new_username + "/.local/share/konsole");
        execute_command("rm -rf /mnt/Arch-Systemtool.zip");
        execute_command("rm -rf /mnt/spitfire-full.zip");
        execute_command("rm -rf /mnt/opt/tweaksspitfire.sh");

        // Fix user-places.xbel with actual home folder name
        std::cout << COLOR_CYAN << "Fixing user-places.xbel with actual home folder name..." << COLOR_RESET << std::endl;
        std::string cmd = "ls -1 /mnt/home | grep -v '^\\.' | head -1";
        FILE* pipe = popen(cmd.c_str(), "r");
        if (pipe) {
            char buffer[128];
            std::string home_folder;
            if (fgets(buffer, sizeof(buffer), pipe) != nullptr) {
                home_folder = buffer;
                home_folder.erase(std::remove(home_folder.begin(), home_folder.end(), '\n'), home_folder.end());
            }
            pclose(pipe);

            if (!home_folder.empty()) {
                std::string user_places_file = "/mnt/home/" + home_folder + "/.local/share/user-places.xbel";
                std::string sed_cmd = "sed -i 's/spitfire/" + home_folder + "/g' " + user_places_file;
                execute_command(sed_cmd);
                std::cout << COLOR_GREEN << "Updated user-places.xbel: replaced 'spitfire' with '" << home_folder << "'" << COLOR_RESET << std::endl;
            }
        }

        std::cout << COLOR_ORANGE << "Spitfire CKGE Full installation completed!" << COLOR_RESET << std::endl;
        prompt_reboot();
    }

    // Function to install Apex CKGE Full
    void install_apex_ckge_full(const std::string& drive) {
        std::cout << COLOR_PURPLE << "Installing Apex CKGE Full..." << COLOR_RESET << std::endl;

        prepare_target_partitions(drive);
        std::string efi_part = drive + "1";
        std::string root_part = drive + "2";

        setup_btrfs_subvolumes(root_part);

        execute_command("cp -r /opt/claudemods-distribution-installer/vconsole.conf /mnt/etc");

        execute_command("cp -r /etc/resolv.conf /mnt/etc");
        execute_command("unzip -o /opt/claudemods-distribution-installer/pacman.d.zip -d /mnt/etc");
        execute_command("unzip -o /opt/claudemods-distribution-installer/pacman.d.zip -d /etc");
        execute_command("cp -r /opt/claudemods-distribution-installer/pacman.conf /mnt/etc");
        execute_command("cp -r /opt/claudemods-distribution-installer/pacman.conf /etc");

        execute_command("pacman -Sy");
        execute_command("pacstrap /mnt claudemods-desktop-full");
        execute_command("mkdir -p /mnt/boot");
        execute_command("mkdir -p /mnt/boot/grub");
        execute_command("mount " + efi_part + " /mnt/boot/efi");
        execute_command("touch /mnt/boot/grub/grub.cfg.new");

        execute_command("chroot /mnt /bin/bash -c \"systemctl enable sddm\"");
        execute_command("chroot /mnt /bin/bash -c \"systemctl enable NetworkManager\"");

        install_grub_btrfs(drive);

        std::cout << COLOR_CYAN << "setting up Custom grub and plymouth" << COLOR_RESET << std::endl;
        execute_command("cp -r /opt/claudemods-distribution-installer/apex-ckge-minimal/grub /mnt/etc/default");
        execute_command("cp -r /opt/claudemods-distribution-installer/apex-ckge-minimal/grub.cfg /mnt/boot/grub");
        execute_command("cp -r /opt/claudemods-distribution-installer/apex-ckge-minimal/cachyos /mnt/usr/share/grub/themes");
        execute_command("chroot /mnt /bin/bash -c \"grub-mkconfig -o /boot/grub/grub.cfg\"");
        execute_command("cp -r /opt/claudemods-distribution-installer/spitfire-ckge-minimal/cachyos-bootanimation /mnt/usr/share/plymouth/themes/");
        execute_command("cp -r /opt/claudemods-distribution-installer/apex-ckge-minimal/termfull.sh /mnt/usr/local/bin");
        execute_command("chroot /mnt /bin/bash -c \"chmod +x /usr/local/bin/termfull.sh\"");
        execute_command("cp -r /opt/claudemods-distribution-installer/apex-ckge-minimal/termfull.service /mnt/etc/systemd/system/");
        execute_command("chroot /mnt /bin/bash -c \"systemctl enable termfull.service >/dev/null 2>&1\"");
        execute_command("chroot /mnt /bin/bash -c \"plymouth-set-default-theme -R cachyos-bootanimation\"");

        std::cout << COLOR_CYAN << "setting up newuser" << COLOR_RESET << std::endl;
        create_new_user(fs_type, drive);

        execute_command("chmod +x /mnt/home/" + new_username + "/.config/fish/config.fish");
        execute_command("chroot /mnt /bin/bash -c \"chmod +x /usr/share/fish/config.fish\"");

        std::cout << COLOR_CYAN << "setting up Apex theme and tweaks" << COLOR_RESET << std::endl;
        execute_cd_command("cd /mnt");
        execute_command("wget --show-progress --no-check-certificate --continue --tries=10 --timeout=30 --waitretry=5 https://claudemodsreloaded.co.uk/claudemods-desktop/apex-full.zip");
        execute_command("wget --show-progress --no-check-certificate --continue --tries=10 --timeout=30 --waitretry=5 https://claudemodsreloaded.co.uk/arch-systemtool/Arch-Systemtool.zip");
        execute_command("unzip -o /mnt/Arch-Systemtool.zip -d /mnt/opt");
        execute_command("unzip -o /mnt/apex-full.zip -d /mnt/home/" + new_username + "/");
        execute_command("mkdir -p /mnt/etc/sddm.conf.d");
        execute_command("cp -r /opt/claudemods-distribution-installer/apex-ckge-minimal/kde_settings.conf /mnt/etc/sddm.conf.d");
        execute_command("cp -r /opt/claudemods-distribution-installer/apex-ckge-minimal/tweaksapex.sh /mnt/opt");
        execute_command("chmod +x /mnt/opt/tweaksapex.sh");
        execute_command("chroot /mnt /bin/bash -c \"su - " + new_username + " -c 'cd /opt && ./tweaksapex.sh " + new_username + "'\"");
        execute_command("cp -r /opt/claudemods-distribution-installer/apex-ckge-minimal/konsolerc /mnt/home/" + new_username + "/.config/");
        execute_command("cp -r /opt/claudemods-distribution-installer/apex-ckge-minimal/ApexLogin2 /mnt/usr/share/sddm/themes");
        execute_command("cp -r /opt/claudemods-distribution-installer/apex-ckge-minimal/claudemods-cyan.colorscheme /mnt/home/" + new_username + "/.local/share/konsole");
        execute_command("cp -r /opt/claudemods-distribution-installer/apex-ckge-minimal/claudemods-cyan.profile /mnt/home/" + new_username + "/.local/share/konsole");
        execute_command("rm -rf /mnt/Arch-Systemtool.zip");
        execute_command("rm -rf /mnt/apex-full.zip");
        execute_command("rm -rf /mnt/opt/tweaksapex.sh");

        // Fix user-places.xbel with actual home folder name
        std::cout << COLOR_CYAN << "Fixing user-places.xbel with actual home folder name..." << COLOR_RESET << std::endl;
        std::string cmd = "ls -1 /mnt/home | grep -v '^\\.' | head -1";
        FILE* pipe = popen(cmd.c_str(), "r");
        if (pipe) {
            char buffer[128];
            std::string home_folder;
            if (fgets(buffer, sizeof(buffer), pipe) != nullptr) {
                home_folder = buffer;
                home_folder.erase(std::remove(home_folder.begin(), home_folder.end(), '\n'), home_folder.end());
            }
            pclose(pipe);

            if (!home_folder.empty()) {
                std::string user_places_file = "/mnt/home/" + home_folder + "/.local/share/user-places.xbel";
                std::string sed_cmd = "sed -i 's/apex/" + home_folder + "/g' " + user_places_file;
                execute_command(sed_cmd);
                std::cout << COLOR_GREEN << "Updated user-places.xbel: replaced 'apex' with '" << home_folder << "'" << COLOR_RESET << std::endl;
            }
        }

        std::cout << COLOR_PURPLE << "Apex CKGE Full installation completed!" << COLOR_RESET << std::endl;
        prompt_reboot();
    }

    // Function to install Spitfire CKGE Minimal Dev
    void install_spitfire_ckge_minimal_dev(const std::string& drive) {
        std::cout << COLOR_ORANGE << "Installing Spitfire CKGE Minimal Dev..." << COLOR_RESET << std::endl;

        prepare_target_partitions(drive);
        std::string efi_part = drive + "1";
        std::string root_part = drive + "2";

        setup_btrfs_subvolumes(root_part);

        execute_command("cp -r /opt/claudemods-distribution-installer/vconsole.conf /mnt/etc");

        execute_command("cp -r /etc/resolv.conf /mnt/etc");
        execute_command("unzip -o /opt/claudemods-distribution-installer/pacman.d.zip -d /mnt/etc");
        execute_command("unzip -o /opt/claudemods-distribution-installer/pacman.d.zip -d /etc");
        execute_command("cp -r /opt/claudemods-distribution-installer/pacman.conf /mnt/etc");
        execute_command("cp -r /opt/claudemods-distribution-installer/pacman.conf /etc");

        execute_command("pacman -Sy");
        execute_command("pacstrap /mnt claudemods-desktop-dev");
        execute_command("mkdir -p /mnt/boot");
        execute_command("mkdir -p /mnt/boot/grub");
        execute_command("mount " + efi_part + " /mnt/boot/efi");
        execute_command("touch /mnt/boot/grub/grub.cfg.new");

        execute_command("chroot /mnt /bin/bash -c \"systemctl enable sddm\"");
        execute_command("chroot /mnt /bin/bash -c \"systemctl enable NetworkManager\"");

        install_grub_btrfs(drive);

        std::cout << COLOR_CYAN << "setting up Custom grub and plymouth" << COLOR_RESET << std::endl;
        execute_command("cp -r /opt/claudemods-distribution-installer/spitfire-ckge-minimal/grub /mnt/etc/default");
        execute_command("cp -r /opt/claudemods-distribution-installer/spitfire-ckge-minimal/grub.cfg /mnt/boot/grub");
        execute_command("cp -r /opt/claudemods-distribution-installer/spitfire-ckge-minimal/cachyos /mnt/usr/share/grub/themes");
        execute_command("chroot /mnt /bin/bash -c \"grub-mkconfig -o /boot/grub/grub.cfg\"");
        execute_command("cp -r /opt/claudemods-distribution-installer/spitfire-ckge-minimal/cachyos-bootanimation /mnt/usr/share/plymouth/themes/");
        execute_command("cp -r /opt/claudemods-distribution-installer/spitfire-ckge-minimal/term.sh /mnt/usr/local/bin");
        execute_command("chroot /mnt /bin/bash -c \"chmod +x /usr/local/bin/term.sh\"");
        execute_command("cp -r /opt/claudemods-distribution-installer/spitfire-ckge-minimal/term.service /mnt/etc/systemd/system/");
        execute_command("chroot /mnt /bin/bash -c \"systemctl enable term.service >/dev/null 2>&1\"");
        execute_command("chroot /mnt /bin/bash -c \"plymouth-set-default-theme -R cachyos-bootanimation\"");

        std::cout << COLOR_CYAN << "setting up newuser" << COLOR_RESET << std::endl;
        create_new_user(fs_type, drive);

        execute_command("chmod +x /mnt/home/" + new_username + "/.config/fish/config.fish");
        execute_command("chroot /mnt /bin/bash -c \"chmod +x /usr/share/fish/config.fish\"");

        std::cout << COLOR_CYAN << "setting up Spitfire theme and tweaks" << COLOR_RESET << std::endl;
        execute_cd_command("cd /mnt");
        execute_command("wget --show-progress --no-check-certificate --continue --tries=10 --timeout=30 --waitretry=5 https://claudemodsreloaded.co.uk/claudemods-desktop/spitfire-minimal.zip");
        execute_command("wget --show-progress --no-check-certificate --continue --tries=10 --timeout=30 --waitretry=5 https://claudemodsreloaded.co.uk/arch-systemtool/Arch-Systemtool.zip");
        execute_command("unzip -o /mnt/Arch-Systemtool.zip -d /mnt/opt");
        execute_command("unzip -o /mnt/spitfire-minimal.zip -d /mnt/home/" + new_username + "/");
        execute_command("mkdir -p /mnt/etc/sddm.conf.d");
        execute_command("cp -r /opt/claudemods-distribution-installer/spitfire-ckge-minimal/kde_settings.conf /mnt/etc/sddm.conf.d");
        execute_command("cp -r /opt/claudemods-distribution-installer/spitfire-ckge-minimal/tweaksspitfire.sh /mnt/opt");
        execute_command("chmod +x /mnt/opt/tweaksspitfire.sh");
        execute_command("chroot /mnt /bin/bash -c \"su - " + new_username + " -c 'cd /opt && ./tweaksspitfire.sh " + new_username + "'\"");
        execute_command("cp -r /opt/claudemods-distribution-installer/spitfire-ckge-minimal/konsolerc /mnt/home/" + new_username + "/.config/");
        execute_command("cp -r /opt/claudemods-distribution-installer/spitfire-ckge-minimal/SpitFireLogin /mnt/usr/share/sddm/themes");
        execute_command("cp -r /opt/claudemods-distribution-installer/spitfire-ckge-minimal/claudemods-cyan.colorscheme /mnt/home/" + new_username + "/.local/share/konsole");
        execute_command("cp -r /opt/claudemods-distribution-installer/spitfire-ckge-minimal/claudemods-cyan.profile /mnt/home/" + new_username + "/.local/share/konsole");
        execute_command("rm -rf /mnt/Arch-Systemtool.zip");
        execute_command("rm -rf /mnt/spitfire-minimal.zip");
        execute_command("rm -rf /mnt/opt/tweaksspitfire.sh");

        // Fix user-places.xbel with actual home folder name
        std::cout << COLOR_CYAN << "Fixing user-places.xbel with actual home folder name..." << COLOR_RESET << std::endl;
        std::string cmd = "ls -1 /mnt/home | grep -v '^\\.' | head -1";
        FILE* pipe = popen(cmd.c_str(), "r");
        if (pipe) {
            char buffer[128];
            std::string home_folder;
            if (fgets(buffer, sizeof(buffer), pipe) != nullptr) {
                home_folder = buffer;
                home_folder.erase(std::remove(home_folder.begin(), home_folder.end(), '\n'), home_folder.end());
            }
            pclose(pipe);

            if (!home_folder.empty()) {
                std::string user_places_file = "/mnt/home/" + home_folder + "/.local/share/user-places.xbel";
                std::string sed_cmd = "sed -i 's/spitfire/" + home_folder + "/g' " + user_places_file;
                execute_command(sed_cmd);
                std::cout << COLOR_GREEN << "Updated user-places.xbel: replaced 'spitfire' with '" << home_folder << "'" << COLOR_RESET << std::endl;
            }
        }

        std::cout << COLOR_ORANGE << "Spitfire CKGE Minimal Dev installation completed!" << COLOR_RESET << std::endl;
        prompt_reboot();
    }

    // Function to install Apex CKGE Minimal Dev
    void install_apex_ckge_minimal_dev(const std::string& drive) {
        std::cout << COLOR_PURPLE << "Installing Apex CKGE Minimal Dev..." << COLOR_RESET << std::endl;

        prepare_target_partitions(drive);
        std::string efi_part = drive + "1";
        std::string root_part = drive + "2";

        setup_btrfs_subvolumes(root_part);

        execute_command("cp -r /opt/claudemods-distribution-installer/vconsole.conf /mnt/etc");

        execute_command("cp -r /etc/resolv.conf /mnt/etc");
        execute_command("unzip -o /opt/claudemods-distribution-installer/pacman.d.zip -d /mnt/etc");
        execute_command("unzip -o /opt/claudemods-distribution-installer/pacman.d.zip -d /etc");
        execute_command("cp -r /opt/claudemods-distribution-installer/pacman.conf /mnt/etc");
        execute_command("cp -r /opt/claudemods-distribution-installer/pacman.conf /etc");

        execute_command("pacman -Sy");
        execute_command("pacstrap /mnt claudemods-desktop-dev");
        execute_command("mkdir -p /mnt/boot");
        execute_command("mkdir -p /mnt/boot/grub");
        execute_command("mount " + efi_part + " /mnt/boot/efi");
        execute_command("touch /mnt/boot/grub/grub.cfg.new");

        execute_command("chroot /mnt /bin/bash -c \"systemctl enable sddm\"");
        execute_command("chroot /mnt /bin/bash -c \"systemctl enable NetworkManager\"");

        install_grub_btrfs(drive);

        std::cout << COLOR_CYAN << "setting up Custom grub and plymouth" << COLOR_RESET << std::endl;
        execute_command("cp -r /opt/claudemods-distribution-installer/apex-ckge-minimal/grub /mnt/etc/default");
        execute_command("cp -r /opt/claudemods-distribution-installer/apex-ckge-minimal/grub.cfg /mnt/boot/grub");
        execute_command("cp -r /opt/claudemods-distribution-installer/apex-ckge-minimal/cachyos /mnt/usr/share/grub/themes");
        execute_command("chroot /mnt /bin/bash -c \"grub-mkconfig -o /boot/grub/grub.cfg\"");
        execute_command("cp -r /opt/claudemods-distribution-installer/spitfire-ckge-minimal/cachyos-bootanimation /mnt/usr/share/plymouth/themes/");
        execute_command("cp -r /opt/claudemods-distribution-installer/apex-ckge-minimal/term.sh /mnt/usr/local/bin");
        execute_command("chroot /mnt /bin/bash -c \"chmod +x /usr/local/bin/term.sh\"");
        execute_command("cp -r /opt/claudemods-distribution-installer/apex-ckge-minimal/term.service /mnt/etc/systemd/system/");
        execute_command("chroot /mnt /bin/bash -c \"systemctl enable term.service >/dev/null 2>&1\"");
        execute_command("chroot /mnt /bin/bash -c \"plymouth-set-default-theme -R cachyos-bootanimation\"");

        std::cout << COLOR_CYAN << "setting up newuser" << COLOR_RESET << std::endl;
        create_new_user(fs_type, drive);

        execute_command("chmod +x /mnt/home/" + new_username + "/.config/fish/config.fish");
        execute_command("chroot /mnt /bin/bash -c \"chmod +x /usr/share/fish/config.fish\"");

        std::cout << COLOR_CYAN << "setting up Apex theme and tweaks" << COLOR_RESET << std::endl;
        execute_cd_command("cd /mnt");
        execute_command("wget --show-progress --no-check-certificate --continue --tries=10 --timeout=30 --waitretry=5 https://claudemodsreloaded.co.uk/claudemods-desktop/apex-minimal.zip");
        execute_command("wget --show-progress --no-check-certificate --continue --tries=10 --timeout=30 --waitretry=5 https://claudemodsreloaded.co.uk/arch-systemtool/Arch-Systemtool.zip");
        execute_command("unzip -o /mnt/Arch-Systemtool.zip -d /mnt/opt");
        execute_command("unzip -o /mnt/apex-minimal.zip -d /mnt/home/" + new_username + "/");
        execute_command("mkdir -p /mnt/etc/sddm.conf.d");
        execute_command("cp -r /opt/claudemods-distribution-installer/apex-ckge-minimal/kde_settings.conf /mnt/etc/sddm.conf.d");
        execute_command("cp -r /opt/claudemods-distribution-installer/apex-ckge-minimal/tweaksapex.sh /mnt/opt");
        execute_command("chmod +x /mnt/opt/tweaksapex.sh");
        execute_command("chroot /mnt /bin/bash -c \"su - " + new_username + " -c 'cd /opt && ./tweaksapex.sh " + new_username + "'\"");
        execute_command("cp -r /opt/claudemods-distribution-installer/apex-ckge-minimal/konsolerc /mnt/home/" + new_username + "/.config/");
        execute_command("cp -r /opt/claudemods-distribution-installer/apex-ckge-minimal/ApexLogin2 /mnt/usr/share/sddm/themes");
        execute_command("cp -r /opt/claudemods-distribution-installer/apex-ckge-minimal/claudemods-cyan.colorscheme /mnt/home/" + new_username + "/.local/share/konsole");
        execute_command("cp -r /opt/claudemods-distribution-installer/apex-ckge-minimal/claudemods-cyan.profile /mnt/home/" + new_username + "/.local/share/konsole");
        execute_command("rm -rf /mnt/Arch-Systemtool.zip");
        execute_command("rm -rf /mnt/apex-minimal.zip");
        execute_command("rm -rf /mnt/opt/tweaksapex.sh");

        // Fix user-places.xbel with actual home folder name
        std::cout << COLOR_CYAN << "Fixing user-places.xbel with actual home folder name..." << COLOR_RESET << std::endl;
        std::string cmd = "ls -1 /mnt/home | grep -v '^\\.' | head -1";
        FILE* pipe = popen(cmd.c_str(), "r");
        if (pipe) {
            char buffer[128];
            std::string home_folder;
            if (fgets(buffer, sizeof(buffer), pipe) != nullptr) {
                home_folder = buffer;
                home_folder.erase(std::remove(home_folder.begin(), home_folder.end(), '\n'), home_folder.end());
            }
            pclose(pipe);

            if (!home_folder.empty()) {
                std::string user_places_file = "/mnt/home/" + home_folder + "/.local/share/user-places.xbel";
                std::string sed_cmd = "sed -i 's/apex/" + home_folder + "/g' " + user_places_file;
                execute_command(sed_cmd);
                std::cout << COLOR_GREEN << "Updated user-places.xbel: replaced 'apex' with '" << home_folder << "'" << COLOR_RESET << std::endl;
            }
        }

        std::cout << COLOR_PURPLE << "Apex CKGE Minimal Dev installation completed!" << COLOR_RESET << std::endl;
        prompt_reboot();
    }

    // Function to install Spitfire CKGE Full Dev
    void install_spitfire_ckge_full_dev(const std::string& drive) {
        std::cout << COLOR_ORANGE << "Installing Spitfire CKGE Full Dev..." << COLOR_RESET << std::endl;

        prepare_target_partitions(drive);
        std::string efi_part = drive + "1";
        std::string root_part = drive + "2";

        setup_btrfs_subvolumes(root_part);

        execute_command("cp -r /opt/claudemods-distribution-installer/vconsole.conf /mnt/etc");

        execute_command("cp -r /etc/resolv.conf /mnt/etc");
        execute_command("unzip -o /opt/claudemods-distribution-installer/pacman.d.zip -d /mnt/etc");
        execute_command("unzip -o /opt/claudemods-distribution-installer/pacman.d.zip -d /etc");
        execute_command("cp -r /opt/claudemods-distribution-installer/pacman.conf /mnt/etc");
        execute_command("cp -r /opt/claudemods-distribution-installer/pacman.conf /etc");

        execute_command("pacman -Sy");
        execute_command("pacstrap /mnt claudemods-desktop-fulldev");
        execute_command("mkdir -p /mnt/boot");
        execute_command("mkdir -p /mnt/boot/grub");
        execute_command("mount " + efi_part + " /mnt/boot/efi");
        execute_command("touch /mnt/boot/grub/grub.cfg.new");

        execute_command("chroot /mnt /bin/bash -c \"systemctl enable sddm\"");
        execute_command("chroot /mnt /bin/bash -c \"systemctl enable NetworkManager\"");

        install_grub_btrfs(drive);

        std::cout << COLOR_CYAN << "setting up Custom grub and plymouth" << COLOR_RESET << std::endl;
        execute_command("cp -r /opt/claudemods-distribution-installer/spitfire-ckge-minimal/grub /mnt/etc/default");
        execute_command("cp -r /opt/claudemods-distribution-installer/spitfire-ckge-minimal/grub.cfg /mnt/boot/grub");
        execute_command("cp -r /opt/claudemods-distribution-installer/spitfire-ckge-minimal/cachyos /mnt/usr/share/grub/themes");
        execute_command("chroot /mnt /bin/bash -c \"grub-mkconfig -o /boot/grub/grub.cfg\"");
        execute_command("cp -r /opt/claudemods-distribution-installer/spitfire-ckge-minimal/cachyos-bootanimation /mnt/usr/share/plymouth/themes/");
        execute_command("cp -r /opt/claudemods-distribution-installer/spitfire-ckge-minimal/termfull.sh /mnt/usr/local/bin");
        execute_command("chroot /mnt /bin/bash -c \"chmod +x /usr/local/bin/termfull.sh\"");
        execute_command("cp -r /opt/claudemods-distribution-installer/spitfire-ckge-minimal/termfull.service /mnt/etc/systemd/system/");
        execute_command("chroot /mnt /bin/bash -c \"systemctl enable termfull.service >/dev/null 2>&1\"");
        execute_command("chroot /mnt /bin/bash -c \"plymouth-set-default-theme -R cachyos-bootanimation\"");

        std::cout << COLOR_CYAN << "setting up newuser" << COLOR_RESET << std::endl;
        create_new_user(fs_type, drive);

        execute_command("chmod +x /mnt/home/" + new_username + "/.config/fish/config.fish");
        execute_command("chroot /mnt /bin/bash -c \"chmod +x /usr/share/fish/config.fish\"");

        std::cout << COLOR_CYAN << "setting up Spitfire theme and tweaks" << COLOR_RESET << std::endl;
        execute_cd_command("cd /mnt");
        execute_command("wget --show-progress --no-check-certificate --continue --tries=10 --timeout=30 --waitretry=5 https://claudemodsreloaded.co.uk/claudemods-desktop/spitfire-full.zip");
        execute_command("wget --show-progress --no-check-certificate --continue --tries=10 --timeout=30 --waitretry=5 https://claudemodsreloaded.co.uk/arch-systemtool/Arch-Systemtool.zip");
        execute_command("unzip -o /mnt/Arch-Systemtool.zip -d /mnt/opt");
        execute_command("unzip -o /mnt/spitfire-full.zip -d /mnt/home/" + new_username + "/");
        execute_command("mkdir -p /mnt/etc/sddm.conf.d");
        execute_command("cp -r /opt/claudemods-distribution-installer/spitfire-ckge-minimal/kde_settings.conf /mnt/etc/sddm.conf.d");
        execute_command("cp -r /opt/claudemods-distribution-installer/spitfire-ckge-minimal/tweaksspitfire.sh /mnt/opt");
        execute_command("chmod +x /mnt/opt/tweaksspitfire.sh");
        execute_command("chroot /mnt /bin/bash -c \"su - " + new_username + " -c 'cd /opt && ./tweaksspitfire.sh " + new_username + "'\"");
        execute_command("cp -r /opt/claudemods-distribution-installer/spitfire-ckge-minimal/konsolerc /mnt/home/" + new_username + "/.config/");
        execute_command("cp -r /opt/claudemods-distribution-installer/spitfire-ckge-minimal/SpitFireLogin /mnt/usr/share/sddm/themes");
        execute_command("cp -r /opt/claudemods-distribution-installer/spitfire-ckge-minimal/claudemods-cyan.colorscheme /mnt/home/" + new_username + "/.local/share/konsole");
        execute_command("cp -r /opt/claudemods-distribution-installer/spitfire-ckge-minimal/claudemods-cyan.profile /mnt/home/" + new_username + "/.local/share/konsole");
        execute_command("rm -rf /mnt/Arch-Systemtool.zip");
        execute_command("rm -rf /mnt/spitfire-full.zip");
        execute_command("rm -rf /mnt/opt/tweaksspitfire.sh");

        // Fix user-places.xbel with actual home folder name
        std::cout << COLOR_CYAN << "Fixing user-places.xbel with actual home folder name..." << COLOR_RESET << std::endl;
        std::string cmd = "ls -1 /mnt/home | grep -v '^\\.' | head -1";
        FILE* pipe = popen(cmd.c_str(), "r");
        if (pipe) {
            char buffer[128];
            std::string home_folder;
            if (fgets(buffer, sizeof(buffer), pipe) != nullptr) {
                home_folder = buffer;
                home_folder.erase(std::remove(home_folder.begin(), home_folder.end(), '\n'), home_folder.end());
            }
            pclose(pipe);

            if (!home_folder.empty()) {
                std::string user_places_file = "/mnt/home/" + home_folder + "/.local/share/user-places.xbel";
                std::string sed_cmd = "sed -i 's/spitfire/" + home_folder + "/g' " + user_places_file;
                execute_command(sed_cmd);
                std::cout << COLOR_GREEN << "Updated user-places.xbel: replaced 'spitfire' with '" << home_folder << "'" << COLOR_RESET << std::endl;
            }
        }

        std::cout << COLOR_ORANGE << "Spitfire CKGE Full Dev installation completed!" << COLOR_RESET << std::endl;
        prompt_reboot();
    }

    // Function to install Apex CKGE Full Dev
    void install_apex_ckge_full_dev(const std::string& drive) {
        std::cout << COLOR_PURPLE << "Installing Apex CKGE Full Dev..." << COLOR_RESET << std::endl;

        prepare_target_partitions(drive);
        std::string efi_part = drive + "1";
        std::string root_part = drive + "2";

        setup_btrfs_subvolumes(root_part);

        execute_command("cp -r /opt/claudemods-distribution-installer/vconsole.conf /mnt/etc");

        execute_command("cp -r /etc/resolv.conf /mnt/etc");
        execute_command("unzip -o /opt/claudemods-distribution-installer/pacman.d.zip -d /mnt/etc");
        execute_command("unzip -o /opt/claudemods-distribution-installer/pacman.d.zip -d /etc");
        execute_command("cp -r /opt/claudemods-distribution-installer/pacman.conf /mnt/etc");
        execute_command("cp -r /opt/claudemods-distribution-installer/pacman.conf /etc");

        execute_command("pacman -Sy");
        execute_command("pacstrap /mnt claudemods-desktop-fulldev");
        execute_command("mkdir -p /mnt/boot");
        execute_command("mkdir -p /mnt/boot/grub");
        execute_command("mount " + efi_part + " /mnt/boot/efi");
        execute_command("touch /mnt/boot/grub/grub.cfg.new");

        execute_command("chroot /mnt /bin/bash -c \"systemctl enable sddm\"");
        execute_command("chroot /mnt /bin/bash -c \"systemctl enable NetworkManager\"");

        install_grub_btrfs(drive);

        std::cout << COLOR_CYAN << "setting up Custom grub and plymouth" << COLOR_RESET << std::endl;
        execute_command("cp -r /opt/claudemods-distribution-installer/apex-ckge-minimal/grub /mnt/etc/default");
        execute_command("cp -r /opt/claudemods-distribution-installer/apex-ckge-minimal/grub.cfg /mnt/boot/grub");
        execute_command("cp -r /opt/claudemods-distribution-installer/apex-ckge-minimal/cachyos /mnt/usr/share/grub/themes");
        execute_command("chroot /mnt /bin/bash -c \"grub-mkconfig -o /boot/grub/grub.cfg\"");
        execute_command("cp -r /opt/claudemods-distribution-installer/spitfire-ckge-minimal/cachyos-bootanimation /mnt/usr/share/plymouth/themes/");
        execute_command("cp -r /opt/claudemods-distribution-installer/apex-ckge-minimal/termfull.sh /mnt/usr/local/bin");
        execute_command("chroot /mnt /bin/bash -c \"chmod +x /usr/local/bin/termfull.sh\"");
        execute_command("cp -r /opt/claudemods-distribution-installer/apex-ckge-minimal/termfull.service /mnt/etc/systemd/system/");
        execute_command("chroot /mnt /bin/bash -c \"systemctl enable termfull.service >/dev/null 2>&1\"");
        execute_command("chroot /mnt /bin/bash -c \"plymouth-set-default-theme -R cachyos-bootanimation\"");

        std::cout << COLOR_CYAN << "setting up newuser" << COLOR_RESET << std::endl;
        create_new_user(fs_type, drive);

        execute_command("chmod +x /mnt/home/" + new_username + "/.config/fish/config.fish");
        execute_command("chroot /mnt /bin/bash -c \"chmod +x /usr/share/fish/config.fish\"");

        std::cout << COLOR_CYAN << "setting up Apex theme and tweaks" << COLOR_RESET << std::endl;
        execute_cd_command("cd /mnt");
        execute_command("wget --show-progress --no-check-certificate --continue --tries=10 --timeout=30 --waitretry=5 https://claudemodsreloaded.co.uk/claudemods-desktop/apex-full.zip");
        execute_command("wget --show-progress --no-check-certificate --continue --tries=10 --timeout=30 --waitretry=5 https://claudemodsreloaded.co.uk/arch-systemtool/Arch-Systemtool.zip");
        execute_command("unzip -o /mnt/Arch-Systemtool.zip -d /mnt/opt");
        execute_command("unzip -o /mnt/apex-full.zip -d /mnt/home/" + new_username + "/");
        execute_command("mkdir -p /mnt/etc/sddm.conf.d");
        execute_command("cp -r /opt/claudemods-distribution-installer/apex-ckge-minimal/kde_settings.conf /mnt/etc/sddm.conf.d");
        execute_command("cp -r /opt/claudemods-distribution-installer/apex-ckge-minimal/tweaksapex.sh /mnt/opt");
        execute_command("chmod +x /mnt/opt/tweaksapex.sh");
        execute_command("chroot /mnt /bin/bash -c \"su - " + new_username + " -c 'cd /opt && ./tweaksapex.sh " + new_username + "'\"");
        execute_command("cp -r /opt/claudemods-distribution-installer/apex-ckge-minimal/konsolerc /mnt/home/" + new_username + "/.config/");
        execute_command("cp -r /opt/claudemods-distribution-installer/apex-ckge-minimal/ApexLogin2 /mnt/usr/share/sddm/themes");
        execute_command("cp -r /opt/claudemods-distribution-installer/apex-ckge-minimal/claudemods-cyan.colorscheme /mnt/home/" + new_username + "/.local/share/konsole");
        execute_command("cp -r /opt/claudemods-distribution-installer/apex-ckge-minimal/claudemods-cyan.profile /mnt/home/" + new_username + "/.local/share/konsole");
        execute_command("rm -rf /mnt/Arch-Systemtool.zip");
        execute_command("rm -rf /mnt/apex-full.zip");
        execute_command("rm -rf /mnt/opt/tweaksapex.sh");

        // Fix user-places.xbel with actual home folder name
        std::cout << COLOR_CYAN << "Fixing user-places.xbel with actual home folder name..." << COLOR_RESET << std::endl;
        std::string cmd = "ls -1 /mnt/home | grep -v '^\\.' | head -1";
        FILE* pipe = popen(cmd.c_str(), "r");
        if (pipe) {
            char buffer[128];
            std::string home_folder;
            if (fgets(buffer, sizeof(buffer), pipe) != nullptr) {
                home_folder = buffer;
                home_folder.erase(std::remove(home_folder.begin(), home_folder.end(), '\n'), home_folder.end());
            }
            pclose(pipe);

            if (!home_folder.empty()) {
                std::string user_places_file = "/mnt/home/" + home_folder + "/.local/share/user-places.xbel";
                std::string sed_cmd = "sed -i 's/apex/" + home_folder + "/g' " + user_places_file;
                execute_command(sed_cmd);
                std::cout << COLOR_GREEN << "Updated user-places.xbel: replaced 'apex' with '" << home_folder << "'" << COLOR_RESET << std::endl;
            }
        }

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

    // Function to display main menu with arrow keys
    void main_menu() {
        // Initialize default values
        if (selected_drive.empty()) selected_drive = "Not set";
        fs_type = "btrfs";
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

            int choice = show_menu(main_options, "Btrfs Arch Linux Installer - Main Menu", current_selection);

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
                    std::cout << COLOR_GREEN << "Using Btrfs filesystem with Zstd compression" << COLOR_RESET << std::endl;
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
                                                "Vanilla Arch",
                                                "CachyOS",
                                                "claudemods Distributions"
                                            };

                                            int install_choice = show_menu(install_options, "Select Installation Type");

                                            switch(install_choice) {
                                                case 0:
                                                    installation_type = "Vanilla Arch";
                                                    // Show desktop environment selection for Vanilla Arch
                                                    {
                                                        std::vector<std::string> desktop_options = {
                                                            "Arch TTY Grub",
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
            } else {
                install_desktop("btrfs", selected_drive);
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
    }

public:
    // Main script
    void run(int argc, char* argv[]) {
        display_header();

        // Step 1: Drive selection (now accepts command line argument)
        get_drive_selection(argc, argv);

        // Show main menu for Btrfs
        main_menu();
    }
};

#endif // BTRFSINSTALLER_H
