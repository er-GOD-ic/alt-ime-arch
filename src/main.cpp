#include<iostream>
#include<fcntl.h>
#include<unistd.h>
#include<linux/input.h>

int main() {
  const char* device = "/dev/input/event9";
  int fd = open(device, O_RDONLY);
  if (fd == -1) {
    perror("Failed to open device");
    return 1;
  }

  const int lAlt = 56;
  const int rAlt = 100;

  struct input_event ev;
  std::cout << "Reading events (Ctrl+C to quit)...\n";
  while (read(fd, &ev, sizeof(ev)) == sizeof(ev)) {
    if (ev.type == EV_KEY && ev.value == 0) {
      if (ev.code == lAlt) {
        system("fcitx5-remote -c");
      }
      if (ev.code == rAlt) {
        system("fcitx5-remote -o");
      }
    }
  }

  close(fd);

  return 0;
}
