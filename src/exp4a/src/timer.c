#include "utils.h"
#include "printf.h"
#include "sched.h"
#include "peripherals/timer.h"
#include "timer.h"

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

void handle_generic_timer_irq( void ) 
{
	struct task_struct * p;
	for (int i = 0; i < NR_TASKS; i++){
			p = task[i];
			if(p && p->state == TASK_WAIT&&(p->suspend_time>0)){
				p->suspend_time--;
				if(p->suspend_time==0){
					p->state = TASK_RUNNING;
				}
			}
		}
	gen_timer_reset(interval);	

	// printf("Timer interrupt received. next in %u ticks\n\r", interval);
}
