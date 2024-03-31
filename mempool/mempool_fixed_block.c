#include <stdio.h>
#include <stdlib.h>
#define MAX_PAGE_SIZE   4096 //4K 与内存的页一致

typedef struct mempool_s {
    int block_size; //每个小固定块大小
    int free_count; //剩余块个数
    
    char *free_ptr; //下一个空余块指针
    char *mem;      //结构体指针
} mempool_t;

//初始化
int mp_init(mempool_t *m, int size) {
    if (!m) return -1;
    if (size < 16) size = 16;

    m->block_size = size;

    m->mem = (char*)malloc(MAX_PAGE_SIZE);
    if (!m->mem) return -1;
    m->free_ptr = m->mem;
    m->free_count = MAX_PAGE_SIZE / size;

    //二级指针 构造链表
    int i = 0;
    char* ptr = m->free_ptr;
    for (i = 0; i < m->free_count; i++) {
        *(char **)ptr = ptr + size;
        ptr += size;
    }
    *(char **)ptr = NULL;
    return 0;
}
//销毁
void mp_dest(mempool_t *m) {
    if (!m || !m->mem) return;
    free(m->mem);
}
//分配
void *mp_alloc(mempool_t *m) {
    if (!m || m->free_count == 0) return NULL;
    char *ptr = m->free_ptr;
    //指向下一个块
    m->free_ptr = *(char **)ptr;
    m->free_count--;

    return ptr;
}
//释放
void *mp_free(mempool_t *m, void *ptr) {
    *(char **)ptr = m->free_ptr;
    m->free_ptr = (char *)ptr;
    m->free_count++;
}

int main() {
    mempool_t m;
    //block size: 32
    mp_init(&m, 32);

    void *p1 = mp_alloc(&m);
    printf("1: mp_alloc: %p\n", p1);

    void *p2 = mp_alloc(&m);
    printf("2: mp_alloc: %p\n", p2);

    void *p3 = mp_alloc(&m);
    printf("3: mp_alloc: %p\n", p3);

    void *p4 = mp_alloc(&m);
    printf("4: mp_alloc: %p\n", p4);
    
    mp_free(&m, p2);

    void *p5 = mp_alloc(&m);
    printf("5: mp_alloc: %p\n", p5);


    return 0;
}