#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "user/thread.h"

#define PGSIZE 4096

int 
thread_create(void *(*start_routine)(void*), void *arg) 
{
    void *stack;
    int pid;
    
    // Allocate a page for the thread's stack
    stack = malloc(PGSIZE);
    if (stack == 0) {
        return -1;
    }
    
    // Stack grows downward, so start at the top of the allocated page
    // Align to 16-byte boundary as required by RISC-V ABI
    void *stack_top = (void*)((char*)stack + PGSIZE);
    stack_top = (void*)((uint64)stack_top & ~15);  // 16-byte align
    
    // Call clone to create the thread
    pid = clone(stack_top);
    
    if (pid < 0) {
        // Clone failed
        free(stack);
        return -1;
    } else if (pid == 0) {
        // Child thread - call the start routine
        // void *result = start_routine(arg);
        start_routine(arg);
        
        // Free the stack before exiting
        free(stack);
        
        // Exit the thread
        exit(0);
    } else {
        // Parent - return success (0 on success, -1 on failure)
        return 0;
    }
}

void 
lock_init(struct lock_t* lock) 
{
    lock->locked = 0;
}

void 
lock_acquire(struct lock_t* lock) 
{
    // Spin until we can acquire the lock using atomic test-and-set
    while (__sync_lock_test_and_set(&lock->locked, 1) != 0) {
        // Busy wait - could add a small delay here if needed
        // In a more sophisticated implementation, we might yield here
    }
    
    // Memory barrier to ensure lock acquisition is visible before 
    // any operations in the critical section
    __sync_synchronize();
}

void 
lock_release(struct lock_t* lock) 
{
    // Memory barrier to ensure all operations in critical section
    // complete before releasing the lock
    __sync_synchronize();
    
    // Release the lock atomically
    __sync_lock_release(&lock->locked);
}