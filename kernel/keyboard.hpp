#ifndef INCLUDE_KEYBOARD_H
#define INCLUDE_KEYBOARD_H
    
#include "io.hpp"
#include "basic_io.hpp"
#include "pic.hpp"
#include "interrupts.hpp"

#define KBD_IRQ 33
#define KBD_DATA_PORT 0x60

INTERRUPT_HANDLER(KBD_IRQ);


/** read_scan_code:
* Reads a scan code from the keyboard
*
* @return The scan code (NOT an ASCII character!)
*/
unsigned char read_scan_code(void);

void register_keyboard_interrupt();


#endif /* INCLUDE_KEYBOARD_H */
    