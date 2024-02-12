#ifndef	_IRQ_H
#define	_IRQ_H

void enable_interrupt_controller( void );

/* functions below defined in irq.S */
void irq_vector_init( void );    
void enable_irq( void );         
void disable_irq( void );

#endif  /*_IRQ_H */
