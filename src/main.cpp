#include <iostream>
#include <unistd.h>
#include <fcntl.h>
#include <linux/input.h>
#include <linux/uinput.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <cstring>
#include <cstdlib>
#include <string>

class AltIMEController {
private:
    int input_fd;
    int uinput_fd;
    bool lalt_pressed = false;
    bool ralt_pressed = false;
    bool lalt_used_as_modifier = false;
    bool ralt_used_as_modifier = false;

    void setup_uinput() {
        uinput_fd = open("/dev/uinput", O_WRONLY | O_NONBLOCK);
        if (uinput_fd < 0) {
            perror("Cannot open /dev/uinput");
            exit(1);
        }

        // Enable key events
        if (ioctl(uinput_fd, UI_SET_EVBIT, EV_KEY) < 0) {
            perror("UI_SET_EVBIT");
            exit(1);
        }
        if (ioctl(uinput_fd, UI_SET_KEYBIT, KEY_LEFTALT) < 0) {
            perror("UI_SET_KEYBIT LEFTALT");
            exit(1);
        }
        if (ioctl(uinput_fd, UI_SET_KEYBIT, KEY_RIGHTALT) < 0) {
            perror("UI_SET_KEYBIT RIGHTALT");
            exit(1);
        }

        // Enable all keys for forwarding
        for (int key = 0; key < KEY_MAX; key++) {
            ioctl(uinput_fd, UI_SET_KEYBIT, key);
        }

        struct uinput_setup usetup;
        memset(&usetup, 0, sizeof(usetup));
        usetup.id.bustype = BUS_USB;
        usetup.id.vendor = 0x1234;
        usetup.id.product = 0x5678;
        strcpy(usetup.name, "Alt IME Controller");

        if (ioctl(uinput_fd, UI_DEV_SETUP, &usetup) < 0) {
            perror("UI_DEV_SETUP");
            exit(1);
        }
        if (ioctl(uinput_fd, UI_DEV_CREATE) < 0) {
            perror("UI_DEV_CREATE");
            exit(1);
        }
    }

    void send_key_event(int key, int value) {
        struct input_event ev;
        memset(&ev, 0, sizeof(ev));
        
        ev.type = EV_KEY;
        ev.code = key;
        ev.value = value;
        gettimeofday(&ev.time, nullptr);
        
        write(uinput_fd, &ev, sizeof(ev));
        
        // Send sync event
        ev.type = EV_SYN;
        ev.code = SYN_REPORT;
        ev.value = 0;
        write(uinput_fd, &ev, sizeof(ev));
    }

    void toggle_ime(bool turn_on) {
        // Get the original user info
        const char* sudo_user = getenv("SUDO_USER");
        const char* sudo_uid = getenv("SUDO_UID");
        const char* sudo_gid = getenv("SUDO_GID");
        
        if (sudo_user && sudo_uid && sudo_gid) {
            // Switch to original user context for dbus communication
            std::string command;
            if (turn_on) {
                command = "sudo -u " + std::string(sudo_user) + " DISPLAY=:0 fcitx5-remote -o";
                std::cout << "IME ON (Left Alt)" << std::endl;
            } else {
                command = "sudo -u " + std::string(sudo_user) + " DISPLAY=:0 fcitx5-remote -c";
                std::cout << "IME OFF (Right Alt)" << std::endl;
            }
            
            int result = system(command.c_str());
            if (result != 0) {
                std::cerr << "Warning: fcitx5-remote command failed" << std::endl;
            }
        } else {
            // Fallback: try direct command
            if (turn_on) {
                system("fcitx5-remote -o");
                std::cout << "IME ON (Left Alt)" << std::endl;
            } else {
                system("fcitx5-remote -c");
                std::cout << "IME OFF (Right Alt)" << std::endl;
            }
        }
    }

public:
    AltIMEController(const char* input_device) {
        input_fd = open(input_device, O_RDONLY);
        if (input_fd < 0) {
            perror("Cannot open input device");
            exit(1);
        }
        setup_uinput();
    }

