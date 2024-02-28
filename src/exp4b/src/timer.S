/** Here, the physical timer at EL1 is used with the TimerValue views.
 *  Once the count-down has reach 0, the interrupt line is HIGH until
 *  a new timer value > 0 is write into the CNTP_TVAL_EL0 system register.
 *
 *  See AArch64-referenc-manual p.2326 at
 *  https://developer.arm.com/docs/ddi0487/ca/arm-architecture-reference-manual-armv8-for-armv8-a-architecture-profile
 */

.globl gen_timer_init
gen_timer_init:
	mov x0, #1
	msr CNTP_CTL_EL0, x0
	ret

.globl gen_timer_reset
gen_timer_reset:
	msr CNTP_TVAL_EL0, x0
    ret

.globl get_timer_count
get_timer_count:
	mrs x0, CNTPCT_EL0
	ret

.globl get_timer_freq
get_timer_freq:
	mrs x0, CNTFRQ_EL0
	ret