#include "types.h"
#include "x86.h"
#include "defs.h"
#include "date.h"
#include "param.h"
#include "memlayout.h"
#include "mmu.h"
#include "proc.h"

int
sys_fork(void)
{
  return fork();
}

int
sys_exit(void)
{
  exit();
  return 0;  // not reached
}

int
sys_wait(void)
{
  return wait();
}

int
sys_kill(void)
{
  int pid;

  if(argint(0, &pid) < 0)
    return -1;
  return kill(pid);
}

int
sys_getpid(void)
{
  return myproc()->pid;
}

int
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

int
sys_sleep(void)
{
  int n;
  uint ticks0;

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

// return how many clock tick interrupts have occurred
// since start.
int
sys_uptime(void)
{
  uint xticks;

  acquire(&tickslock);
  xticks = ticks;
  release(&tickslock);
  return xticks;
}

extern int toggle;
extern int sys_call_count[28];


char* syscall_names[] = {
              "",
              "sys_fork",
              "sys_exit",
              "sys_wait",
              "sys_pipe",
              "sys_read",
              "sys_kill",
              "sys_exec",
              "sys_fstat",
              "sys_chdir",
              "sys_dup",
              "sys_getpid",
              "sys_sbrk",
              "sys_sleep",
              "sys_uptime",
              "sys_open",
              "sys_write",
              "sys_mknod",
              "sys_unlink",
              "sys_link",
              "sys_mkdir",
              "sys_close",
              "sys_toggle", 
              "sys_print_count",
              "sys_add",
              "sys_ps",
              "sys_send",
              "sys_recv"
};


int
sys_toggle(void)
{
  toggle = 1 - toggle;
  for(int i=0;i<28;i++)
    sys_call_count[i] = 0;
  // cprintf("toggle: %d\n", toggle);
  return toggle;
}

int
sys_print_count(void)
{
  for(int i=0;i<28;i++){
    if(sys_call_count[i])
      cprintf("%s %d\n", syscall_names[i], sys_call_count[i]);
  }
  return 0;
}

int
sys_add(void)
{
	int a, b;
  if(argint(0, &a) < 0)
    return -1;
  if(argint(1, &b) < 0)
    return -1;
  // cprintf("%d", a + b);
  return a + b;
}

int
sys_ps(void)
{
  ps();
  return 0;
}

int
sys_send(void)
{
  int sender_pid, rec_pid;
  char *msg;
  if(argint(0, &sender_pid) < 0)
    return -1;
  if(argint(1, &rec_pid) < 0)
    return -1;
  if(argstr(2, &msg) < 0)
    return -1;
  
  return send(sender_pid, rec_pid, msg);;
}

int 
sys_recv(void)
{
  char *msg;
  if(argstr(0, &msg) < 0)
    return -1;
  recv(msg);
  return 0;
}

// int
// sys_send_multi(void)
// {
//   int sender_pid, length;
//   int *rec_pid;
//   char *msg;
//   if(argint(0, &sender_pid) < 0)
//     return -1;
//   if(argint(1, &rec_pid) < 0)
//     return -1;
//   if(argstr(2, &msg) < 0)
//     return -1;
//   if(argint(3, &length) < 0)
//     return -1;

// }