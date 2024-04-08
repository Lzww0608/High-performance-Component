
#define _GNU_SOURCE

#include <dlfcn.h>
#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include <stdint.h>

//typedef unsigned long int uint64;

// Maximum number of vertex in the graph
#define MAX     100

enum Type {PROCESS, RESOURCE};
// Structure to represent a source (process or resource) in the graph.
struct source_type {

    uint64_t id;    // Unique identifier.
    enum Type type; // Distinguishes between process and resource.

    uint64_t lock_id;    // ID of the lock associated with this source.
    int degree; // Degree of the vertex in the graph (not used in provided code segment)
};

// Structure for a vertex in the adjacency list.
struct vertex {
    struct source_type s;
    struct vertex *next;
};

// Structure for the graph, represented as an adjacency list.
struct task_graph {
    
    struct vertex list[MAX];
    int num;    //number of vertex

    struct source_type locklist[MAX];// List of locks (not used in provided code segment)
    int lock_idx;   // Index for locklist (not used).

    pthread_mutex_t mutex;  // Mutex to protect access to the graph.
};


struct task_graph *tg = NULL;
int path[MAX+1];// Array to store paths during deadlock detection.
int visited[MAX];// Counter for the current position in the path array.
int k = 0;
int deadlock = 0;// Flag to indicate a deadlock.


// Creates a new vertex in the graph for a given source (process or resource).
// Allocates memory for the vertex, initializes it with the given source, and sets its next pointer to NULL.
struct vertex *create_vertex(struct source_type type) {
    struct vertex *tex = (struct vertex*)malloc(sizeof(struct vertex));
    tex->s = type;
    tex->next = NULL;

    return tex;
}

// Searches the graph for a vertex that matches the given source type (process or resource) and ID.
// Returns the index of the matching vertex in the graph's list array if found, or -1 if not found.
int search_vertex(struct source_type type) {
    int i = 0;

    for (i = 0; i < tg->num; i++) {
        if (tg->list[i].s.type == type.type && tg->list[i].s.id == type.id) {
            return i;
        }
    }

    return -1;
}

// Adds a new vertex to the graph if it does not already exist.
// It first checks for the existence of the vertex using search_vertex.
// If not found, it adds the vertex to the graph's list array and increments the number of vertices.
void add_vertex(struct source_type type) {
    
    if (search_vertex(type) == -1) {
        tg->list[tg->num].s = type;
        tg->list[tg->num].next = NULL;
        tg->num++;
    }
}

// Adds a directed edge from one vertex to another, representing a process waiting on a resource or vice versa.
// It ensures that both vertices exist in the graph, then finds the first vertex and adds the second vertex to its adjacency list.
int add_edge(struct source_type from, struct source_type to) {

    add_vertex(from);
    add_vertex(to);

    struct vertex *v = &(tg->list[search_vertex(from)]);

    while (v->next != NULL) {
        v = v->next;
    }

    v->next = create_vertex(to);
}

// Checks if there is an edge from one specified vertex to another.
// Returns 1 if such an edge exists, indicating the specified relationship (e.g., process waiting on a resource) is present.
int verify_edge(struct source_type i, struct source_type j) {
    if (tg->num == 0) return 0;

    int idx = search_vertex(i);
    if (idx == -1) return 0;

    struct vertex *v = &(tg->list[idx]);

    while (v != NULL) {
        if (v->s.id == j.id) {
            return 1;
        }

        v = v->next;
    }
    return 0;
}


int remove_edge(struct source_type from, struct source_type to) {
    int i = search_vertex(from);
    int j = search_vertex(to);

    if (i != -1 && j != -1) {
        struct vertex *v = &(tg->list[i]);
        struct vertex *remove;

        while (v->next != NULL) {
            if (v->next->s.id == to.id) {
                remove = v->next;
                v->next = v->next->next;
                
                free(remove);
                break;
            }

            v = v->next;
        }
        
    }
}


void print_deadlock(void) {
    int i = 0;

    printf("cycle: ");
    for (i = 0; i < k - 1; i++) {
        printf("%ld --> ", tg->list[path[i]].s.id);
    }

    printf("%ld\n ", tg->list[path[i]].s.id);
}


