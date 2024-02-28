.globl get_el
get_el:
	mrs x0, CurrentEL
	lsr x0, x0, #2
	ret

.globl put32
put32:
	str w1,[x0]
	ret

.globl get32
get32:
	ldr w0,[x0]
	ret

.globl get_elr
get_elr:
	// ldr	x23, [sp, #16 * 16]
	// ldr x0,[x23]
	mrs x0, ELR_EL1
	ret
.globl get_sp
get_sp:
	mov x0, SP
	ret
.globl delay
delay:
	subs x0, x0, #1
	bne delay
	ret
