/*

Put this inside of your .xinitrc file:
ssb-dwm&

*/

#include "config.h"

#include <X11/Xlib.h>
#include <stdio.h>
#include <string.h>
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

void get_battery(char *out) {
  int capacity = 0;
  char status[12];
  char s = '?';
  FILE *cp = fopen("/sys/class/power_supply/BAT0/capacity", "r");
  FILE *st = fopen("/sys/class/power_supply/BAT0/status", "r");

  if (!(cp || st)) {
    snprintf(out, 11, "No battery");
    return;
  }
  fscanf(cp, "%d", &capacity);
  fscanf(st, "%s", status);
  if (strcmp(status, "Charging") == 0) {
    s = '+';
  }
  if (strcmp(status, "Discharging") == 0) {
    s = '-';
  }
  if (strcmp(status, "Full") == 0) {
    s = '=';
  }

  fclose(cp);
  fclose(st);
  snprintf(out, 15, battery_format, capacity, s);
}

void get_cpu(char *out) {
  FILE *fl = fopen("/proc/stat", "r");
  char line[256];
  int usr = 0, ni = 0, sys = 0, idl = 0, io = 0, irq = 0, soft = 0;

  while (fgets(line, sizeof(line), fl) != NULL) {
    if (strncmp(line, "cpu ", 4) == 0) {
      sscanf(line, "%*s %d %d %d %d %d %d %d", &usr, &ni, &sys, &idl, &io, &irq,
             &soft);
      break;
    }
  }
  fclose(fl);

  int current_total = usr + ni + sys + idl + io + irq + soft;
  int current_idle = idl + io;

  static int prev_total = 0;
  static int prev_idle = 0;

  float usage = 0.0;

  int diff_total = current_total - prev_total;
  int diff_idle = current_idle - prev_idle;

  if (diff_total > 0 && prev_total > 0) {
    usage = (1.0 - ((float)diff_idle / diff_total)) * 100.0;
  }
  prev_total = current_total;
  prev_idle = current_idle;
  snprintf(out, 10, cpu_format, usage);
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
