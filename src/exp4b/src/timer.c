#include "utils.h"
#include "printf.h"
#include "sched.h"
#include "peripherals/timer.h"
#include "timer.h"

#ifdef USE_QEMU
const int interval = (1 << 26); // xzl: around 1 sec
int interval_ms = interval/100000; // xzl: around 1 sec
#else
int interval = 1 * 1000 * 1000; // xzl: around 1 sec
#endif

/* 	These are for Arm generic timer. 
	They are fully functional on both QEMU and Rpi3 
	Recommended.
*/
void generic_timer_init ( void )
{
	printf("interval is set to: %u\r\n", interval/10);
	gen_timer_init();
	gen_timer_reset(interval/10);
}
int cnt = 0;

void handle_generic_timer_irq( void ) 
{
	// printf("Timer interrupt received\r\n");
	//Get current stack pointer
	unsigned long sp;
	sp = get_sp();
	// printf("stack pointer: %X\r\n", sp);
	//get current program counter
	unsigned long pc;
	pc = get_elr();
	current->pc_ = pc;
	current->sp_ = sp;
	// printf("program counter: %X\r\n", pc);
	switch_tracing_stack[stackcnt].time_stamp = get_time_ms();
	switch_tracing_stack[stackcnt].prev_pid = getpid();
	switch_tracing_stack[stackcnt].prev_pc = pc;
	switch_tracing_stack[stackcnt].prev_sp = sp;
	cnt+=100;
	printf("time elapsed in irq: %u ms\r\n", cnt);
	printf("time elapsed: %u ms\r\n", get_time_ms());
	gen_timer_reset(interval/10);
    timer_tick();
}
unsigned long get_time_ms(void)
{
	// unsigned long count = ((unsigned long)(get32(TIMER_CHI)) << 32) + get32(TIMER_CLO);
	// return count/interval_ms;
	unsigned long count = get_timer_count();
	unsigned long freq = get_timer_freq();
	freq = freq/1000;
	return count/freq;

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
	timer_tick();
}