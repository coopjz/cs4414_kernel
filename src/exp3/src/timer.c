#include "utils.h"
#include "printf.h"
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

/* CNTFRQ_EL0 reports the frequency of the system count */
static unsigned int read_cntfrq(void)
{
	unsigned int val;
	asm volatile ("mrs %0, cntfrq_el0" : "=r" (val));
  	return val;
}

void generic_timer_init ( void )
{
	unsigned int freq  = read_cntfrq();
	printf("System count freq (CNTFRQ) is: %u\n", freq);

	printf("interval is set to: %d\n", interval);
	gen_timer_init();
	gen_timer_reset(interval);
}

void handle_generic_timer_irq( void ) 
{
	printf("Timer interrupt received. next in %d ticks\n\r", interval);
	gen_timer_reset(interval);
}


/* 
	These are for "System Timer". They are NOT in use by this project. 
	I leave the code here FYI. 
	Rpi3: System Timer works fine. Can generate intrerrupts and be used as a counter for timekeeping.
	QEMU: System Timer can be used for timekeeping. Cannot generate interrupts. 
		You may want to adjust @interval as needed
	cf: 
	https://fxlin.github.io/p1-kernel/exp3/rpi-os/#fyi-other-timers-on-rpi3
*/
unsigned int curVal = 0;

void timer_init ( void )
{
	curVal = get32(TIMER_CLO);
	curVal += interval;
	put32(TIMER_C1, curVal);
}

void handle_timer_irq( void ) 
{
	curVal += interval;
	put32(TIMER_C1, curVal);
	put32(TIMER_CS, TIMER_CS_M1);
	printf("Timer interrupt received\n\r");
}
