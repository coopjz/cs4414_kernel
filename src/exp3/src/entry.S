#include "entry.h"
#include "arm/sysregs.h"
#include "mm.h"
 	// **An exception vector** (or handler) is a piece of code the CPU will execute when a specific 
	//  exception happens. "*These would normally be branch instructions that direct the core to the full 
	//  exception handler.*" (the ARM64 manual). 

	//  The ARM64 hardware mandates: each exception vector can occupy `0x80` bytes maximum (thus `.align 7` in the asm code). 

	// For code clarity, we are not going to handle exceptions right inside the exception vector. 
	// Instead, we make each vector a branch instruction (`b \label`) that jumps to a label provided for the macro as `label` argument.

	// A default handler just to print out meaningful message in case our kernel triggers some unwanted exceptions due to our programming mistakes
	.macro handle_invalid_entry type
	kernel_entry
	// below: arguments to handle_invalid_entry are passed in 3 registers: x0, x1, and x2.
	// x0: the exception type. It can take one of the values defined in entry.h. It tells us exactly which exception handler has been executed
	mov	x0, #\type   
	// x1: what causes the exception. Value from `esr_el1` (Exception Syndrome Register). EL1 implies "when the exception is handled at EL1".
	mrs	x1, esr_el1
	// x2: the address of the instruction being executed when the exception happens. EL1 implies "when taking an exception to EL1, (this reg) holds the address to return to" 
	mrs	x2, elr_el1
	bl	show_invalid_entry_message
	b	err_hang
	.endm

	.macro handle_invalid_entry_EL2 type
	kernel_entry
	// below: arguments to handle_invalid_entry are passed in 3 registers: x0, x1, and x2.
	// x0: the exception type. It can take one of the values defined in entry.h. It tells us exactly which exception handler has been executed
	mov	x0, #\type   
	// x1: what causes the exception. Value from `esr_el1` (Exception Syndrome Register). EL1 implies "when the exception is handled at EL1".
	mrs	x1, esr_el2
	// x2: the address of the instruction being executed when the exception happens. EL1 implies "when taking an exception to EL1, (this reg) holds the address to return to" 
	mrs	x2, elr_el2
	bl	show_invalid_entry_message
	b	err_hang
	.endm

	.macro handle_invalid_entry_el1h type
	kernel_entry
	// below: arguments to handle_invalid_entry are passed in 3 registers: x0, x1, and x2.
	// x0: the exception type. It can take one of the values defined in entry.h. It tells us exactly which exception handler has been executed
	mov	x0, #\type   
	// x1: what causes the exception. Value from `esr_el1` (Exception Syndrome Register). EL1 implies "when the exception is handled at EL1".
	mrs	x1, esr_el1
	// x2: the address of the instruction being executed when the exception happens. EL1 implies "when taking an exception to EL1, (this reg) holds the address to return to" 
	mrs	x2, elr_el1
	bl	show_invalid_entry_message
	bl  show_EL1h_message
	// handle the exception in EL2
	bl	show_EL
	hvc #0
	kernel_exit

	
	.endm

	.macro	ventry	label
	.align	7	// We need `.align 7` because all exception vectors should be spaced at `0x80` bytes (2<<7) one from another
	b	\label
	.endm

	.macro	kernel_entry
	// The first thing to do in handling an exception: saving the processor state, notably registers x0 - x30, to the stack.
	// subtracts from `sp` the size of total stored registers (#S_FRAME_SIZE) ...
	sub	sp, sp, #S_FRAME_SIZE  
	// ... and then fills the stack space
	stp	x0, x1, [sp, #16 * 0]
	stp	x2, x3, [sp, #16 * 1]
	stp	x4, x5, [sp, #16 * 2]
	stp	x6, x7, [sp, #16 * 3]
	stp	x8, x9, [sp, #16 * 4]
	stp	x10, x11, [sp, #16 * 5]
	stp	x12, x13, [sp, #16 * 6]
	stp	x14, x15, [sp, #16 * 7]
	stp	x16, x17, [sp, #16 * 8]
	stp	x18, x19, [sp, #16 * 9]
	stp	x20, x21, [sp, #16 * 10]
	stp	x22, x23, [sp, #16 * 11]
	stp	x24, x25, [sp, #16 * 12]
	stp	x26, x27, [sp, #16 * 13]
	stp	x28, x29, [sp, #16 * 14]
	str	x30, [sp, #16 * 15] 
	.endm

	// A mirror of kernel_entry (above), to be called as the last thing of an exception handler
	.macro	kernel_exit
	// restores the CPU state by copying back the values of x0 - x30
	ldp	x0, x1, [sp, #16 * 0]
	ldp	x2, x3, [sp, #16 * 1]
	ldp	x4, x5, [sp, #16 * 2]
	ldp	x6, x7, [sp, #16 * 3]
	ldp	x8, x9, [sp, #16 * 4]
	ldp	x10, x11, [sp, #16 * 5]
	ldp	x12, x13, [sp, #16 * 6]
	ldp	x14, x15, [sp, #16 * 7]
	ldp	x16, x17, [sp, #16 * 8]
	ldp	x18, x19, [sp, #16 * 9]
	ldp	x20, x21, [sp, #16 * 10]
	ldp	x22, x23, [sp, #16 * 11]
	ldp	x24, x25, [sp, #16 * 12]
	ldp	x26, x27, [sp, #16 * 13]
	ldp	x28, x29, [sp, #16 * 14]
	ldr	x30, [sp, #16 * 15] 
	add	sp, sp, #S_FRAME_SIZE		
	eret	// returns to the interrupted execution! (e.g. still at EL1)
	.endm


	// **A vector table** is an array of exception vectors. Each exception level (EL) has its own vector table. 	 
	
/* 
	Below: **The vector table for EL1**. Purpose: to handle exceptions *taken from* EL0 (user programs) 
	or EL1 (the kernel's own execution). 
	``vectors'' below is set as vector table for EL1 by irq_vector_init 

	the kernel defines 16 exception handlers: 
		4 types [sync, irq, fiq, serror] for each of the CPU 4 execution states [EL1t, EL1h, EL0_64, EL0_32]. 
*/	
.align	11
.globl vectors 
vectors:
	//  EL1t -- Exception happens when CPU is at EL1 while the stack pointer (SP) was set to be shared with EL0
	ventry	sync_invalid_el1t			// Synchronous EL1t
	ventry	irq_invalid_el1t			// IRQ EL1t
	ventry	fiq_invalid_el1t			// FIQ EL1t
	ventry	error_invalid_el1t			// Error EL1t

	// EL1h -- Exception happens at EL1 at the time when a dedicated SP was allocated for EL1.
	//  		This is the mode that our kernel is currently using
	ventry	sync_invalid_el1h			// Synchronous EL1h   (also try ventry try_to_return)
	ventry	el1_irq						// IRQ EL1h
	ventry	fiq_invalid_el1h			// FIQ EL1h
	ventry	error_invalid_el1h			// Error EL1h

	// EL0_64 -- Exception is taken from EL0 executing in 64-bit mode. 
	//		The exceptions caused in 64-bit user programs
	ventry	sync_invalid_el0_64			// Synchronous 64-bit EL0
	ventry	irq_invalid_el0_64			// IRQ 64-bit EL0
	ventry	fiq_invalid_el0_64			// FIQ 64-bit EL0
	ventry	error_invalid_el0_64			// Error 64-bit EL0

	// EL0_32 -- Exception is taken from EL0 executing in 32-bit mode
	//		The exceptions caused in 32-bit user programs
	ventry	sync_invalid_el0_32			// Synchronous 32-bit EL0
	ventry	irq_invalid_el0_32			// IRQ 32-bit EL0
	ventry	fiq_invalid_el0_32			// FIQ 32-bit EL0
	ventry	error_invalid_el0_32			// Error 32-bit EL0

sync_invalid_el1t:
	handle_invalid_entry  SYNC_INVALID_EL1t

irq_invalid_el1t:
	handle_invalid_entry  IRQ_INVALID_EL1t

fiq_invalid_el1t:
	handle_invalid_entry  FIQ_INVALID_EL1t

error_invalid_el1t:
	handle_invalid_entry  ERROR_INVALID_EL1t

sync_invalid_el1h:
	handle_invalid_entry_el1h  SYNC_INVALID_EL1h

fiq_invalid_el1h:
	handle_invalid_entry  FIQ_INVALID_EL1h

error_invalid_el1h:
	handle_invalid_entry  ERROR_INVALID_EL1h

sync_invalid_el0_64:
	handle_invalid_entry  SYNC_INVALID_EL0_64

irq_invalid_el0_64:
	handle_invalid_entry  IRQ_INVALID_EL0_64

fiq_invalid_el0_64:
	handle_invalid_entry  FIQ_INVALID_EL0_64

error_invalid_el0_64:
	handle_invalid_entry  ERROR_INVALID_EL0_64

sync_invalid_el0_32:
	handle_invalid_entry  SYNC_INVALID_EL0_32

irq_invalid_el0_32:
	handle_invalid_entry  IRQ_INVALID_EL0_32

fiq_invalid_el0_32:
	handle_invalid_entry  FIQ_INVALID_EL0_32

error_invalid_el0_32:
	handle_invalid_entry  ERROR_INVALID_EL0_32

/* ---- end of EL1 vectors ----- */


/* 
   TODO: populate a vector table for EL2 here. 
   The format is the same as that of EL1, e.g. 16 (=4x4) exception handlers. 
   Note: EL2 vectors are for exceptions taken from EL2 and EL1
*/
 .align	11
.globl vectors_el2
vectors_el2:
	///* --- add yours ---  */
	ventry	sync_invalid_el2t			// Synchronous EL2t
	ventry	irq_invalid_el2t			// IRQ EL2t
	ventry	fiq_invalid_el2t			// FIQ EL2t
	ventry	error_invalid_el2t			// Error EL2t
//
	ventry	sync_invalid_el2h			// Synchronous EL2h
	ventry	irq_invalid_el2h			// IRQ EL2h
	ventry	fiq_invalid_el2h			// FIQ EL2h
	ventry	error_invalid_el2h			// Error EL2h
//
	ventry	sync_invalid_el1_64			// Synchronous 64-bit EL1
	ventry	irq_invalid_el1_64			// IRQ 64-bit EL1
	ventry	fiq_invalid_el1_64			// FIQ 64-bit EL1
	ventry	error_invalid_el1_64			// Error 64-bit EL1
//
	ventry	sync_invalid_el1_32			// Synchronous 32-bit EL1
	ventry	irq_invalid_el1_32			// IRQ 32-bit EL1
	ventry	fiq_invalid_el1_32			// FIQ 32-bit EL1
	ventry	error_invalid_el1_32			// Error 32-bit EL1 

sync_invalid_el2t:
	handle_invalid_entry_EL2  SYNC_INVALID_EL2t

irq_invalid_el2t:
	handle_invalid_entry_EL2  IRQ_INVALID_EL2t

fiq_invalid_el2t:
	handle_invalid_entry_EL2  FIQ_INVALID_EL2t

error_invalid_el2t:	
	handle_invalid_entry_EL2  ERROR_INVALID_EL2t

sync_invalid_el2h:
	b try_to_return


irq_invalid_el2h:
	handle_invalid_entry_EL2  IRQ_INVALID_EL2h

fiq_invalid_el2h:
	handle_invalid_entry_EL2  FIQ_INVALID_EL2h

error_invalid_el2h:	
	handle_invalid_entry_EL2  ERROR_INVALID_EL2h

sync_invalid_el1_64:
	eret
sync_invalid_el1_64_:
	mov	sp, #LOW_MEMORY
	sub	sp, sp, #S_FRAME_SIZE
	stp	x0, x1, [sp, #16 * 0]
	stp	x2, x3, [sp, #16 * 1]
	stp	x4, x5, [sp, #16 * 2]
	stp	x6, x7, [sp, #16 * 3]
	stp	x8, x9, [sp, #16 * 4]
	stp	x10, x11, [sp, #16 * 5]
	stp	x12, x13, [sp, #16 * 6]
	stp	x14, x15, [sp, #16 * 7]
	stp	x16, x17, [sp, #16 * 8]
	stp	x18, x19, [sp, #16 * 9]
	stp	x20, x21, [sp, #16 * 10]
	stp	x22, x23, [sp, #16 * 11]
	stp	x24, x25, [sp, #16 * 12]
	stp	x26, x27, [sp, #16 * 13]
	stp	x28, x29, [sp, #16 * 14]

	mrs	x21, sp_el1

	stp	x30, x21, [sp, #16 * 15] 
	stp	x22, x23, [sp, #16 * 16]
	bl show_EL

	mov x0, #SYNC_INVALID_EL1_64
	mrs	x1, esr_el1
	// x2: the address of the instruction being executed when the exception happens. EL1 implies "when taking an exception to EL1, (this reg) holds the address to return to" 
	mrs	x2, elr_el1
	bl	show_invalid_entry_message
/////
	ldp	x22, x23, [sp, #16 * 16]
	ldp	x30, x21, [sp, #16 * 15] 

	msr	sp_el1, x21

	ldp	x0, x1, [sp, #16 * 0]
	ldp	x2, x3, [sp, #16 * 1]
	ldp	x4, x5, [sp, #16 * 2]
	ldp	x6, x7, [sp, #16 * 3]
	ldp	x8, x9, [sp, #16 * 4]
	ldp	x10, x11, [sp, #16 * 5]
	ldp	x12, x13, [sp, #16 * 6]
	ldp	x14, x15, [sp, #16 * 7]
	ldp	x16, x17, [sp, #16 * 8]
	ldp	x18, x19, [sp, #16 * 9]
	ldp	x20, x21, [sp, #16 * 10]
	ldp	x22, x23, [sp, #16 * 11]
	ldp	x24, x25, [sp, #16 * 12]
	ldp	x26, x27, [sp, #16 * 13]
	ldp	x28, x29, [sp, #16 * 14]
	add	sp, sp, #S_FRAME_SIZE		
	eret
	

irq_invalid_el1_64:
	handle_invalid_entry_EL2  IRQ_INVALID_EL1_64

fiq_invalid_el1_64:
	handle_invalid_entry_EL2  FIQ_INVALID_EL1_64

error_invalid_el1_64:
	kernel_entry
	// below: arguments to handle_invalid_entry are passed in 3 registers: x0, x1, and x2.
	// x0: the exception type. It can take one of the values defined in entry.h. It tells us exactly which exception handler has been executed
	bl show_EL
	kernel_exit

sync_invalid_el1_32:
	handle_invalid_entry_EL2  SYNC_INVALID_EL1_32

irq_invalid_el1_32:
	handle_invalid_entry_EL2  IRQ_INVALID_EL1_32

fiq_invalid_el1_32:
	handle_invalid_entry_EL2  FIQ_INVALID_EL1_32

error_invalid_el1_32:
	handle_invalid_entry_EL2  ERROR_INVALID_EL1_32

el1_irq:
	kernel_entry 
	bl	handle_irq
	kernel_exit 


.globl err_hang
err_hang: b err_hang

try_to_return:
	kernel_entry
	mrs x1, elr_el2
	add x1, x1, 4
	msr elr_el2, x1
	kernel_exit

