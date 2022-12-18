#include "types.h"
#include "riscv.h"
#include "defs.h"
#include "date.h"
#include "param.h"
#include "memlayout.h"
#include "spinlock.h"
#include "proc.h"

uint64
sys_exit(void)
{
  int n;
  if(argint(0, &n) < 0)
    return -1;
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
  if(argaddr(0, &p) < 0)
    return -1;
  return wait(p);
}

uint64
sys_sbrk(void)
{
  int addr;
  int n;

  if(argint(0, &n) < 0)
    return -1;
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

  backtrace();
  if(argint(0, &n) < 0)
    return -1;
  acquire(&tickslock);
  ticks0 = ticks;
  while(ticks - ticks0 < n){
    if(myproc()->killed){
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

  if(argint(0, &pid) < 0)
    return -1;
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

uint64 sys_sigalarm(void) {
  struct proc *p = myproc();
  int n;
  uint64 addr;
  if(argint(0, &n) < 0)
    return -1;
  if(argaddr(1, &addr) < 0)
    return -1;
  if(p->handling_signal == 1 || p->in_a_handler == 1)
    return -1;
  p->alarm_interval = n;
  // printf("alarm_interval: %d\n", p->alarm_interval);
  p->handler = (void *) addr; // syntax of casting a function pointer is like (void (*)()) ptr
  p->handling_signal = 1;
  if (addr == 0)
    p->handler_not_null = 0;
  else 
    p->handler_not_null = 1;
  // printf("handler: %p\n", p->handler);
  return 0;
}
uint64 sys_sigreturn(void)
{
  struct proc *p = myproc();
  if(p->handling_signal == 0)
    return -1;
  p->handling_signal = 0;
  p->alarm_interval = 0;
  p->handler = 0;
  // if (p->handler_not_null == 0) {
    // means that the handler was null
    // so we need to restore the old trapframe
    memmove(p->trapframe, p->old_trapframe, sizeof(struct trapframe));
    kfree(p->old_trapframe);
    p->old_trapframe = 0;
    p->alarm_interval = p->old_alarm_interval;
    p->handler_not_null = p->old_handler_not_null;
    p->handler = p->old_handler;
    p->handling_signal = p->old_handling_signal;
  // }
  p->in_a_handler = 0;
  return 0;
  
}
