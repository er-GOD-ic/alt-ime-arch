#include <iostream>
#include <fcntl.h>
#include <unistd.h>
#include <linux/input.h>
#include <chrono>
#include <unordered_set>

int main(int argc, char* argv[]) {
  if (argc < 2) {
    std::cerr << "Usage: " << argv[0] << " /dev/input/eventX\n";
    return 1;
  }

  const char* device = argv[1];
  int fd = open(device, O_RDONLY);
  if (fd == -1) {
    perror("Failed to open device");
    return 1;
  }

  const int lAlt = 184;
  const int rAlt = 185;

  struct input_event ev;

  // 押されているキーコードを管理する集合
  std::unordered_set<int> pressed_keys;

  // Altキーが押されたかどうかのフラグ
  bool lAlt_pressed = false;
  bool rAlt_pressed = false;

  std::cout << "Reading events from " << device << " (Ctrl+C to quit)...\n";
  while (read(fd, &ev, sizeof(ev)) == sizeof(ev)) {
    if (ev.type == EV_KEY) {
      if (ev.value == 1) { // key press
        pressed_keys.insert(ev.code);
        if (ev.code == lAlt) lAlt_pressed = true;
        if (ev.code == rAlt) rAlt_pressed = true;
      } else if (ev.value == 0) { // key release
        pressed_keys.erase(ev.code);

        if (ev.code == lAlt) {
          lAlt_pressed = false;
          // 他のキーが押されていなければIME切り替え
          if (pressed_keys.empty()) {
            system("fcitx5-remote -c");
          }
        }
        if (ev.code == rAlt) {
          rAlt_pressed = false;
          if (pressed_keys.empty()) {
            system("fcitx5-remote -o");
          }
        }
      }
    }
  }

  close(fd);
  return 0;
}
