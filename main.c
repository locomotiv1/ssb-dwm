#include <X11/Xlib.h>
#include <alsa/asoundlib.h>
#include <alsa/control.h>
#include <fcntl.h>
#include <stdio.h>
#include <time.h>
#include <unistd.h>

char *get_date() {
  time_t raw_time;
  time(&raw_time);
  struct tm *local_time = localtime(&raw_time);
  static char buffer[35];

  strftime(buffer, sizeof(buffer), "%a %b %d | %R", local_time); // make user be able to format this
  return buffer;
}

int get_vol(void) {
  long min, max, vol;
  int unmuted;
  snd_mixer_t *handle;
  snd_mixer_selem_id_t *sid;
  const char *card = "default";

  const char *selem_name = "Master";

  snd_mixer_open(&handle, 0);
  snd_mixer_attach(handle, card);
  snd_mixer_selem_register(handle, NULL, NULL);
  snd_mixer_load(handle);

  snd_mixer_selem_id_alloca(&sid);
  snd_mixer_selem_id_set_index(sid, 0);
  snd_mixer_selem_id_set_name(sid, selem_name);
  snd_mixer_elem_t *elem = snd_mixer_find_selem(handle, sid);

  if (!elem) {
    snd_mixer_close(handle);
    return -1;
  }

  snd_mixer_selem_get_playback_volume_range(elem, &min, &max);

  // Get the current volume (using the Left channel as the standard)
  snd_mixer_selem_get_playback_volume(elem, SND_MIXER_SCHN_FRONT_LEFT, &vol); // TODO: Handle Muted string

  snd_mixer_close(handle);

  int percentage = (int)(((double)(vol - min) / (double)(max - min)) * 100);

  return percentage;
}

typedef struct {
    long total;
    long used;
} RamInfo;


RamInfo get_ram() {
  RamInfo info = { .total = 0, .used = 0 };

  char buffer[100];
  int found = 0;
  FILE *fl = fopen("/proc/meminfo", "r");

  while (fgets(buffer, sizeof(buffer), fl)) {
    if (strncmp(buffer, "MemTotal:", 9) == 0) {
      sscanf(buffer, "MemTotal: %lu", &info.total);
      found++;
    }
    else if (strncmp(buffer, "MemAvailable:", 13) == 0) {
      sscanf(buffer, "MemAvailable: %lu", &info.used);
      found++;
    }

    if (found == 2) {
      break;
    }
  }

  fclose(fl);
  return info;
}

int main(void) {
  Display *dpy = XOpenDisplay(NULL);
  if (!dpy)
    return 1;



  Window root = DefaultRootWindow(dpy);

  while (1) {
    char status[400];

    RamInfo ram;
    ram = get_ram();

    double total_gb = (double)ram.total / 1048576.0;
    double used_gb = (double)ram.used / 1048576.0;

    snprintf(status, sizeof(status), " %s  Vol: %d RAM: %lf %lf", get_date(), get_vol(), total_gb, used_gb);

    XStoreName(dpy, root, status);
    XFlush(dpy);

    sleep(1);
  }

  XCloseDisplay(dpy);
  return 0;
}
