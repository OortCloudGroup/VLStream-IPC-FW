//#include "libgeneral.h"
#include "../libpthread/libpthread.h"
#include "t_record.h"
#include "fbmap.h"

#define RECORD_THREAD_ALIVE_THRESHOLD				(10 * 60 * 100)
#define RECORD_THREAD_PRIORITY						3

THREAD_CONTEXT g_record_thread_context;
static void * record_thread_function(void * arg);

bool t_create_record_thread()
{
	bool ret;
	g_record_thread_context.alive_threshold = RECORD_THREAD_ALIVE_THRESHOLD;
	g_record_thread_context.handler = record_thread_function;
	g_record_thread_context.priority = RECORD_THREAD_PRIORITY;
	if (! (ret = create_thread(&g_record_thread_context)))
		printf("%s: failed\n", __func__);
	
	return ret;
}

static int fd = 0;
static char *file = "outPath.mp4"; //outPath
static char *device = "/dev/video0";
static char *format = "H264"; //example JPEG,H264,MJPG,BGR3...
static int second = 30;
static int width = 1080;
static int height = 720;
static int shoeInfo = 1;

static uint32_t timeStamp = 0;
static uint32_t frameCount = 0;

static CameraMap_Struct *cms;

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

int c_init_cap()
{
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
        return 0;
    }

    return 1;
}


void c_do_record_handler(){
	int index=0;
	// 视频流模式
	if (second)
	{
		// 等待开始
		while(!frameCount)
			usleep(100);
		// 开始计时
		for (index = 0; index < second; index++)
			sleep(1);
	}
	// 拍照模式
	else
	{
		// 等待开始
		while(!frameCount)
			usleep(100);
	}

	printf("record_thread_function::\r\n");
}

void c_close_cap()
{
	cameraMap_close(cms);
    if (fd > 0)
        close(fd);
}

static void * record_thread_function(void * arg)
{
	THREAD_CONTEXT * c = (THREAD_CONTEXT *)arg;
    
	if (! c_init_cap())
	{
		printf("%s: reboot\n", __func__);
		//c_reboot();
		return NULL;
	}

	while (! c->exit)//
	{
		c_do_record_handler();
		break;
	}
	
	c_close_cap();
	printf("record quited\n");
	c->quited = true;
	
	return NULL;
}