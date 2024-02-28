#include "printf.h"
#include "utils.h"
#include "timer.h"
#include "irq.h"
#include "fork.h"
#include "sched.h"
#include "mini_uart.h"

void process(char *array)
{
	while (1) {
		for (int i = 0; i < 5; i++){
			// uart_send(array[i]);
			delay(5000000);
		}
	}
}

void process2(char *array)
{
	while (1) {
		for (int i = 0; i < 5; i++){
			// uart_send(array[i]);
			delay(5000000);
		}
		
	}
}
void process3(char *array)
{
	while (1) {
		// for (int i = 0; i < 5; i++){
		// 	// uart_send(array[i]);
		// 	delay(500000);
		// }
		// delay(1);
		asm("nop");
		
	}
}
void process4(char *array)
{
	while (1) {
		// for (int i = 0; i < 5; i++){
		// 	// uart_send(array[i]);
		// 	delay(500000);
		// }
		// delay(1);
		asm("nop");
	}
}


void kernel_main(void)
{
	uart_init();
	init_printf(0, putc);

	printf("kernel boots\n");

	irq_vector_init();
	generic_timer_init();
	enable_interrupt_controller();
	enable_irq();

	int res = copy_process((unsigned long)&process, (unsigned long)"task1");
	if (res != 0) {
		printf("error while starting process 1");
		return;
	}
	res = copy_process((unsigned long)&process2, (unsigned long)"task2");
	if (res != 0) {
		printf("error while starting process 2");
		return;
	}
	res = copy_process((unsigned long)&process3, (unsigned long)"task3");
	if (res != 0) {
		printf("error while starting process 3");
		return;
	}
	res = copy_process((unsigned long)&process4, (unsigned long)"task4");
	if (res != 0) {
		printf("error while starting process 4");
		return;
	}
	while (1){
		schedule();
	}	
}
