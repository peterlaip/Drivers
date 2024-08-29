#include <stdio.h>
#include <stdlib.h>     // system
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>

#define DEVICE_BLTEST "/dev/dht11"
#define TEMP_FILE "/home/user/path/to/current_temperature.txt"

int main(void)
{
    int humidityfd;
    int ret = 0;
    char buf[5];
    unsigned char tempz = 0;
    unsigned char tempx = 0;
    unsigned char humidiyz = 0;
    unsigned char humidiyx = 0;
    FILE *file;

    humidityfd = open(DEVICE_BLTEST, O_RDONLY);
    if(humidityfd < 0)
    {
        perror("can not open device");
        exit(1);
    }

    while(1)
    {
        ret = read(humidityfd, buf, sizeof(buf));
        if(ret < 0)
        {
            printf("read err!\n");
        }
        else
        {
            humidiyz = buf[0];
            humidiyx = buf[1];
            tempz = buf[2];
            tempx = buf[3];

            printf("humidity = %d.%d%%\n", humidiyz, humidiyx);
            printf("temperature = %d.%d\n", tempz, tempx);

            // 將溫度值寫入文件
            file = fopen(TEMP_FILE, "w");
            if (file != NULL)
            {
                fprintf(file, "%d.%d\n", tempz, tempx);
                fclose(file);
            }
            else
            {
                perror("Error opening temperature file");
            }
        }
        sleep(2); // 每2秒更新一次
    }

    if (humidityfd >= 0)     // close humidityfd if open
    {
        close(humidityfd);
    }

    return 0;
}

