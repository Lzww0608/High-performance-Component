
#include "chainbuffer.h"
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>

struct buf_chain_s {
    struct buf_chain_s *next;
    uint32_t buffer_len;
    uint32_t misalign;
    uint32_t offset;
    uint8_t *buffer;
};

struct buffer_s {
    buf_chain_t *head;
    buf_chain_t *tail;
    buf_chain_t **last_with_data;
    uint32_t total_len;
    uint32_t last_read_pos;
};

#define CHAIN_SPACE_LEN(ch) ((ch)->buffer_len - ((ch)->misalign + (ch)->off))
#define MIN_BUFFER_SIZE 1024
#define MAX_TO_COPY_IN_EXPAND 4096
#define BUFFER_CHAIN_MAX_AUTO_SIZE 4096
#define MAX_TO_REALIGN_IN_EXPAND 2048
#define BUFFER_CHAIN_MAX 16 * 1024 * 1024 //16M
#define BUFFER_CHAIN_EXTRA(t, c) (t *)((buf_chain_t *)(c) + 1)
#define BUFFER_CHAIN_SIZE sizeof(buf_chain_t)

uint32_t buffer_len(buffer_t *buf) {
    return buf->total_len;
}

buffer_t *buffer_new(uint32_t sz) {
    (void)sz; //消除警告，不使用
    buffer_t *buf = (buffer_t *)malloc(sizeof(buffer_t));
    if (!buf) {
        return NULL;
    }
    memset(buf, 0, sizeof(*buf));
    return buf;
}

static buf_chain_t *buf_chain_new(uint32_t sz) {
    buf_chain_t *chain;
    uint32_t to_alloc;
    if (sz > BUFFER_CHAIN_MAX - BUFFER_CHAIN_SIZE) {
        return (NULL);
    }
    sz += BUFFER_CHAIN_SIZE;

    if (sz < BUFFER_CHAIN_MAX / 2) {
        to_alloc = MIN_BUFFER_SIZE;
        while (to_alloc < sz) {
            to_alloc <<= 1;
        }
    } else {
        to_alloc = sz;
    }
    if ((chain = malloc(to_alloc)) == NULL) {
        return (NULL);
    }
    memset(chain, 0, BUFFER_CHAIN_SIZE);
    chain->buffer_len = to_alloc - BUFFER_CHAIN_SIZE;
    chain->buffer = BUFFER_CHAIN_EXTRA(uint8_t, chain);
    return (chain);
}

static void buf_chain_free_all(buf_chain_t *chain) {
    buf_chain_t *next;
    for (; chain; chain = next) {
        next = chain->next;
        free(chain);
    }
}

void buffer_free(buffer_t *buf) {
    buf_chain_free_all(buf->head);
}
//释放空的链表
static buf_chain_t **free_empty_chains(buffer_t *buf) {
    buf_chain_t **ch = buf->last_with_data;
    while ((*ch) && (*ch)->offset != 0) {
        ch = &(*ch)->next;
    }
    if (*ch) {
        buf_chain_free_all(*ch);
        *ch = NULL;
    }
    return ch;
}
//插入链表
static void buf_chain_insert(buffer_t *buf, buf_chain_t *chain) {
    if (*buf->last_with_data == NULL) {
        buf->head = buf->tail = chain;
    } else {
        buf_chain_t **ch;
        ch = free_empty_chains(buf);
        *ch = chain;
        if (chain->offset) 
            buf->last_with_data = ch;
        buf->tail = chain;
    }

    buf->total_len += chain->offset;
}

static inline buf_chain_t *buf_chain_insert_new(buffer_t *buf, uint32_t len) {
    buf_chain_t *chain;
    if ((chain = buf_chain_new(len)) == NULL) {
        return NULL;
    }
    buf_chain_insert(buf, chain);
    return chain;
}

