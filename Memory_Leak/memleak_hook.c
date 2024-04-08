
//multi_files
#define _GNU_SOURCE
#include <dlfcn.h>
#include <link.h>

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define MAX     128

//hook

typedef void *(*malloc_t)(size_t size);
malloc_t malloc_f = NULL;

typedef void (*free_t)(void *ptr);
free_t free_f = NULL;

int enable_malloc = 1;
int enable_free = 1;


void *ConvertToELF(void *addr) {
    Dl_info info;
    struct link_map *link;

    dladdr1(addr, &info, (void **)&link, RTLD_DL_LINKMAP);

    return (void *)((size_t)addr - link->l_addr);
}

void *malloc(size_t size) {
    void *p = NULL;

    if (enable_malloc) {
        enable_malloc = 0;

        p = malloc_f(size);

        void *caller = __builtin_return_address(0);

        char buffer[MAX] = {0};
        sprintf(buffer, "./%p.mem", p);

        FILE *fp = fopen(buffer, "w");
        if (!fp) {
            free(p);
            return NULL;
        }

        fprintf(fp, "[+]%p, addr: %p, size: %ld\n", ConvertToELF(caller), p, size);
        fflush(fp);

        enable_malloc = 1;
    } else {
        p = malloc_f(size);
    }

    return p;
}

void free(void *ptr) {

    if (enable_free) {
        enable_free = 0;

        char buffer[MAX] = {0};
        snprintf(buffer, MAX, "./%p.mem", ptr);

        if (unlink(buffer) < 0) {
            printf("double free: %p", ptr);
            return;
        }

        free_f(ptr);

        enable_free = 1;
    } else {
        free_f(ptr);
    }

    return;
}


void init_hook(void) {

    if (!malloc_f) {
        malloc_f = (malloc_t)dlsym(RTLD_NEXT, "malloc");
    }
    if (!free_f) {
        free_f = (free_t)dlsym(RTLD_NEXT, "free");
    }
}



int main() {

    init_hook();

    void *p1 = malloc(5);
    void *p2 = malloc(10);

    free(p1);
    //free(p2);

    //getchar();
}