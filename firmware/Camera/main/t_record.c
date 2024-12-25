//#include "libgeneral.h"
#include "../libpthread/libpthread.h"
#include "../libutility/libutility.h"

#include <linux/videodev2.h>
#include <errno.h>
#include <sys/times.h>
#include <assert.h>

#include "../include/x264.h"
#include "../include/x264_config.h"
#include "t_record.h"
#include "fbmap.h"
#include "h264encoder.h"


extern FrameQueue g_queue;  // 引用全局队列

#define RECORD_THREAD_ALIVE_THRESHOLD				(10 * 60 * 100)
#define RECORD_THREAD_PRIORITY						3

THREAD_CONTEXT g_record_thread_context;
static void * record_thread_function(void * arg);

bool t_create_record_thread(FrameQueue* queue)
{
	bool ret;
	g_record_thread_context.alive_threshold = RECORD_THREAD_ALIVE_THRESHOLD;
	g_record_thread_context.handler = record_thread_function;
	g_record_thread_context.priority = RECORD_THREAD_PRIORITY;
    g_record_thread_context.queue = queue;
	if (! (ret = create_thread(&g_record_thread_context)))
		printf("%s: failed\n", __func__);
	
	return ret;
}


#define ERROR -1
// #define JPG "./out/image%d"
#define JPG "./out/image%d.jpg"
#define WIDTH 640
#define HIGHT 480
//#define COUNT 2000

typedef struct
{
    void *start;
    int length;
} BUFTYPE;
BUFTYPE *usr_buf;
static unsigned int n_buffer = 0;

//h264
Encoder en;
FILE *h264_fp;
char h264_file_name[20] = "./test.h264";
char *h264_buf;

void init_encoder(int width, int height)
{
    compress_begin(&en, width, height);
    h264_buf = (char *)malloc(width * height * 2);
}

void init_file()
{
    h264_fp = fopen(h264_file_name, "wa+");
}

void close_encoder()
{
    compress_end(&en);
    free(h264_buf);
}

void close_file()
{
    fclose(h264_fp);
}

void encode_frame(uint8_t *yuv_frame, size_t yuv_length)
{
    int h264_length = 0;
    static int count = 0;
    
    // clock_t start, end;
    // double cpu_time_used;

    // // 获取程序开始执行的时刻
    // start = clock();

    h264_length = compress_frame(&en, -1, yuv_frame, h264_buf);
    //     // 获取程序执行完的时刻
    // end = clock();

    // 计算程序运行的时间（秒）
    // cpu_time_used = ((double)(end - start)) / CLOCKS_PER_SEC;

    // printf("程序执行时间: %f 秒\n", cpu_time_used);

    if (h264_length > 0)
    {
        // 直接通过上下文获取队列
        enqueue(&g_queue, h264_buf, h264_length);
        // if (fwrite(h264_buf, h264_length, 1, h264_fp) > 0)
        // {
        //     // 直接通过上下文获取队列
        //     enqueue(&g_queue, h264_buf, h264_length);
        //     printf("encode_frame num = %d\n",count++);
           
        // }
        // else
        // {
        //     perror("encode_frame fwrite err\n");
        // }
    }
}

int open_camera(char *Directory)
{
    int fd;
    struct v4l2_input inp;
    //打开摄像头设备，非阻塞方式
    fd = open(Directory, O_RDWR | O_NONBLOCK, 0);
    if(fd < 0)
    {
        // DEBUG_PRINT(DEBUG_ERROR, "open camera failed: %s\n", strerror(errno));
        return ERROR;
    }
    inp.index = 0;
    if (ERROR == ioctl(fd, VIDIOC_S_INPUT, &inp))
    {
        // DEBUG_PRINT(DEBUG_ERROR, "ioctl VIDIOC_S_INPUT  failed: %s\n", strerror(errno));
    }
    return fd;
}