static int buf_chain_should_realign(buf_chain_t *chain, uint32_t len) {
    return chain->buffer_len - chain->offset >= len && (chain->offset < chain->buffer_len / 2) && (chain->offset <= MAX_TO_REALIGN_IN_EXPAND);
}

static void buf_chain_align(buf_chain_t *chain) {
    memmove(chain->buffer, chain->buffer + chain->misalign, chain->offset);
    chain->misalign = 0;
}

int buffer_add(buffer_t *buf, const void *data_in, uint32_t len) {
    buf_chain_t *chain, *tmp;
    const uint8_t *data = data_in;
    uint32_t remain, to_alloc;
    int res = -1;

    if (len > BUFFER_CHAIN_MAX - buf->total_len) {
        return res;
    }

    if (*buf->last_with_data == NULL) {
        chain = buf->tail;
    } else {
        chain = *buf->last_with_data;
    }

    if (chain == NULL) {
        chain = buf_chain_insert_new(buf, len);
        if (!chain) {
            return res;
        }
    }

    remain = chain->buffer_len - chain->misalign - chain->offset;
    if (remain >= len) {
        memcpy(chain->buffer + chain->misalign + chain->offset, data, len);
        chain->offset += len;
        buf->total_len += len;
        return 0;
    } else if (buf_chain_should_realign(chain, len)) {
        buf_chain_align(chain);
        memcpy(chain->buffer + chain->offset, data, len);
        buf->total_len += len;
        return 0;
    }

    to_alloc = chain->buffer_len;
    if (to_alloc <= BUFFER_CHAIN_MAX_AUTO_SIZE / 2) {
        to_alloc <<= 1;
    }
    if (len > to_alloc) {
        to_alloc = len;
    }
    tmp = buf_chain_new(to_alloc);
    if (tmp == NULL) {
        return res;
    }
    if (remain) {
        memcpy(chain->buffer + chain->misalign + chain->offset, data, remain);
        chain->offset += remain;
        buf->total_len += remain;
    }

    data += remain;
    len -= remain;

    memcpy(tmp->buffer, data, len);
    tmp->offset = len;
    buf_chain_insert(buf, tmp);

    return res;
}

static uint32_t buf_copyout(buffer_t *buf, void *data_out, uint32_t len) {
    buf_chain_t *chain;
    char *data = data_out;
    uint32_t nread;
    chain = buf->head;
    if (len > buf->total_len) {
        len = buf->total_len;
    }
    if (len == 0) {
        return 0;
    }
    nread = len;

    while (len && len >= chain->offset) {
        uint32_t copylen = chain->offset;
        memcpy(data, chain->buffer + chain->misalign, copylen);
        data += copylen;
        chain = chain->next;
    }

    if (len) {
        memcpy(data, chain->buffer + chain->misalign, len);
    }

    return nread;
}

static inline void ZERO_CHAIN(buffer_t *dst) {
    dst->head = NULL;
    dst->tail = NULL;
    dst->last_with_data = &(dst)->head;
    dst->total_len = 0;
}

int buffer_delete(buffer_t *buf, uint32_t len) {
    buf_chain_t *chain, *next;
    uint32_t remaining, old_len;
    old_len = buf->total_len;
    if (old_len == 0) {
        return 0;
    }

    if (len >= old_len) {
        len = old_len;
        for (chain = buf->head; chain != NULL; chain = next) {
            next = chain->next;
            free(chain);
        }
        ZERO_CHAIN(buf);
    } else {
        buf->total_len -= len;
        remaining = len;
        for (chain = buf->head; remaining >= chain->offset; chain = next) {
            next = chain->next;
            remaining -= chain->offset;

            if (chain == *buf->last_with_data) {
                buf->last_with_data = &buf->head;
            }
            if (&chain->next == buf->last_with_data) {
                buf->last_with_data = &buf->head;
            }
            free(chain);
        }

        buf->head = chain;
        chain->misalign += remaining;
        chain->offset -= remaining;
    }
    return len;
}

