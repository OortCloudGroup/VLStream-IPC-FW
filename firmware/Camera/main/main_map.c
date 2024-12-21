#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>

#include "fbmap.h"

void help(char *own)
{
    printf(
        "\r\n"
        "Usage: %s [option] [save file]\r\n"
        "\r\n"
        "Option: \r\n"
        "    -d device : example /dev/video0 (default)\r\n"
        "    -f format : example JPEG,H264,MJPG,BGR3...\r\n"
        "    -t second : recording time, skip this in photo mode\r\n"
        "    -p WidthxHeight : example 1920x1080\r\n"
        "    -h : hide frame info\r\n"
        "\r\n"
        "Example: \r\n"
        "    %s -d /dev/video0 \r\n"
        "\r\n",
        own, own);

    exit(1);
}

static int fd = 0;
static char *file = NULL;
static char *device = "/dev/video0";
static char *format = NULL;
static int second = 0;
static int width = 640;
static int height = 480;
static int shoeInfo = 1;

static uint32_t timeStamp = 0;
static uint32_t frameCount = 0;

void getParam(int argc, char **argv)
{
    int i, len;
    char *p;

    if (argc < 2) {
        help(argv[0]);
    }

    for (i = 1; i < argc; i++)
    {
        p = argv[i];
        len = strlen(p);

        if (len == 2 && strstr(p, "-d") && i + 1 < argc)
        {
            device = argv[++i];
        }
        else if (len == 2 && strstr(p, "-f") && i + 1 < argc)
        {
            if (strlen(argv[++i]) < 4)
            {
                fprintf(stderr, "param error: -f format \r\n");
                help(argv[0]);
            }
            format = argv[i];
        }
        else if (len == 2 && strstr(p, "-t") && i + 1 < argc)
        {
            second = atoi(argv[++i]);
            if (second < 1)
                second = 1;
        }
        else if (len == 2 && strstr(p, "-p") && i + 1 < argc)
        {
            if (sscanf(argv[++i], "%dx%d", &width, &height) < 2)
            {
                fprintf(stderr, "param error: -p WidthxHeight \r\n");
                help(argv[0]);
            }
        }
        else if (len == 2 && strstr(p, "-h"))
            shoeInfo = 0;
        else if (strstr(p, "-?") || strstr(p, "help"))
            help(argv[0]);
        else
            file = p;
    }

    printf("dev:%s, format:%4s %dx%d, second:%d, file:%s\r\n",
           device, format, width, height, second, file);
}

void camera_callback(CameraMap_Struct *cms, uint8_t *mem, int memSize)
{
    if (shoeInfo)
    {
        printf("frame: %d len %d delay %dms -- "
               "%02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X\r\n",
               frameCount, memSize,
               cms->timeStamp - timeStamp,
               mem[0], mem[1], mem[2], mem[3],
               mem[4], mem[5], mem[6], mem[7],
               mem[8], mem[9], mem[10], mem[11]);
        timeStamp = cms->timeStamp;
    }

    // 写文件
    if (fd > 0)
    {
        // 拍照模式
        if (second == 0)
        {
            write(fd, mem, memSize);
            close(fd);
            fd = 0;
        }
        else
        {
            write(fd, mem, memSize);
            fsync(fd);
        }
    }

    frameCount++;
}

int main(int argc, char **argv)
{
    int i;
    CameraMap_Struct *cms;

    getParam(argc, argv);

    if (file)
    {
        if ((fd = open(file, O_RDWR | O_CREAT | O_TRUNC, 0666)) < 1)
            fprintf(stderr, "open %s failed \r\n", file);
    }

    if (!(cms = cameraMap_open(device, format, width, height, NULL, &camera_callback)))
    {
        fprintf(stderr, "cameraMap_open failed \r\n");
        if (fd > 0)
            close(fd);
        return -1;
    }

    // 视频流模式
    if (second)
    {
        // 等待开始
        while(!frameCount)
            usleep(100);
        // 开始计时
        for (i = 0; i < second; i++)
            sleep(1);
    }
    // 拍照模式
    else
    {
        // 等待开始
        while(!frameCount)
            usleep(100);
    }

    cameraMap_close(cms);

    if (fd > 0)
        close(fd);

    return 0;
}
