#ifndef INCLUDE_PIC_H
#define INCLUDE_PIC_H

#include "io.hpp"

#define PIC1_PORT_COMMAND 0x20
#define PIC1_PORT_DATA PIC1_PORT_COMMAND + 1
#define PIC2_PORT_COMMAND 0xA0
#define PIC2_PORT_DATA PIC2_PORT_COMMAND + 1
/* The PIC interrupts have been remapped */
#define PIC1_START_INTERRUPT 0x20
#define PIC2_START_INTERRUPT 0x28
#define PIC2_END_INTERRUPT PIC2_START_INTERRUPT + 7
#define PIC_ACK 0x20


/* reinitialize the PIC controllers, giving them specified vector offsets
   rather than 8h and 70h, as configured by default */

#define ICW1_ICW4	0x01		/* Indicates that ICW4 will be present */
#define ICW1_SINGLE	0x02		/* Single (cascade) mode */
#define ICW1_INTERVAL4	0x04		/* Call address interval 4 (8) */
#define ICW1_LEVEL	0x08		/* Level triggered (edge) mode */
#define ICW1_INIT	0x10		/* Initialization - required! */

#define ICW4_8086	0x01		/* 8086/88 (MCS-80/85) mode */
#define ICW4_AUTO	0x02		/* Auto (normal) EOI */
#define ICW4_BUF_SLAVE	0x08		/* Buffered mode/slave */
#define ICW4_BUF_MASTER	0x0C		/* Buffered mode/master */
#define ICW4_SFNM	0x10		/* Special fully nested (not) */

/**
* pic_remap:
* @param offset1 - vector offset for master PIC vectors on the master become offset1..offset1+7
* @param offset2 - same for slave PIC: offset2..offset2+7
*/
void pic_remap(int offset1, int offset2);
/** pic_acknowledge:
* Acknowledges an interrupt from either PIC 1 or PIC 2.
*
* @param num The number of the interrupt
*/
void pic_acknowledge(unsigned int interrupt);
/**
 * Remap the pic to unreserved CPU IRQ
 */
void pic_initialise();


#endif /* INCLUDE_PIC_H */
    