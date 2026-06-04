#include <X11/Xlib.h>
#include <stdio.h>
#include <unistd.h>
#include <time.h>

void date_info(char *buffer, size_t max_size) {
    time_t raw_time;
    time(&raw_time);

    struct tm *local_time = localtime(&raw_time);
    strftime(buffer, max_size, "%a %b %d | %R", local_time);
}

int main(void) {
    Display *dpy = XOpenDisplay(NULL);
    if (!dpy) return 1;
    
    Window root = DefaultRootWindow(dpy);

    while (1) {
        char date_str[40];
        char final_status[256];

        date_info(date_str, sizeof(date_str));

        snprintf(final_status, sizeof(final_status), " %s ", date_str);

        XStoreName(dpy, root, final_status);
        XFlush(dpy);

        sleep(1);
    }

    XCloseDisplay(dpy);
    return 0;
}
