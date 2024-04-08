
//single_file
#define _GNU_SOURCE
#include <dlfcn.h>
#include <link.h>

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define MAX     128
//memory leak start flag
int flag = 1;


void *nMalloc(size_t size, const char *filename, int line) {
    void *p = malloc(size);

    if (flag) {
        char buffer[MAX] = {0};
        snprintf(buffer, MAX, "./%p.mem", p);

        FILE *fp = fopen(buffer, "w");
        if (!fp) {
            free(p);
            return NULL;
        }

        fprintf(fp, "[+]%s:%d, addr: %p, size: %ld\n", filename, line, p, size);
        fflush(fp);
        fclose(fp);
    }

    //printf("nMalloc: %p, size: %ld\n", p, size);
    return p;
}

void nFree(void *p) {
    //printf("nFree: %p, \n", p);

    if (flag) {
        char buffer[MAX] = {0};
        snprintf(buffer, MAX, "./%p.mem", p);

        if (unlink(buffer) < 0) {
            printf("double free: %p", p);
            return;
        }
    }

    return free(p);
}

#define malloc(size) nMalloc(size, __FILE__, __LINE__)
#define free(ptr) nFree(ptr)


int main() {

    init_hook();

    void *p1 = malloc(5);
    void *p2 = malloc(10);

    free(p1);
    //free(p2);

    getchar();
}