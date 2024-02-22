#include "sched.h"

.globl cpu_switch_to
/* the context switch magic */
cpu_switch_to:
	mov	x10, #THREAD_CPU_CONTEXT
	add	x8, x0, x10
	// now `x8` will contain a pointer to the current `cpu_context`
	mov	x9, sp
	
	// below: all callee-saved registers are stored in the order, in which they are defined in `cpu_context` structure
	stp	x19, x20, [x8], #16		
	stp	x21, x22, [x8], #16
	stp	x23, x24, [x8], #16
	stp	x25, x26, [x8], #16
	stp	x27, x28, [x8], #16
	stp	x29, x9, [x8], #16
	str	x30, [x8]			// save LR (x30) to cpu_context.pc, pointing to where this function is called from
	
	add	x8, x1, x10			// calculate the address of the next task's `cpu_contex`
	
	// below: restore the CPU context of "switch_to" task to CPU regs
	ldp	x19, x20, [x8], #16		
	ldp	x21, x22, [x8], #16
	ldp	x23, x24, [x8], #16
	ldp	x25, x26, [x8], #16
	ldp	x27, x28, [x8], #16
	ldp	x29, x9, [x8], #16
	ldr	x30, [x8]				// x30 == LR
	mov	sp, x9

	// The `ret` instruction will jump to the location pointed to by the link register (LR or `x30`). 
	// If we are switching to a task for the first time, this will be the beginning of the `ret_from_fork` function.
	// In all other cases this will be the location previously saved in the `cpu_context.pc` by the `cpu_switch_to` function
	ret							

