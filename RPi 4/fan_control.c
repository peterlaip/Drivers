#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>  

int main(int argc, char *argv[]) {
    int fd;
    char buffer[10];  

    if (argc != 3) {
        printf("Usage: %s <fan_number> <state>\n", argv[0]);
        return -1;
    }


    int fan_number = atoi(argv[1]);
    int state = atoi(argv[2]);
    if ((fan_number < 1 || fan_number > 2) || (state < 0 || state > 1)) {
        printf("Invalid arguments. Fan number should be '1' or '2', state should be '0' or '1'.\n");
        return -1;
    }

    fd = open("/dev/fan_driver", O_WRONLY);
    if (fd < 0) {
        perror("Failed to open the device...");
        return -1;
    }


    snprintf(buffer, sizeof(buffer), "%d %d", fan_number, state);

    if (write(fd, buffer, strlen(buffer)) < 0) {
        perror("Failed to write to the device...");
        close(fd);
        return -1;
    }

    close(fd);
    return 0;
}

