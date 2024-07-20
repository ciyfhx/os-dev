#include "cursor.hpp"
#include "basic_io.hpp"
#include "serial.hpp"

extern "C" void main(){
    char msg[] = "Hello World from C++!";
    fb_write(msg);
    // serial_write(SERIAL_COM1_BASE, msg, sizeof(msg));
    return;
}
