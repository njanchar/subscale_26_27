#include <stdio.h>
#include <stdlib.h>
#include <time.h>

int take_capture() {
    char filename[128];
    char command[512];

    // Generate a  timestamped filename so previous photos aren't overwritten
    time_t now = time(NULL);
    struct tm *t = localtime(&now);
    strftime(filename, sizeof(filename), "../../captures/ai_pic_%Y%m%d_%H%M%S.jpg", t);

    // -n Headless mode no GUI preview
    // -t 2000 -> wait 2 seconds
    // --width 1280 --height 720
    snprintf(command, sizeof(command),
             "rpicam-still -n -t 2000 --width 1280 --height 720 "
             "--post-process-file /usr/share/rpi-camera-assets/imx500_mobilenet_ssd.json "
             "-o %s",
             filename);

    printf("Initializing AI Camera and capturing to %s...\n", filename);
    int status = system(command);

    if (status == 0) {
        printf("Success! Saved to %s\n", filename);
    } else {
        fprintf(stderr, "Capture failed with exit code %d\n", status);
        return 1;
    }

    return 0;
}