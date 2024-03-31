
#include <stdlib.h>
#include <stdio.h>

#define MAX_PAGE_SIZE   4096

typedef struct mp_node_s {
    char *free_ptr; //free_start_postion
    char *end;      //end_postion

    struct mp_node_s *next;
} mp_node_t;


typedef struct mp_pool_s {
    struct mp_pool_s *first; //mempool start postion
    struct mp_pool_s *current; //current free node
    int max;        //size of node 
} mp_pool_t;

int mp_init(mp_pool_t *m, int size) {
    if (!m || size <= 0) return -1;

    void *addr = malloc(size);
    mp_node_t *node = (mp_node_t *)addr;

    node->free_ptr = (char *)node + sizeof(mp_node_t);
    node->end = (char *)node + size;
    node->next = NULL;

    m->first = node;
    m->current = node;
    m->max = size;
    return 0;
}

void mp_dest(mp_pool_t *m) {
    if (!m) return ;

    while (m->first) {
        void *addr = m->first;
        mp_node_t *node = (mp_node_t *)addr;

        m->first = node->next;
        free(addr);
    }

    return ;
}

void *mp_alloc(mp_pool_t *m, int size) {
    void *addr = m->current;
    mp_node_t *node = (mp_node_t*)addr;
    do {
        if (size <= (node->end - node->free_ptr)) {
            char *ptr = node->free_ptr;
            node->free_ptr += size;

            return ptr;
        } 
        node = node->next;
    } while (node);

    //new node
    addr = malloc(m->max);
    node = (mp_node_t*)addr;

    node->free_ptr = (char*)addr + sizeof(mp_node_t);
    node->end = (char*)addr + m->max;
    //head-insert
    node->next = m->current;
    m->current = node;

    char *ptr = node->free_ptr;
    node->free_ptr += size;

    return ptr;
}

//destroy
void *mp_free(mp_pool_t *m, void *ptr) {

}





int main() {
    mp_pool_t m;

    mp_init(&m, MAX_PAGE_SIZE);

    void *p1 = mp_alloc(&m, 16);
    printf("1: mp_alloc: %p\n", p1);

    void *p2 = mp_alloc(&m, 32);
    printf("2: mp_alloc: %p\n", p2);

    void *p3 = mp_alloc(&m, 64);
    printf("3: mp_alloc: %p\n", p3);

    void *p4 = mp_alloc(&m, 128);
    printf("4: mp_alloc: %p\n", p4);
    //destroy
    mp_dest(&m);

    void *p5 = mp_alloc(&m, 256);
    printf("5: mp_alloc: %p\n", p5);


    return 0;
}