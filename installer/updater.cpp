#include <iostream>
#include <cstdlib>
#include <cstring>
#include <unistd.h>
#include <pthread.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <array>
#include <memory>
#include <string>
#include <limits>

#define COLOR_GREEN "\033[38;2;0;255;0m"
#define COLOR_CYAN "\033[38;2;0;255;255m"
#define COLOR_RED "\033[38;2;255;0;0m"
#define COLOR_YELLOW "\033[38;2;255;255;0m"
#define COLOR_RESET "\033[0m"

char detected_distro[64] = "";
const char* executable_name = "cmiimg";
bool commands_completed = false;
bool loading_complete = false;
char current_version[64] = "unknown";
char downloaded_version[64] = "unknown";
char installed_version[64] = "unknown";
char username[256] = "";

// Function to get current username
const char* get_username() {
    if (username[0] == '\0') {
        const char* user_env = getenv("USER");
        if (user_env) {
            strncpy(username, user_env, sizeof(username) - 1);
        } else {
            // Fallback to getpwuid if USER environment variable is not set
            struct passwd *pw = getpwuid(getuid());
            if (pw) {
                strncpy(username, pw->pw_name, sizeof(username) - 1);
            } else {
                strcpy(username, "unknown");
            }
        }
    }
    return username;
}

void silent_command(const char* cmd) {
    char full_cmd[1024];
    snprintf(full_cmd, sizeof(full_cmd), "%s >/dev/null 2>&1", cmd);
    system(full_cmd);
}

std::string run_command(const char* cmd) {
    std::array<char, 128> buffer;
    std::string result;
    std::unique_ptr<FILE, decltype(&pclose)> pipe(popen(cmd, "r"), pclose);
    if (!pipe) {
        throw std::runtime_error("popen() failed!");
    }
    while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr) {
        result += buffer.data();
    }
    if (!result.empty() && result.back() == '\n') {
        result.pop_back();
    }
    return result;
}

