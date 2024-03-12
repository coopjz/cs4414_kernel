#ifndef	_SYS_H
#define	_SYS_H

#define __NR_syscalls	    5

/* syscall number, which is just an index in @sys_call_table */
#define SYS_WRITE_NUMBER    0 		
#define SYS_MALLOC_NUMBER   1 	
#define SYS_CLONE_NUMBER    2 	
#define SYS_EXIT_NUMBER     3 	
#define SYS_GET_EL_NUMBER          4

#ifndef __ASSEMBLER__

void sys_write(char * buf);
int sys_fork();

/* call_sys_XXX functions are user-level (EL0) syscall entries. 
    implemented in sys.S as assembly */
void call_sys_write(char * buf);
int call_sys_clone(unsigned long fn, unsigned long arg, unsigned long stack);
unsigned long call_sys_malloc();
void call_sys_exit();
int call_sys_get_el();
int sys_get_el();

#endif
#endif  /*_SYS_H */
