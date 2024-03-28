#ifndef _CHAIN_BUFFER_H
#define _CHAIN_BUFFER_H

#include <stdint.h>

typedef struct buf_chain_s buf_chain_t;
typedef struct buffer_s buffer_t;

buffer_t *buffer_new(uint32_t sz);

uint32_t buffer_len(buffer_t *c);

int buffer_add(buffer_t *c, const void *data, uint32_t len);

int buffer_remove(buffer_t *c, void *data, uint32_t len);
//
int buffer_delete(buffer_t *c, uint32_t len);

void buffer_free(buffer_t *c);
//寻找特定字符串（界定字符串）
int buffer_search(buffer_t *buf, const char* data, const int len);

uint8_t *buffer_write_atmost(buffer_t *c);



#endif