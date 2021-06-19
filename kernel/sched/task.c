#include "task.h"
#include "debug.h"
#include "gdt.h"
#include "heap.h"
#include "pmm.h"
#include "sched.h"
#include "string.h"
#include "vmm.h"

pid_t now_pid = 0;

int32_t kernel_thread(int (*fn)(void *), void *arg) {
  // 从内核堆中申请一块区域作为新内核线程的栈
  struct task_struct *new_task = (struct task_struct *)kmalloc(STACK_SIZE);

  assert(new_task != NULL, "kern_thread: kmalloc error!");

  bzero(new_task, sizeof(struct task_struct));

  new_task->state = TASK_RUNNABLE;
  new_task->stack = new_task;
  new_task->pid = now_pid++;
  new_task->mm = NULL;

  uint32_t *stack_top = (uint32_t *)((uint32_t)new_task + STACK_SIZE);

  // 初始化内核线程的栈
  *(--stack_top) = (uint32_t)arg;
  *(--stack_top) = (uint32_t)kthread_exit;
  *(--stack_top) = (uint32_t)fn;

  // 内核线程的栈指针指向了将要运行的函数fn地址，这样在切换线程上下文时，
  // switch_to中最后的ret指令自动从栈中弹出fn的地址，并跳转过去执行
  new_task->context.esp =
      (uint32_t)new_task + STACK_SIZE - sizeof(uint32_t) * 3;

  // 设置新任务的标志寄存器未屏蔽中断，很重要
  new_task->context.eflags = 0x200;
  new_task->next = running_proc_head;

  // 找到当前进任务队列，插入到末尾
  struct task_struct *tail = running_proc_head;
  assert(tail != NULL, "Must ini sched!");

  while (tail->next != running_proc_head)
    tail = tail->next;

  tail->next = new_task;

  return new_task->pid;
}

void kthread_exit() {
  register uint32_t val asm("eax");

  printk("Thread exited with value %d\n", val);

  // TODO
  // 内核退出函数在这里只实现了简陋的一部分，标准做法是将退出线程的PCB结构转移到
  // 不可调度链表去，等待其他线程join后再清理结构。
  while (1)
    ;
}