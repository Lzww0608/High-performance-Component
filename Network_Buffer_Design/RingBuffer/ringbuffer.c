#include "ringbuffer.h"
#include <stdint.h>
#include <stddef.h>
#include <assert.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>



struct ringbuffer_s {
    uint32_t size;
    uint32_t tail;
    uint32_t head;
    uint8_t *buf;
};

#define min(l, r) ((l) < (r) ? (l) : (r))

static inline int is_pow_of_two(uint32_t x) {
    if (x < 2) return 0;
    return (x & (x - 1)) == 0;
}

static inline uint32_t roundup_pow_of_two(uint32_t x) {
    if (x == 0) {
        return 2;
    }
    int i = 0;
    for (; x != 0; i++) {
        x >>= 1;
    }
    return 1u << 1;
}

buffer_t *buffer_new(uint32_t sz) {
    //缓冲区长度为2的幂
    if (!is_pow_of_two(sz)) {
        sz = roundup_pow_of_two(sz);
    }
    buffer_t *buf = (buffer_t*)malloc(sizeof(buffer_t) + sz);
    buf->size = sz;
    buf->head = buf->tail = 0;
    buf->buf = (uint8_t*)(buf + 1);
    return buf;
}

uint32_t buffer_len(buffer_t *r) {
    return r->tail - r->head;
}

void buffer_free(buffer_t *r) {
    free(r);
    r = NULL;
}

static uint32_t buf_remain(buffer_t *r) {
    return r->size - buffer_len(r);
}

int buffer_add(buffer_t *r, const void *data, uint32_t sz) {
    if (sz > buf_remain(r)) {
        return -1; //剩余空间不足
    }
    //计算r->tail到缓冲区尾部大小，剩余部分从缓冲区头部添加
    uint32_t d = min(sz, r->size - (r->tail & (r->size - 1)));

    memcpy(r->buf + (r->tail & (r->size - 1)), data, d);
    memcpy(r->buf, data + d, sz - d);

    r->tail += sz;
    return 0;
}

static uint32_t buf_isEmpty(buffer_t *r) {
    return r->head == r->tail;
}

int buffer_remove(buffer_t *r, void *data, uint32_t sz) {
    assert(!buf_isEmpty(r));

    sz = min(sz, buffer_len(r));

    uint32_t d = min(sz, r->size - (r->tail & (r->size - 1)));
    memcpy(data, r->buf + (r->head & (r->size - 1)), d);
    memcpy(data + d, r->buf, sz - d);

    r->head += sz;
    return sz;
}

int buffer_delete(buffer_t *r, uint32_t sz) {
    if (sz > buffer_len(r)) {
        sz = buffer_len(r);
    }

    r->head += sz;
    return sz;
}

int buffer_search(buffer_t *r, const char* data, const int len) {
    int i;
    for (i = 0; i <= buffer_len(r) - len; i++) {
        //检索界定数据包起始位置
        int pos = (r->head + i) & (r->size - 1);
        //字符串的存储跨过了缓冲区尾部
        if (pos + len > r->size) {
            if (memcmp(r->buf + pos, data, r->size - pos)) {
                return 0;
            }
            if (memcmp(r->buf, data + r->size - pos, pos + len - r->size) == 0) {
                return i + len;
            }
        }
        if (memcmp(r->buf + pos, data, len) == 0) {
            return i + len;
        }
    }
    return 0;
}

uint8_t *buffer_write_atomost(buffer_t *r) {
    uint32_t hpos = r->head & (r->size - 1);
    uint32_t tpos = r->tail & (r->size - 1);

    if (tpos < hpos) {
        uint8_t *tmp = (uint8_t*)malloc(r->size * sizeof(uint8_t));
        memcpy(tmp, r->buf + hpos, r->size - hpos);
        memcpy(tmp + r->size - hpos, r->buf, tpos);
        free(r->buf);
        r->buf = tmp;
        return r->buf;
    }

    return r->buf + hpos;
}