/*set video capture ways(mmap)*/
int init_mmap(int fd)
{
    /*to request frame cache, contain requested counts*/
    struct v4l2_requestbuffers reqbufs;
    memset(&reqbufs, 0, sizeof(reqbufs));
    reqbufs.count = 4; /*the number of buffer*/     //缓冲区个数
    reqbufs.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    reqbufs.memory = V4L2_MEMORY_MMAP;
    if (-1 == ioctl(fd, VIDIOC_REQBUFS, &reqbufs))  //申请缓冲区
    {
        // DEBUG_PRINT(DEBUG_ERROR, "ioctl 'VIDIOC_REQBUFS'failed: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }
    n_buffer = reqbufs.count;
    printf("n_buffer = %d\n", n_buffer);
    usr_buf = calloc(reqbufs.count, sizeof(BUFTYPE)); //申请地址user_buf(void *start;int length;共8byte)
    if (usr_buf == NULL)
    {
        // DEBUG_PRINT(DEBUG_ERROR, "Out of memory failed: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }
    /*map kernel cache to user process*/
    for (n_buffer = 0; n_buffer < reqbufs.count; ++n_buffer)
    {
        //stand for a frame
        struct v4l2_buffer buf;      //定义一帧数据buf
        memset(&buf, 0, sizeof(buf));
        buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        buf.memory = V4L2_MEMORY_MMAP;
        buf.index = n_buffer; 
        /*check the information of the kernel cache requested*/
        if (-1 == ioctl(fd, VIDIOC_QUERYBUF, &buf))    //获取数据帧地址和长度
        {
            // DEBUG_PRINT(DEBUG_ERROR, "Fail to ioctl : VIDIOC_QUERYBUF %s\n", strerror(errno));
            exit(EXIT_FAILURE);
        }
        usr_buf[n_buffer].length = buf.length;  //数据帧的长度
        printf("usr_buf[n_buffer].length :%d\n", usr_buf[n_buffer].length);  //打印测试
        usr_buf[n_buffer].start = (char *)mmap(NULL, buf.length, PROT_READ | PROT_WRITE, MAP_SHARED, fd, buf.m.offset); //进程与设备数据帧地址关联起来
        //之后就可以可以用进程的数据地址访问设备文件的数据
        if (MAP_FAILED == usr_buf[n_buffer].start)
        {
            // DEBUG_PRINT(DEBUG_ERROR, "Fail to mmap:%s\n", strerror(errno));
            exit(EXIT_FAILURE);
        }
    }
    return 0;
}

int init_camera(int fd)
{
    struct v4l2_capability cap;  /* decive fuction, such as video input */
    struct v4l2_format tv_fmt;   /* frame format */
    struct v4l2_fmtdesc fmtdesc; /* detail control value */
    int ret;
    /*show all the support format*/
    memset(&fmtdesc, 0, sizeof(fmtdesc));
    /* the number to check */
    fmtdesc.index = 0;
    fmtdesc.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    /* check video decive driver capability */
    if (ret = ioctl(fd, VIDIOC_QUERYCAP, &cap) < 0)  //获取设备信息
    {
        // DEBUG_PRINT(DEBUG_ERROR, "ioctl VIDEO_QUERYCAP failed: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }
    /*judge wherher or not to be a video-get device*/
    if (!(cap.capabilities & V4L2_BUF_TYPE_VIDEO_CAPTURE))
    {
        // DEBUG_PRINT(DEBUG_ERROR, "The Current device is not a video capture device %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }
    /*judge whether or not to supply the form of video stream*/
    if (!(cap.capabilities & V4L2_CAP_STREAMING))
    {
        // DEBUG_PRINT(DEBUG_ERROR, "The Current device is not a video capture device %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }
    printf("\ncamera driver name is : %s\n", cap.driver);
    printf("camera device name is : %s\n", cap.card);
    printf("camera bus information: %s\n", cap.bus_info);
    /*display the format device support*/
    while (ioctl(fd, VIDIOC_ENUM_FMT, &fmtdesc) != -1)  //获取设备支持的所有格式
    {
        printf("support device %d.%s\n", fmtdesc.index + 1, fmtdesc.description);
        fmtdesc.index++;
    }
    printf("\n");
    /*set the form of camera capture data*/
    tv_fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE; /*v4l2_buf_typea,camera must use V4L2_BUF_TYPE_VIDEO_CAPTURE*/
    tv_fmt.fmt.pix.width = 640;
    tv_fmt.fmt.pix.height = 480;
    tv_fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_YUYV; /*V4L2_PIX_FMT_YYUV*/ //V4L2_PIX_FMT_YUYV //V4L2_PIX_FMT_MJPEG
    tv_fmt.fmt.pix.field = V4L2_FIELD_NONE;           /*V4L2_FIELD_NONE*/
    if (ioctl(fd, VIDIOC_S_FMT, &tv_fmt) < 0)         //设置设备工作格式
    {
        // DEBUG_PRINT(DEBUG_ERROR, "VIDIOC_S_FMT set err %s\n", strerror(errno));
        close(fd);
        exit(-1);
    }
    init_mmap(fd); //重点和难点（内存映射）
    init_encoder(WIDTH, HIGHT);  //初始化编码器
    init_file();                 //初始化输出文件
    return 0;
}

int start_capture(int fd) 
{
    unsigned int i;
    enum v4l2_buf_type type;
    /*place the kernel cache to a queue*/
    for (i = 0; i < n_buffer; i++)
    {
        struct v4l2_buffer buf;
        memset(&buf, 0, sizeof(buf));
        buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        buf.memory = V4L2_MEMORY_MMAP;
        buf.index = i;
        if (-1 == ioctl(fd, VIDIOC_QBUF, &buf))
        {
            // DEBUG_PRINT(DEBUG_ERROR, "Fail to ioctl 'VIDIOC_QBUF' %s\n", strerror(errno));
            exit(EXIT_FAILURE);
        }
    }
    type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    if (-1 == ioctl(fd, VIDIOC_STREAMON, &type))  //启动采集数据
    {
        printf("i=%d.\n", i);
        // DEBUG_PRINT(DEBUG_ERROR, "VIDIOC_STREAMON %s\n", strerror(errno));
        close(fd);
        exit(EXIT_FAILURE);
    }
    return 0;
}

int process_image(void *addr, int length)
{
    FILE *fp;
    static int num = 0;
    char image_name[20];
    sprintf(image_name, JPG, num++);
    if ((fp = fopen(image_name, "w")) == NULL)
    {
        perror("Fail to fopen");
        exit(EXIT_FAILURE);
    }
    fwrite(addr, length, 1, fp);
    usleep(500);
    fclose(fp);
    return 0;
}

int read_frame(int fd)
{
    struct v4l2_buffer buf;  //定义一帧数据buf
    memset(&buf, 0, sizeof(buf));
    buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    buf.memory = V4L2_MEMORY_MMAP;
    if (-1 == ioctl(fd, VIDIOC_DQBUF, &buf))   //从缓冲区取出一帧数据
    {
        perror("Fail to ioctl 'VIDIOC_DQBUF'");
        exit(EXIT_FAILURE);
    }
    assert(buf.index < n_buffer);
    //read process space's data to a file
    // process_image(usr_buf[buf.index].start, usr_buf[buf.index].length);
    encode_frame(usr_buf[buf.index].start, usr_buf[buf.index].length);
    if (-1 == ioctl(fd, VIDIOC_QBUF, &buf))
    {
        perror("Fail to ioctl 'VIDIOC_QBUF'");
        exit(EXIT_FAILURE);
    }
    return 1;
}

// int mainloop(int fd)
// {
//     int count = COUNT;
//     while (count-- > 0)
//     {
//         while(1)
//         {
//             fd_set fds;
//             struct timeval tv;
//             int r;
//             FD_ZERO(&fds);
//             FD_SET(fd, &fds);
//             /*Timeout*/
//             tv.tv_sec = 2;
//             tv.tv_usec = 0;
//             r = select(fd + 1, &fds, NULL, NULL, &tv);  //等到一帧数据到来
//             if (-1 == r)
//             {
//                 if (EINTR == errno)
//                     continue;
//                 // DEBUG_PRINT(DEBUG_ERROR, "Fail to select: %s\n", strerror(errno));
//                 exit(EXIT_FAILURE);
//             }
//             if (0 == r) //2秒都没有一帧数据-->超时处理
//             {
//                 // DEBUG_PRINT(DEBUG_INFO, "select Timeout!\n");
//                 exit(-1);
//             }
//             if (read_frame(fd))   //读取一帧数据并生成一张图片
//                 break;
//         }
//     }
//     return 0;
// }

void stop_capture(int fd)
{
    enum v4l2_buf_type type;
    type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    if (-1 == ioctl(fd, VIDIOC_STREAMOFF, &type))
    {
        perror("Fail to ioctl 'VIDIOC_STREAMOFF'");
        exit(EXIT_FAILURE);
    }
}

void close_camera_device(int fd)
{
    unsigned int i;
    close_encoder();
    close_file();
    for (i = 0; i < n_buffer; i++)
    {
        if (-1 == munmap(usr_buf[i].start, usr_buf[i].length))
        {
            exit(-1);
        }
    }

    free(usr_buf);

    if (-1 == close(fd))
    {
        perror("Fail to close fd");
        exit(EXIT_FAILURE);
    }
}

static void * record_thread_function(void * arg)
{
	THREAD_CONTEXT * c = (THREAD_CONTEXT *)arg;
    int fd;
    fd = open_camera("/dev/video0");

	if (fd == -1)
	{
		printf("%s: reboot\n", __func__);
		//c_reboot();
		return NULL;
	}
    //设置摄像头参数
    if(init_camera(fd) != 0)
    {
        printf("%s: reboot\n", __func__);
		//c_reboot();
		return NULL;
    };
    //启动摄像头数据流
    if(start_capture(fd) != 0)
    {
        printf("%s: reboot\n", __func__);
		//c_reboot();
		return NULL;
    };
    //读取数据
	// while (! c->exit)//
	// {
    //     c->tick = get_tickcount();

	// }
    while (! c->exit)
    {
        c->tick = get_tickcount();
        while(1)
        {
            fd_set fds;
            struct timeval tv;
            int r;
            FD_ZERO(&fds);
            FD_SET(fd, &fds);
            /*Timeout*/
            tv.tv_sec = 2;
            tv.tv_usec = 0;
            r = select(fd + 1, &fds, NULL, NULL, &tv);  //等到一帧数据到来
            if (-1 == r)
            {
                if (EINTR == errno)
                    continue;
                // DEBUG_PRINT(DEBUG_ERROR, "Fail to select: %s\n", strerror(errno));
                printf("Fail to select: %s\n");
                exit(EXIT_FAILURE);
            }
            if (0 == r) //2秒都没有一帧数据-->超时处理
            {
                // DEBUG_PRINT(DEBUG_INFO, "select Timeout!\n");
                printf("select Timeout!\n");
                exit(-1);
            }
            if (read_frame(fd))   //读取一帧数据并生成一张图片
                break;
        }
    }
	
	//关闭摄像头数据流
    stop_capture(fd);
    close_camera_device(fd);
	printf("record quited\n");
	c->quited = true;
	
	return NULL;
}