#ifndef _SCHED_H
#define _SCHED_H

#define THREAD_CPU_CONTEXT			0 		// offset of cpu_context in task_struct 

#ifndef __ASSEMBLER__

#define THREAD_SIZE				4096

#define NR_TASKS				64 

#define FIRST_TASK task[0]
#define LAST_TASK task[NR_TASKS-1]

/* 
	By classic OS textbook/lectures, 
	READY means ready to run but is not running because of no idle CPU, not scheduled yet, etc. 
	RUNNING means the task is running and currently uses CPU.

	However, in the simple impl. below, 
	TASK_RUNNING represents either RUNNING or READY 
*/
#define TASK_RUNNING				0
/* TODO: define more task states (as constants) below, e.g. TASK_WAIT */
#define TASK_WAIT					1
extern struct task_struct *current;
extern struct task_struct * task[NR_TASKS];
extern int nr_tasks;

// Contains values of all registers that might be different between the tasks.
struct cpu_context {
	unsigned long x19;
	unsigned long x20;
	unsigned long x21;
	unsigned long x22;
	unsigned long x23;
	unsigned long x24;
	unsigned long x25;
	unsigned long x26;
	unsigned long x27;
	unsigned long x28;
	unsigned long fp;
	unsigned long sp;
	unsigned long pc;
};

// the metadata describing a task
struct task_struct {
	struct cpu_context cpu_context;	// register values
	long state;		// the state of the current task, e.g. TASK_RUNNING
	long counter;	// how long this task has been running? decreases by 1 each timer tick. Reaching 0, kernel will attempt to schedule another task. Support our simple sched
	long priority;	// when kernel schedules a new task, the kernel copies the task's  `priority` value to `counter`. Regulate CPU time the task gets relative to other tasks 
	long preempt_count; // a flag. A non-zero means that the task is executing in a critical code region cannot be interrupted, Any timer tick should be ignored and not triggering rescheduling
	long suspend_time; // the time when the task should be waken up
};

extern void sched_init(void);
extern void schedule(void);
//extern void timer_tick(void);
//extern void preempt_disable(void);
//extern void preempt_enable(void);
extern void switch_to(struct task_struct* next);
extern void cpu_switch_to(struct task_struct* prev, struct task_struct* next);
extern void sleep(int time);
// the initial values for task_struct that belongs to the init task. see sched.c 
#define INIT_TASK 									\
{ 													\
	{0,0,0,0,0,0,0,0,0,0,0,0,0}, 	/*cpu_context*/	\
	1,	/* state */									\
	0,	/* counter */								\
	1,	/* priority */								\
	0, 	/* preempt_count */							\
	0	/* suspend_time */							\
}

#endif
#endif
