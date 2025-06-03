#ifndef THREAD_H
#define THREAD_H

// Lock structure for spin locks
struct lock_t {
    uint locked;
};

// Create a type alias so we can use 'lock_t' instead of 'struct lock_t'
typedef struct lock_t lock_t;

// Function prototypes
int thread_create(void *(*start_routine)(void*), void *arg);
void lock_init(lock_t* lock);
void lock_acquire(lock_t* lock);
void lock_release(lock_t* lock);

#endif