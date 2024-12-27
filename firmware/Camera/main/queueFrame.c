#include "queueFrame.h"
#include <pthread.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>

// 初始化队列
void init_queue(FrameQueue *q, int capacity) {
    q->capacity = capacity;
    q->queue = (Frame *)malloc(sizeof(Frame) * capacity); // 分配内存
    q->front = 0;
    q->rear = 0;
    q->count = 0;
    pthread_mutex_init(&q->mutex, NULL);
    pthread_cond_init(&q->not_empty, NULL);
    pthread_cond_init(&q->not_full, NULL);
}

// 入队操作
void enqueue(FrameQueue *q, uint8_t *data, size_t size) {
    pthread_mutex_lock(&q->mutex);

    // 如果队列已满，则等待
    while (q->count == q->capacity) {
        pthread_cond_wait(&q->not_full, &q->mutex);
    }

    // 拷贝数据到队列
    q->queue[q->rear].data = (uint8_t *)malloc(size);
    memcpy(q->queue[q->rear].data, data, size);
    q->queue[q->rear].size = size;
    q->rear = (q->rear + 1) % q->capacity;
    q->count++;

    // 通知其他线程队列非空
    pthread_cond_signal(&q->not_empty);

    pthread_mutex_unlock(&q->mutex);
}

// 出队操作
Frame dequeue(FrameQueue *q) {
    pthread_mutex_lock(&q->mutex);

    // 如果队列为空，则等待
    while (q->count == 0) {
        pthread_cond_wait(&q->not_empty, &q->mutex);
    }

    Frame frame = q->queue[q->front];
    q->front = (q->front + 1) % q->capacity;
    q->count--;

    // 通知其他线程队列非满
    pthread_cond_signal(&q->not_full);

    pthread_mutex_unlock(&q->mutex);

    return frame;
}

// 释放帧数据内存
void free_frame(Frame *frame) {
    free(frame->data);
}

// 释放队列内存
void free_queue(FrameQueue *q) {
    for (int i = 0; i < q->count; i++) {
        free_frame(&q->queue[i]);
    }
    free(q->queue);
    pthread_mutex_destroy(&q->mutex);
    pthread_cond_destroy(&q->not_empty);
    pthread_cond_destroy(&q->not_full);
}
