#include "utils.h"
#include "printf.h"
#include "sched.h"
#include "peripherals/timer.h"
#include "timer.h"
#include "stddef.h"

#ifdef USE_QEMU
int interval = (1 << 26); // xzl: around 1 sec
#else
int interval = 1 * 1000 * 1000; // xzl: around 1 sec
#endif

/* 	These are for Arm generic timers. 
	They are fully functional on both QEMU and Rpi3.
	Recommended.
*/
void generic_timer_init ( void )
{
	gen_timer_init();
}
struct task_struct * task_to_unblock=NULL;
int least_suspend_time = 0x7FFFFFFF;
void handle_generic_timer_irq( void ) 
{
	if(task_to_unblock!=NULL)
	{
		task_to_unblock->state = TASK_RUNNING;
		task_to_unblock->suspend_time = 0;
		task_to_unblock = NULL;

	}
	
	// else

	struct task_struct * p;

	for (int i = 0; i < NR_TASKS; i++){
		p = task[i];
		if(p && p->state == TASK_WAIT&&(p->suspend_time>0)){
			p->suspend_time -= least_suspend_time;
		}
	}
	for (int i = 0; i < NR_TASKS; i++){
			p = task[i];
			if(p && p->state == TASK_WAIT&&(p->suspend_time>0)){
				if(p->suspend_time<least_suspend_time){
					least_suspend_time = p->suspend_time;
					task_to_unblock = p;
				}
			}
		}
	
	printf("Timer interrupt received. next in %u ticks\n\r", least_suspend_time);
	gen_timer_reset(least_suspend_time);	


}
