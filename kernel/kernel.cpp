#include "kernel.hpp"

extern "C" int main(){
    //Init PIC 
    pic_initialise();

    char msg[] = "Hello World from C++!";
    fb_write(msg);
    // serial_write(SERIAL_COM1_BASE, msg, sizeof(msg));

    //Init interrupt handler
    register_keyboard_interrupt();
    interrupts_install_idt();

    //Read test
    // uint8_t buf[512];
    // ata_lba_read(0, 1, buf);

    return 0;
}

