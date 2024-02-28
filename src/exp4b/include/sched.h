#ifndef _SCHED_H
#define _SCHED_H

#define THREAD_CPU_CONTEXT			0 		// offset of cpu_context in task_struct 

#ifndef __ASSEMBLER__

#define THREAD_SIZE				4096

#define NR_TASKS				64 

#define FIRST_TASK task[0]
#define LAST_TASK task[NR_TASKS-1]

#define TASK_RUNNING				0

#define SWITCH_TRACING_STACK_SIZE 512
extern struct task_struct *current;
extern struct task_struct * task[NR_TASKS];
extern int nr_tasks;

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

struct task_struct {
	struct cpu_context cpu_context;
	long state;	
	long counter; /* countdown for scheduling. higher value means having run for less. recharged in schedule(). decremented in timer_tick(). always non negative */
	long priority;
	long preempt_count;
	unsigned long pc_;
	unsigned long sp_;
};
struct switch_tracing {
	unsigned long time_stamp;
	int prev_pid;
	int next_pid;
	unsigned long prev_pc;
	unsigned long next_pc;
	unsigned long prev_sp;
	unsigned long next_sp;
};
extern struct switch_tracing switch_tracing_stack[SWITCH_TRACING_STACK_SIZE];
extern int stackcnt;
extern struct switch_tracing *switch_tracing_head;
extern struct switch_tracing *switch_tracing_tail;
struct tracing_link_list {
	struct switch_tracing *tracing;
	struct tracing_link_list *next;
};

extern void sched_init(void);
extern void schedule(void);
extern void timer_tick(void);
extern void preempt_disable(void);
extern void preempt_enable(void);
extern void switch_to(struct task_struct* next);
extern void cpu_switch_to(struct task_struct* prev, struct task_struct* next);
extern int getpid(void);

#define INIT_TASK \
/*cpu_context*/	{ {0,0,0,0,0,0,0,0,0,0,0,0,0}, \
/* state etc */	0,0,1, 0 \
}

#endif
#endif
