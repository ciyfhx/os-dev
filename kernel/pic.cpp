#include "pic.hpp"

void pic_remap(int offset1, int offset2)
{
	// char a1, a2;
	
	// a1 = inb(PIC1_PORT_DATA);                        // save masks
	// a2 = inb(PIC2_PORT_DATA);
	
	outb(PIC1_PORT_COMMAND, ICW1_INIT | ICW1_ICW4);  // starts the initialization sequence (in cascade mode)
	io_wait();
	outb(PIC2_PORT_COMMAND, ICW1_INIT | ICW1_ICW4);
	io_wait();
	outb(PIC1_PORT_DATA, offset1);                 // ICW2: Master PIC vector offset
	io_wait();
	outb(PIC2_PORT_DATA, offset2);                 // ICW2: Slave PIC vector offset
	io_wait();
	outb(PIC1_PORT_DATA, 4);                       // ICW3: tell Master PIC that there is a slave PIC at IRQ2 (0000 0100)
	io_wait();
	outb(PIC2_PORT_DATA, 2);                       // ICW3: tell Slave PIC its cascade identity (0000 0010)
	io_wait();
	
	outb(PIC1_PORT_DATA, ICW4_8086);               // ICW4: have the PICs use 8086 mode (and not 8080 mode)
	io_wait();
	outb(PIC2_PORT_DATA, ICW4_8086);
	io_wait();
	
	outb(PIC1_PORT_DATA, 0xFD);   // Enable keyboard mask
	outb(PIC2_PORT_DATA, 0xFF);

	asm("sti"); // Enable interrupt
}


void pic_acknowledge(unsigned int interrupt)
{
    if (interrupt < PIC1_START_INTERRUPT || interrupt > PIC2_END_INTERRUPT) {
        return;
    }
    if (interrupt < PIC2_START_INTERRUPT) {
        outb(PIC1_PORT_COMMAND, PIC_ACK);
    } else {
        outb(PIC2_PORT_COMMAND, PIC_ACK);
    }
}

void pic_initialise(){
    //Remap the interrupt offset for the pic
    pic_remap(PIC1_START_INTERRUPT, PIC2_START_INTERRUPT);
}