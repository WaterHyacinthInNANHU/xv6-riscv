#include "types.h"
#include "riscv.h"
#include "defs.h"
#include "param.h"
#include "memlayout.h"
#include "spinlock.h"
#include "proc.h"

uint64
sys_exit(void)
{
  int n;
  argint(0, &n);
  exit(n);
  return 0;  // not reached
}

uint64
sys_getpid(void)
{
  return myproc()->pid;
}

uint64
sys_fork(void)
{
  return fork();
}

uint64
sys_wait(void)
{
  uint64 p;
  argaddr(0, &p);
  return wait(p);
}

uint64
sys_sbrk(void)
{
  uint64 addr;
  int n;

  argint(0, &n);
  addr = myproc()->sz;
  if(growproc(n) < 0)
    return -1;
  return addr;
}

uint64
sys_sleep(void)
{
  int n;
  uint ticks0;

  argint(0, &n);
  acquire(&tickslock);
  ticks0 = ticks;
  while(ticks - ticks0 < n){
    if(killed(myproc())){
      release(&tickslock);
      return -1;
    }
    sleep(&ticks, &tickslock);
  }
  release(&tickslock);
  return 0;
}

uint64
sys_kill(void)
{
  int pid;

  argint(0, &pid);
  return kill(pid);
}

// return how many clock tick interrupts have occurred
// since start.
uint64
sys_uptime(void)
{
  uint xticks;

  acquire(&tickslock);
  xticks = ticks;
  release(&tickslock);
  return xticks;
}

uint64
sys_clone(void)
{
    uint64 stack;
    struct proc *np;
    struct proc *p = myproc();
    
    // Get stack argument
    argaddr(0, &stack);
    
    // Basic sanity check
    if(stack == 0)
        return -1;
    
    // Create new thread
    if((np = allocproc_thread(p)) == 0){
        return -1;
    }
    
    // Set up the child's stack pointer
    np->trapframe->sp = stack;
    
    // Map trapframe for the child thread
    uint64 trapframe_va = TRAPFRAME - PGSIZE * np->thread_id;
    if(mappages(np->pagetable, trapframe_va, PGSIZE,
                (uint64)(np->trapframe), PTE_R | PTE_W) < 0) {
        freeproc(np);
        return -1;
    }
    
    // Set child return value to 0
    np->trapframe->a0 = 0;
    
    acquire(&np->lock);
    np->state = RUNNABLE;
    release(&np->lock);
    
    // Return child PID to parent
    return np->pid;
}