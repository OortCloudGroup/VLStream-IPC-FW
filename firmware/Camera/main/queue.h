#ifndef QUEUE_H
#define QUEUE_H

#include <stdlib.h>
#include <stdint.h>

// 定义帧结构体，包含数据指针和数据大小
typedef struct {
    uint8_t *data;
    size_t size;
} Frame;

// 定义一个队列结构体，包含队列数组、队列的前后指针、计数器以及同步机制
typedef struct {
    Frame *queue;             // 动态分配的队列数组
    int front;                // 队列前端
    int rear;                 // 队列尾端
    int count;                // 当前队列中的元素数目
    int capacity;             // 队列的最大容量
    pthread_mutex_t mutex;    // 队列操作的互斥锁
    pthread_cond_t not_empty; // 队列为空时的条件变量
    pthread_cond_t not_full;  // 队列已满时的条件变量
} FrameQueue;

// 队列相关函数声明
void init_queue(FrameQueue *q, int capacity);             // 初始化队列，指定容量
void enqueue(FrameQueue *q, uint8_t *data, size_t size);  // 入队操作
Frame dequeue(FrameQueue *q);                             // 出队操作
void free_frame(Frame *frame);                            // 释放帧内存
void free_queue(FrameQueue *q);                           // 释放队列内存

#endif // QUEUE_H
