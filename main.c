#include "config.h"
#include <X11/Xlib.h>
#include <alsa/asoundlib.h>
#include <alsa/control.h>
#include <fcntl.h>
#include <stdio.h>
#include <sys/statvfs.h>
#include <time.h>
#include <unistd.h>

void get_date(char *out) {
  time_t raw_time;
  time(&raw_time);
  struct tm *local_time = localtime(&raw_time);

  strftime(out, 70, date_format, local_time);
}

void get_ram(char *out) {
  // Fix the GiB format

  char buffer[100];
  long total = 0;
  long available = 0;
  int found = 0;
  FILE *fl = fopen("/proc/meminfo", "r");

  while (fgets(buffer, sizeof(buffer), fl)) {
    if (strncmp(buffer, "MemTotal:", 9) == 0) {
      sscanf(buffer, "MemTotal: %lu", &total);
      found++;
    } else if (strncmp(buffer, "MemAvailable:", 13) == 0) {
      sscanf(buffer, "MemAvailable: %lu", &available);
      found++;
    }

    if (found == 2) {
      long used = total - available;
      double total_gb = (double)total / 1048576.0;
      double used_gb = (double)used / 1048576.0;
      snprintf(out, 50, ram_format, used_gb, total_gb);
      break;
    }
  }

  fclose(fl);
}

void get_disk(char *out) {
  const unsigned int GB = (1024.0 * 1024.0) * 1024.0;
  struct statvfs buffer;
  if (statvfs("/", &buffer) == -1) {
    snprintf(out, 50, "statvfs() error");
    return;
  }

  unsigned long long total_blocks = buffer.f_blocks;
  unsigned long long avail_blocks = buffer.f_bavail;
  unsigned long long used_blocks = total_blocks - avail_blocks;

  double used_gb = ((double)used_blocks * buffer.f_frsize) / GB;
  double total_gb = ((double)total_blocks * buffer.f_frsize) / GB;
  int percentage = (used_blocks * 100) / total_blocks;

  snprintf(out, 50, disk_format, used_gb, total_gb, percentage);
}

int main(void) {
  Display *dpy = XOpenDisplay(NULL);
  if (!dpy)
    return 1;
  Window root = DefaultRootWindow(dpy);

  for (;; sleep(30)) {
    char status[400];
    status[0] = '\0';

    char module_output[100];
    char formatted_block[120];

    size_t num_blocks = sizeof(blocks) / sizeof(blocks[0]);

    for (size_t i = 0; i < num_blocks; i++) {

      blocks[i].func(module_output);

      snprintf(formatted_block, sizeof(formatted_block), blocks[i].format,
               module_output);

      strcat(status, formatted_block);
    }

    XStoreName(dpy, root, status);
    XFlush(dpy);
  }

  XCloseDisplay(dpy);
  return 0;
}
