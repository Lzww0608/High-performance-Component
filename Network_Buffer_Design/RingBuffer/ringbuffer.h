#ifndef _RING_BUFFER_H
#define _RING_BUFFER_H

#include <stdint.h>

/*
* 用户接口
*/

//不展示buffer结构体定义
typedef struct ringbuffer_s buffer_t;
//初始化buffer
buffer_t *buffer_new(uint32_t sz);
//获取buffer长度
uint32_t buffer_len(buffer_t *r);
//清空buffer
void buffer_free(buffer_t *r);
//向buffer中添加数据，sz为数据长度，返回0表示成功
int buffer_add(buffer_t *r, const void *data, uint32_t sz);
//移除buffer中的数据,返回sz表示成功
int buffer_remove(buffer_t *r, void *data, uint32_t sz);
//清除buffer中sz长度的数据，返回sz表示清除的长度
int buffer_delete(buffer_t *r, uint32_t sz);
//搜索buffer中的特殊字符串，可用于界定数据包（粘包问题）,返回起始地址
int buffer_search(buffer_t *r, const char* data, const int len);
//重新分配空间是的buffer中数据在物理上连续,返回队列头指针
uint8_t *buffer_write_atmost(buffer_t *r);

#endif // !_RING_BUFFER_H
#define _RING_BUFFER_H
