#include "config.h"
#include <X11/Xlib.h>
#include <alsa/asoundlib.h>
#include <alsa/control.h>
#include <fcntl.h>
#include <stdio.h>
#include <time.h>
#include <unistd.h>

void get_date(char *out) {
  time_t raw_time;
  time(&raw_time);
  struct tm *local_time = localtime(&raw_time);

  strftime(out, 70, date_format, local_time);
}

void get_vol(char *out) {
  long min, max, vol;
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

  snd_mixer_selem_get_playback_volume_range(elem, &min, &max);

  // Get the current volume (using the Left channel as the standard)
  snd_mixer_selem_get_playback_volume(elem, SND_MIXER_SCHN_FRONT_LEFT,
                                      &vol); // TODO: Handle Muted string
  snd_mixer_close(handle);

  int percentage = (int)(((double)(vol - min) / (double)(max - min)) * 100);
  snprintf(out, 10, "%d", percentage);
}

void get_ram(char *out) {
  // for some reason it constantly shows 300mb more that its actually using

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

void get_mic(char *out) {
    long min, max, vol;
    snd_mixer_t *handle;
    snd_mixer_selem_id_t *sid;
    const char *card = "default";
    
    // Most Linux systems use "Capture" for the default microphone.
    // If this fails, run `amixer scontrols` to find your specific mic name
    const char *selem_name = "Capture"; 

    snd_mixer_open(&handle, 0);
    snd_mixer_attach(handle, card);
    snd_mixer_selem_register(handle, NULL, NULL);
    snd_mixer_load(handle);

    snd_mixer_selem_id_alloca(&sid);
    snd_mixer_selem_id_set_index(sid, 0);
    snd_mixer_selem_id_set_name(sid, selem_name);
    snd_mixer_elem_t* elem = snd_mixer_find_selem(handle, sid);

    if (!elem) {
        snprintf(out, 15, "ERR");
        snd_mixer_close(handle);
        snd_config_update_free_global();
        return;
    }

    snd_mixer_selem_get_capture_volume_range(elem, &min, &max);
    
    snd_mixer_selem_get_capture_volume(elem, SND_MIXER_SCHN_FRONT_LEFT, &vol); 

    snd_mixer_close(handle);
    snd_config_update_free_global();

    int percentage = (int)(((double)(vol - min) / (double)(max - min)) * 100);
    snprintf(out, 10, "%d", percentage);
}

int main(void) {
  Display *dpy = XOpenDisplay(NULL);
  if (!dpy)
    return 1;
  Window root = DefaultRootWindow(dpy);

  for (;;sleep(1)){
    char status[400];
    status[0] = '\0';

    char module_output[100];
    char formatted_block[150];

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
