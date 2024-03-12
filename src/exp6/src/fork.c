#include "mm.h"
#include "sched.h"
#include "fork.h"
#include "utils.h"
#include "entry.h"

int copy_process(unsigned long clone_flags, unsigned long fn, unsigned long arg)
{
	preempt_disable();
	struct task_struct *p;

	unsigned long page = allocate_kernel_page();
	p = (struct task_struct *) page;
	struct pt_regs *childregs = task_pt_regs(p);

	if (!p)
		return -1;

	if (clone_flags & PF_KTHREAD) {
		p->cpu_context.x19 = fn;
		p->cpu_context.x20 = arg;
	} else {
		struct pt_regs * cur_regs = task_pt_regs(current);
		/* Copy cur_regs to childregs. It's a NOT bug, b/c it will be compiled to invocation of memcpy(), 
			and our memcpy() has src/dst reversed. To reverse below, memcpy() has to be fixed as well...*/
		*cur_regs = *childregs;
		childregs->regs[0] = 0;
		copy_virt_memory(p);
	}
	p->flags = clone_flags;
	p->priority = current->priority;
	p->state = TASK_RUNNING;
	p->counter = p->priority;
	p->preempt_count = 1; //disable preemtion until schedule_tail
	// @page is 0-filled, many fields (e.g. mm.pgd) are implicitly init'd
	
	p->cpu_context.pc = (unsigned long)ret_from_fork;
	p->cpu_context.sp = (unsigned long)childregs;
	int pid = nr_tasks++;
	task[pid] = p;	

	preempt_enable();
	return pid;
}


/* 
	Populate pt_regs for returning to user space (via kernel_exit) for the 1st time. 
   	Note that the actual switch will not happen until kernel_exit. 

	@start: a pointer to the beginning of the user code (to be copied to the new task), 
   @size: size of the area 
   @pc: offset of the startup function inside the area
*/   

int move_to_user_mode(unsigned long start, unsigned long size, unsigned long pc)
{
	struct pt_regs *regs = task_pt_regs(current);
	regs->pstate = PSR_MODE_EL0t;
	regs->pc = pc;
	/* assumption: our toy user program will not exceed 1 page. the 2nd page will serve as the stack */
	regs->sp = 2 *  PAGE_SIZE;  
	/* only allocate 1 code page here b/c the stack page is to be mapped on demand. 
	   this will trigger allocating the task's pgtable tree (mm.pgd) */
	unsigned long code_page = allocate_user_page(current, 0);
	if (code_page == 0)	{
		return -1;
	}
	memcpy(start, code_page, size); /* NB: arg1-src; arg2-dest */
	set_pgd(current->mm.pgd);
	return 0;
}

/* get a task's saved registers, which are at the top of the task's kernel page. 
   these regs are saved/restored by kernel_entry()/kernel_exit(). 
*/
struct pt_regs * task_pt_regs(struct task_struct *tsk)
{
	unsigned long p = (unsigned long)tsk + THREAD_SIZE - sizeof(struct pt_regs);
	return (struct pt_regs *)p;
}
