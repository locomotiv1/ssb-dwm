#include <X11/Xlib.h>
#include <alsa/asoundlib.h>
#include <alsa/control.h>
#include <fcntl.h>
#include <stdio.h>
#include <time.h>
#include <unistd.h>

char get_date(char *buffer, size_t max_size) {
  time_t raw_time;
  time(&raw_time);

  struct tm *local_time = localtime(&raw_time);
  return strftime(buffer, max_size, "%a %b %d | %R", local_time); // make user be able to format this
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
    snd_mixer_elem_t* elem = snd_mixer_find_selem(handle, sid);

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

int main(void) {
  Display *dpy = XOpenDisplay(NULL);
  if (!dpy)
    return 1;

  Window root = DefaultRootWindow(dpy);

  while (1) {
    char date[40];
    char vol[10];
    char status[400];

    get_date(date, sizeof(date));

    snprintf(status, sizeof(status), " %s Vol: %d ", date, get_vol());

    XStoreName(dpy, root, status);
    XFlush(dpy);

    sleep(1);
  }

  XCloseDisplay(dpy);
  return 0;
}
