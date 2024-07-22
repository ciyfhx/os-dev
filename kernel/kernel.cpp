#include "kernel.hpp"

extern "C" int main(){
    char msg[] = "Hello World from C++!";
    fb_write(msg);
    serial_write(SERIAL_COM1_BASE, msg, sizeof(msg));

    //Init PIC 
    pic_initialise();

    //Init interrupt handler
    register_keyboard_interrupt();
    interrupts_install_idt();

    return 0;
}

