#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>

int main(int argc, char *argv[]) {
    int fd;
    char buffer;

    if (argc != 2) {
        printf("Usage: %s <state>\n", argv[0]);
        return -1;
    }

    fd = open("/dev/coolerchip_driver", O_WRONLY);
    if (fd < 0) {
        perror("Failed to open the device...");
        return -1;
    }

    buffer = argv[1][0];  // State ('1' for on, '0' for off)
    
    write(fd, &buffer, 1);

    close(fd);
    return 0;
}

