#ifndef	_TIMER_H
#define	_TIMER_H

/* These are for "System Timer". See timer.c for details */
void timer_init ( void );
void handle_timer_irq ( void );

/* below are for Arm generic timers */
void generic_timer_init ( void );
void handle_generic_timer_irq ( void );

extern void gen_timer_init();
/* set timer to be fired after @interval System ticks */
extern void gen_timer_reset(int interval); 
extern unsigned long get_timer_interval();

extern struct task_struct * task_to_unblock;
extern int least_suspend_time;
#endif  /*_TIMER_H */
