#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include<sys/ioctl.h>

#define IOCTL_CMD_WRITE 0x00
#define IOCTL_CMD_CLEAR 0x01

int main()
{
    int value;
    int fd;
    char *message = "Hello\nworld!";
    fd = open("/dev/i2c_device", O_RDWR);
        if(fd < 0) {
                printf("Cannot open device file...\n");
                return 0;
        }

     if (ioctl(fd, IOCTL_CMD_CLEAR, 0) == -1) {
        perror("ioctl failed");
        close(fd);
        return -1;
    } 

    if (ioctl(fd, IOCTL_CMD_WRITE, message) == -1) {
        perror("ioctl failed");
        close(fd);
        return -1;
    }  

    close(fd);
    return 0;     
}