    ~AltIMEController() {
        if (input_fd >= 0) close(input_fd);
        if (uinput_fd >= 0) {
            ioctl(uinput_fd, UI_DEV_DESTROY);
            close(uinput_fd);
        }
    }

    void run() {
        struct input_event ev;
        
        std::cout << "Alt IME Controller started..." << std::endl;
        std::cout << "Left Alt: IME ON" << std::endl;
        std::cout << "Right Alt: IME OFF" << std::endl;
        std::cout << "Alt + other keys: Normal Alt behavior" << std::endl;

        while (true) {
            ssize_t bytes = read(input_fd, &ev, sizeof(ev));
            if (bytes != sizeof(ev)) continue;

            if (ev.type == EV_KEY) {
                handle_key_event(ev.code, ev.value);
            }
        }
    }

private:
    void handle_key_event(int key, int value) {
        if (key == KEY_LEFTALT) {
            if (value == 1) { // Key press
                lalt_pressed = true;
                lalt_used_as_modifier = false;
            } else if (value == 0 && lalt_pressed) { // Key release
                lalt_pressed = false;
                
                if (!lalt_used_as_modifier) {
                    // Single Left Alt press - turn IME ON
                    toggle_ime(false);
                } else {
                    // Was used as modifier - send Alt release
                    send_key_event(KEY_LEFTALT, 0);
                }
            }
        } else if (key == KEY_RIGHTALT) {
            if (value == 1) { // Key press
                ralt_pressed = true;
                ralt_used_as_modifier = false;
            } else if (value == 0 && ralt_pressed) { // Key release
                ralt_pressed = false;
                
                if (!ralt_used_as_modifier) {
                    // Single Right Alt press - turn IME OFF
                    toggle_ime(true);
                } else {
                    // Was used as modifier - send Alt release
                    send_key_event(KEY_RIGHTALT, 0);
                }
            }
        } else {
            // Other key event
            if (value == 1) { // Key press
                if (lalt_pressed && !lalt_used_as_modifier) {
                    // Left Alt is being used as modifier
                    lalt_used_as_modifier = true;
                    send_key_event(KEY_LEFTALT, 1);
                }
                if (ralt_pressed && !ralt_used_as_modifier) {
                    // Right Alt is being used as modifier
                    ralt_used_as_modifier = true;
                    send_key_event(KEY_RIGHTALT, 1);
                }
            }
            
            // Forward the key event normally
            send_key_event(key, value);
        }
    }
};

int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cout << "Usage: " << argv[0] << " <input_device>" << std::endl;
        std::cout << "Example: " << argv[0] << " /dev/input/event0" << std::endl;
        std::cout << "\nTo find your keyboard device:" << std::endl;
        std::cout << "sudo libinput list-devices" << std::endl;
        std::cout << "or check /proc/bus/input/devices" << std::endl;
        return 1;
    }

    // Check if we can access the input device
    if (access(argv[1], R_OK) != 0) {
        std::cout << "Cannot access input device: " << argv[1] << std::endl;
        std::cout << "Make sure you have proper permissions or run as root." << std::endl;
        std::cout << "To fix permissions, run:" << std::endl;
        std::cout << "  sudo usermod -a -G input $USER" << std::endl;
        std::cout << "  sudo tee /etc/udev/rules.d/99-uinput.rules << EOF" << std::endl;
        std::cout << "  KERNEL==\"uinput\", GROUP=\"input\", MODE=\"0660\"" << std::endl;
        std::cout << "  EOF" << std::endl;
        std::cout << "  sudo udevadm control --reload-rules && sudo udevadm trigger" << std::endl;
        std::cout << "Then logout and login again." << std::endl;
        return 1;
    }

    try {
        AltIMEController controller(argv[1]);
        controller.run();
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