// DFS would mark each visited vertex and track the path. If it encounters a vertex already in the path, a cycle (deadlock) is detected.
int DFS(int idx) {
    struct vertex *v = &(tg->list[idx]);
    
    if (visited[idx] == 1) {
        path[k++] = idx;
        print_deadlock();

        deadlock = 1;
        return 0;
    }

    visited[idx] = 1;
    path[k++] = idx;

    while (v->next != NULL) {
        DFS(search_vertex(v->next->s));
        k--;

        v = v->next;
    }

    return 1;
}


int search_for_cycle(int idx) {

    struct vertex *v = &(tg->list[idx]);
    visited[idx] = 1;
    k = 0;
    path[k++] = idx;

    while (v->next != NULL) {
        int i = 0;
        //访问记录置空
        for (i = 0; i < tg->num; i++) {
            if (i == idx) continue;

            visited[i] = 0;
        }
        //路径置空
        for (i = 1; i <= MAX; i++) {
            path[i] = -1;
        }
        k = 1;

        DFS(search_vertex(v->next->s));
        v = v->next;
    }
}


int search_lock(uint64_t lock) {
    int i = 0;

    for (i = 0; i < tg->lock_idx; i++) {
        if (tg->locklist[i].lock_id == lock) {
            return i;
        }
    }

    return -1;
}


int search_empty_lock(uint64_t lock) {
    int i = 0;

    for (i = 0; i < tg->lock_idx; i++) {
        if (tg->locklist[i].lock_id == 0) {
            return i;
        }
    }

    return tg->lock_idx;
}


void lock_before(uint64_t tid, uint64_t lock_addr) {
    int i = 0;

    for (i = 0; i < tg->lock_idx; i++) {
        if (tg->locklist[i].lock_id == lock_addr) {

            struct source_type from;
            from.id = tid;
            from.type = PROCESS;
            add_vertex(from);

            struct source_type to;
            to.id = tg->locklist[i].id;
            to.type = PROCESS;
            add_vertex(to);

            tg->locklist[i].degree++;

            if (!verify_edge(from, to)) {
                add_edge(from, to);
            }
        }
    }
}


void lock_after(uint64_t tid, uint64_t lock_addr) {
    int i = 0;
    if (-1 == (i = search_lock(lock_addr))) {
        int idx = search_empty_lock(lock_addr);

        tg->locklist[idx].id = tid;
        tg->locklist[idx].lock_id = lock_addr;

        tg->lock_idx++;
    } else {
        struct source_type from;
        from.id = tid;
        from.type = PROCESS;
        add_vertex(from);

        struct source_type to;
        to.id = tg->locklist[i].id;
        to.type = PROCESS;
        add_vertex(to);

        tg->locklist[i].degree--;

        if (verify_edge(from, to)) {
            remove_edge(from, to);
        }

        tg->locklist[i].id = tid;
    }
}

void unlock_after(uint64_t tid, uint64_t lock_addr) {
    int idx = search_lock(lock_addr);

    if (tg->locklist[idx].degree == 0) {
        tg->locklist[idx].id = 0;
        tg->locklist[idx].lock_id = 0;
    }
}

void check_dead_lock(void) {
    int i = 0;

    deadlock = 0;
    for (i = 0; i < tg->num; i++) {
        if (deadlock == 1) break;
        search_for_cycle(i);
    }

    if (deadlock == 0) {
        printf("no deadlock\n");
    }
}

static void *thread_routine(void *args) {
    while (1) {
        sleep(1);
        check_dead_lock();
    }
}

void start_check(void) {
    tg = (struct task_graph*)malloc(sizeof(struct task_graph));
    tg->num = 0;
    tg->lock_idx = 0;

    pthread_t tid;

    pthread_create(&tid, NULL, thread_routine, NULL);
}


//hook
typedef int (*pthread_mutex_lock_t)(pthread_mutex_t *mtx);
pthread_mutex_lock_t pthread_mutex_lock_f = NULL;

typedef int (*pthread_mutex_unlock_t)(pthread_mutex_t *mtx);
pthread_mutex_unlock_t pthread_mutex_unlock_f = NULL;

