/** Here, the physical timer at EL1 is used with the TimerValue views.
 *  Once the count-down reaches 0, the interrupt line is HIGH until
 *  a new timer value > 0 is written into the CNTP_TVAL_EL0 system register.
 *
 *  Read: 
 *  https://fxlin.github.io/p1-kernel/exp3/rpi-os/#arms-generic-hardware-timer
 * 
 *  Reference: AArch64-referenc-manual p.2326 at
 *  https://developer.arm.com/docs/ddi0487/ca/arm-architecture-reference-manual-armv8-for-armv8-a-architecture-profile
 */

.globl gen_timer_init
gen_timer_init:
# Below, writes 1 to the control register (CNTP_CTL_EL0) of the EL1 physical timer
	# Explanation: 
	# 		CTL indicates this is a control register
	#		CNTP_XXX_EL0 indicates that this is for the EL1 physical timer
	#		(Why named _EL0? I guess it means that the timer is accessible to both EL1 and EL0)
	mov x0, #1
	msr CNTP_CTL_EL0, x0	// writes 1 to the control register (`CNTP_CTL_EL0`) of **the EL1 physical timer**
	ret

.globl gen_timer_reset
gen_timer_reset:
// Take the 1st arugment (passed in x0) as the interval before next firing of the timer. 
// alternatively, the two lines below demonstrate how to set a fixed interval (1<<24 ticks). 
//   mov x0, #1
//	 lsl x0, x0, #24
	msr CNTP_TVAL_EL0, x0
    ret