void* execute_update_thread(void* /*arg*/) {
    while (!loading_complete) usleep(10000);
    
    const char* user = get_username();
    
    // 1. GIT CLONE
    char git_cmd[512];
    snprintf(git_cmd, sizeof(git_cmd), "cd /home/%s/ && git clone https://github.com/claudemods-distribution-installer.git", user);
    silent_command(git_cmd);
    
    // 2. CURRENT VERSION
    try {
        std::string version_output = run_command("cat /opt/claudemods-distribution-installer/version.txt");
        strncpy(current_version, version_output.empty() ? "not installed" : version_output.c_str(), 
                sizeof(current_version) - 1);
    } catch (...) {
        strcpy(current_version, "not installed");
    }
    
    // 3. DETECT DISTRO (ONLY ARCH AND CACHYOS SUPPORTED)
    try {
        std::string distro_output = run_command("cat /etc/os-release | grep '^ID=' | cut -d'=' -f2 | tr -d '\"'");
        if (distro_output == "arch" || distro_output == "cachyos") {
            strcpy(detected_distro, distro_output.c_str());
        } else {
            std::cout << COLOR_RED << "\nError: Unsupported distribution. Only Arch Linux and CachyOS are supported.\n" << COLOR_RESET;
            exit(EXIT_FAILURE);
        }
    } catch (...) {
        strcpy(detected_distro, "unknown");
        std::cout << COLOR_RED << "\nError: Could not detect distribution.\n" << COLOR_RESET;
        exit(EXIT_FAILURE);
    }
    
    // 4. DOWNLOADED VERSION
    if (strcmp(detected_distro, "arch") == 0 || strcmp(detected_distro, "cachyos") == 0) {
        try {
            char version_cmd[512];
            snprintf(version_cmd, sizeof(version_cmd), "cat /home/%s/claudemods-distribution-installer/installer/version.txt", user);
            std::string version_output = run_command(version_cmd);
            strncpy(downloaded_version, version_output.c_str(), sizeof(downloaded_version) - 1);
        } catch (...) {
            strcpy(downloaded_version, "unknown");
        }
    }
    
    // INSTALLATION PROCESS
    run_command("sudo rm -rf /opt/claudemods-distribution-installer/mainscript");
    run_command("sudo rm -rf /opt/claudemods-distribution-installer/updater");
    
    // ARCH AND CACHYOS INSTALLATION
    if (strcmp(detected_distro, "arch") == 0 || strcmp(detected_distro, "cachyos") == 0) {
        char copy_version_cmd[512];
        snprintf(copy_version_cmd, sizeof(copy_version_cmd), "cp -r /home/%s/claudemods-distribution-installer/installer/version.txt /opt/claudemods-distribution-installer", user);
        silent_command(copy_version_cmd);
        
        char compile_main_cmd[512];
        snprintf(compile_main_cmd, sizeof(compile_main_cmd), "cd /home/%s/claudemods-distribution-installer/installer && sudo g++ -o mainscript mainscript.cpp -std=c++23", user);
        silent_command(compile_main_cmd);
        
        char compile_updater_cmd[512];
        snprintf(compile_updater_cmd, sizeof(compile_updater_cmd), "cd /home/%s/claudemods-distribution-installer/installer && sudo g++ -o updater updater.cpp -std=c++23", user);
        silent_command(compile_updater_cmd);
        
        char copy_main_cmd[512];
        snprintf(copy_main_cmd, sizeof(copy_main_cmd), "sudo cp /home/%s/claudemods-distribution-installer/installer/mainscript /opt/claudemods-distribution-installer", user);
        silent_command(copy_main_cmd);
        
        char copy_updater_cmd[512];
        snprintf(copy_updater_cmd, sizeof(copy_updater_cmd), "sudo cp /home/%s/claudemods-distribution-installer/installer/updater /opt/claudemods-distribution-installer", user);
        silent_command(copy_updater_cmd);
    }
    
    // Cleanup
    char cleanup_cmd[512];
    snprintf(cleanup_cmd, sizeof(cleanup_cmd), "rm -rf /home/%s/claudemods-multi-iso-konsole-script", user);
    silent_command(cleanup_cmd);
    
    // GET INSTALLED VERSION
    try {
        std::string installed_version_output = run_command("cat /opt/claudemods-distribution-installer/version.txt");
        strncpy(installed_version, installed_version_output.c_str(), sizeof(installed_version) - 1);
    } catch (...) {
        strcpy(installed_version, "unknown");
    }
    
    commands_completed = true;
    return nullptr;
}

void show_loading_bar() {
    std::cout << COLOR_GREEN << "Progress: [" << COLOR_RESET;
    for (int i = 0; i < 50; i++) {
        std::cout << COLOR_YELLOW << "=" << COLOR_RESET;
        std::cout.flush();
        usleep(50000);
    }
    std::cout << COLOR_GREEN << "] 100%\n" << COLOR_RESET;
    loading_complete = true;
}

int main() {
    pthread_t thread;
    pthread_create(&thread, nullptr, execute_update_thread, nullptr);
    
    show_loading_bar();
    
    while (!commands_completed) usleep(10000);
    pthread_join(thread, nullptr);
    
    // >>> ORIGINAL SUMMARY <<<
    std::cout << COLOR_GREEN << "\nInstallation complete!\n" << COLOR_RESET;
    std::cout << COLOR_GREEN << "Executable installed in location: /opt/claudemods-distribution-installer/mainscript\n" << COLOR_RESET;
    std::cout << COLOR_GREEN << "Detected distro: " << detected_distro << COLOR_RESET << std::endl;
    std::cout << COLOR_GREEN << "Current version: " << current_version << COLOR_RESET << std::endl;
    std::cout << COLOR_GREEN << "Downloaded version: " << downloaded_version << COLOR_RESET << std::endl;
    std::cout << COLOR_GREEN << "Installed version: " << installed_version << COLOR_RESET << std::endl;
    
    std::cout << COLOR_CYAN << "\nLaunch now? (y/n): " << COLOR_RESET;
    char response;
    std::cin >> response;
    
    if (response == 'y' || response == 'Y') {
        system("cd /opt/claudemods-distribution-installer/mainscript && ./mainscript");
    }
    
    return EXIT_SUCCESS;
}