//implementation
int pthread_mutex_lock(pthread_mutex_t *mtx) {
    /*printf("before pthread_mutex_lock: %ld\n", pthread_self());
    pthread_mutex_lock_f(mtx);
    printf("after pthread_mutex_lock\n");*/

    pthread_t selfid = pthread_self();
    lock_before((uint64_t)selfid, (uint64_t)mtx);
    pthread_mutex_lock_f(mtx);
    lock_after((uint64_t)selfid, (uint64_t)mtx);

}

int pthread_mutex_unlock(pthread_mutex_t *mtx) {
    /*printf("before pthread_mutex_unlock: %ld\n", pthread_self());
    pthread_mutex_unlock_f(mtx);
    printf("after pthread_mutex_unlock\n");*/

    pthread_mutex_unlock_f(mtx);
    pthread_t selfid = pthread_self();
    unlock_after((uint64_t)selfid, (uint64_t)mtx);
}

//function hook
void init_hook(void) {
    if (!pthread_mutex_lock_f) {
        pthread_mutex_lock_f = dlsym(RTLD_NEXT,"pthread_mutex_lock");
    }
    if (!pthread_mutex_unlock_f) {
        pthread_mutex_unlock_f = dlsym(RTLD_NEXT,"pthread_mutex_unlock");
    }
}


pthread_mutex_t mtx1 = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mtx2 = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mtx3 = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mtx4 = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mtx5 = PTHREAD_MUTEX_INITIALIZER;

void *t1_cb(void *arg) {
    printf("t1: %ld\n", pthread_self());

    pthread_mutex_lock(&mtx1);
    sleep(1);
    pthread_mutex_lock(&mtx2);
    //sleep(1);
    //printf("t1_cb\n");
    pthread_mutex_unlock(&mtx2);


    pthread_mutex_unlock(&mtx1);
}

void *t2_cb(void *arg) {
    printf("t2: %ld\n", pthread_self());

    pthread_mutex_lock(&mtx2);
    sleep(1);
    pthread_mutex_lock(&mtx3);
    //sleep(1);
    //printf("t2_cb\n");
    pthread_mutex_unlock(&mtx3);

    pthread_mutex_unlock(&mtx2);
}

void *t3_cb(void *arg) {
    printf("t3: %ld\n", pthread_self());

    pthread_mutex_lock(&mtx3);
    sleep(1);
    pthread_mutex_lock(&mtx4);

    //printf("t3_cb\n");

    pthread_mutex_unlock(&mtx4);

    pthread_mutex_unlock(&mtx3);
}

void *t4_cb(void *arg) {
    printf("t4: %ld\n", pthread_self());

    pthread_mutex_lock(&mtx4);
    sleep(1);
    pthread_mutex_lock(&mtx5);

    //printf("t4_cb\n");

    pthread_mutex_unlock(&mtx5);

    pthread_mutex_unlock(&mtx4);
}

void *t5_cb(void *arg) {
    printf("t5: %ld\n", pthread_self());

    pthread_mutex_lock(&mtx1);
    sleep(1);
    pthread_mutex_lock(&mtx5);

    //printf("t5_cb\n");

    pthread_mutex_unlock(&mtx5);

    pthread_mutex_unlock(&mtx1);
}

int main() {

    init_hook();
    start_check();

    pthread_t t1, t2, t3, t4, t5;
    //int pthread_create(pthread_t *thread, const pthread_attr_t *attr,void *(*start_routine)(void *), void *arg);
    //return 0 if succeed
    pthread_create(&t1, NULL, t1_cb, NULL);
    pthread_create(&t2, NULL, t2_cb, NULL);
    pthread_create(&t3, NULL, t3_cb, NULL);
    pthread_create(&t4, NULL, t4_cb, NULL);
    pthread_create(&t5, NULL, t5_cb, NULL);

    //int pthread_join(pthread_t thread, void **retval);
    //return 0 if succeed
    
    pthread_join(t1, NULL);
    pthread_join(t2, NULL);
    pthread_join(t3, NULL);
    pthread_join(t4, NULL);
    //pthread_join(t5, NULL);
    
    printf("complete\n");

    return 0;
}