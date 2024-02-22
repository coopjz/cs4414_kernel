#include "sched.h"
#include "irq.h"
#include "printf.h"
#include "timer.h"
#include "stddef.h"
#include "utils.h"
static struct task_struct init_task = INIT_TASK; // the very first task with its task_struct values
struct task_struct *current = &(init_task);		 // points to the currently running task. when kernel boots, sets to init_task
struct task_struct * task[NR_TASKS] = {&(init_task), }; // holds all task_strcuts. only has init_ask at beginning
int nr_tasks = 1;

void _schedule(void)
{
	int next, c;
	int running =0;
	struct task_struct * p;
	while (1) {
		c = -1;	// the maximum counter found so far
		next = 0;
		running = 0;

		/* Iterates over all tasks and tries to find a task in 
		TASK_RUNNING state with the maximum counter. If such 
		a task is found, we immediately break from the while loop 
		and switch to this task. */

		for (int i = 0; i < NR_TASKS; i++){
			p = task[i];
			if (p && p->state == TASK_RUNNING)
			{
				++running;
				if(p->counter > c) 
				{ /* NB: p->counter always be non negative */
				c = p->counter;
				next = i;
				}
			}
		}
		// if (c) {	/* found a RUNNING/READY task w/ the most positive counter.  NB: c won't be -1 as counter always nonnegative */
		// 	break;
		// }
		if(running>0&&c>0)
		{
			break;
		}
		/* If no such task is found, this is either because i) no 
		task is in TASK_RUNNING state or ii) all such tasks have 0 counters.
		in our current implemenation which misses TASK_WAIT, only condition ii) is possible. 
		Hence, we recharge counters. Bump counters for all tasks once. */

	
		if(running==0)
		{
			printf("No task is running. System idle\n");
			asm("wfi");
		}
		else
		{

		for (int i = 0; i < NR_TASKS; i++) {
			p = task[i];
			if (p) {
				p->counter = (p->counter >> 1) + p->priority; // The increment depends on a task's priority.
			}
		}
		}

		/* loops back to pick the next task */
	}
	switch_to(task[next]);
}

void schedule(void)
{
	current->counter = 0;
	_schedule();
}

// where the multitasking magic happens
void switch_to(struct task_struct * next) 
{
	if (current == next) 
		return;
	struct task_struct * prev = current;
	current = next;
	cpu_switch_to(prev, next); // do context switch
}

void schedule_tail(void) {
	/* nothing */
}


void sleep(int time)
{
	int interval = (1 << 26);
	
	current->state = TASK_WAIT;
	current->suspend_time = time*interval;
	printf("suspend time: %u\n", current->suspend_time);
	
	if(time>0)
	{
		
		
		if(task_to_unblock==NULL)
		{
			task_to_unblock = current;
			least_suspend_time = current->suspend_time;
			printf("block current task\n");
			printf("for %u ticks\n", current->suspend_time);
			gen_timer_reset(current->suspend_time);

		}
		else
		{	
			unsigned int current_time = ((get_timer_interval()<<32)>>32);
			printf ("current time: %u\n", current_time);
			int ticked_time = least_suspend_time - current_time;
			printf ("tick time: %u\n", ticked_time);
			if (current->suspend_time<current_time)
			{	
				printf("go 1\n");
				if(current->suspend_time<least_suspend_time)
				{
					printf("go 2\n");
					least_suspend_time = current->suspend_time;
				}
				struct task_struct * p;
				for (int i = 0; i < NR_TASKS; i++){
					p = task[i];
					if(p && p->state == TASK_WAIT&&(p->suspend_time>0)){
						p->suspend_time -= ticked_time;
					}
				}
				task_to_unblock = current;
				gen_timer_reset(current->suspend_time);
			}
		}
	
		
		schedule();
	}
	else
	{
	schedule();
	}

}