int buffer_remove(buffer_t *buf, void *data_out, uint32_t len) {
    uint32_t n = buf_copyout(buf, data_out, len);
    if (n > 0) {
        if (buffer_delete(buf, n) < 0) {
            n = -1;
        }
    }
    return (int)n;
}

static bool check_sep(buf_chain_t *chain, int from, const char* sep, int len) {
    while (true) {
        int sz = chain->offset - from;
        if (sz >= len) {
            return memcmp(chain->buffer + chain->misalign + from, sep, len) == 0;
        }
        if (sz > 0) {
            if(memcmp(chain->buffer + chain->misalign + from, sep, sz)) {
                return false;
            }
        }
        chain = chain->next;
        sep += sz;
        len -= sz;
        from = 0;
    }
}


int buffer_search(buffer_t *buf, const char* sep, const int seplen) {
    buf_chain_t *chain;
    int i;
    chain = buf->head;
    if (chain == NULL)
        return 0;
    int bytes = chain->offset;
    while (bytes <= buf->last_read_pos) {
        chain = chain->next;
        if (chain == NULL)
            return 0;
        bytes += chain->offset;
    }
    bytes -= buf->last_read_pos;
    int from = chain->offset - bytes;
    for (i = buf->last_read_pos; i <= buf->total_len - seplen; i++) {
        if (check_sep(chain, from, sep, seplen)) {
            buf->last_read_pos = 0;
            return i+seplen;
        }
        ++from;
        --bytes;
        if (bytes == 0) {
            chain = chain->next;
            from = 0;
            if (chain == NULL)
                break;
            bytes = chain->offset;
        }
    }
    buf->last_read_pos = i;
    return 0;
}

uint8_t * buffer_write_atmost(buffer_t *p) {
    buf_chain_t *chain, *next, *tmp, *last_with_data;
    uint8_t *buffer;
    uint32_t remaining;
    int removed_last_with_data = 0;
    int removed_last_with_datap = 0;

    chain = p->head;
    uint32_t size = p->total_len;

    if (chain->offset >= size) {
        return chain->buffer + chain->misalign;
    }

    remaining = size - chain->offset;
    for (tmp=chain->next; tmp; tmp=tmp->next) {
        if (tmp->offset >= (size_t)remaining)
            break;
        remaining -= tmp->offset;
    }
    if (chain->buffer_len - chain->misalign >= (size_t)size) {
        /* already have enough space in the first chain */
        size_t old_off = chain->offset;
        buffer = chain->buffer + chain->misalign + chain->offset;
        tmp = chain;
        tmp->offset = size;
        size -= old_off;
        chain = chain->next;
    } else {
        if ((tmp = buf_chain_new(size)) == NULL) {
            return NULL;
        }
        buffer = tmp->buffer;
        tmp->offset = size;
        p->head = tmp;
    }

    last_with_data = *p->last_with_data;
    for (; chain != NULL && (size_t)size >= chain->offset; chain = next) {
        next = chain->next;

        if (chain->buffer) {
            memcpy(buffer, chain->buffer + chain->misalign, chain->offset);
            size -= chain->offset;
            buffer += chain->offset;
        }
        if (chain == last_with_data)
            removed_last_with_data = 1;
        if (&chain->next == p->last_with_data)
            removed_last_with_datap = 1;

        free(chain);
    }

    if (chain != NULL) {
        memcpy(buffer, chain->buffer + chain->misalign, size);
        chain->misalign += size;
        chain->offset -= size;
    } else {
        p->tail = tmp;
    }

    tmp->next = chain;

    if (removed_last_with_data) {
        p->last_with_data = &p->head;
    } else if (removed_last_with_data) {
        if (p->head->next && p->head->next->offset)
            p->last_with_data = &p->head->next;
        else
            p->last_with_data = &p->head;
    }
    return tmp->buffer + tmp->misalign;
}

