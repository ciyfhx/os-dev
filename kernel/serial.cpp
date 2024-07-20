#include "serial.hpp"


void serial_configure_baud_rate(unsigned short com, unsigned short divisor)
{
    // Configure the LCR register to enable DLAB registers
    outb(SERIAL_LINE_COMMAND_PORT(com),
            SERIAL_LINE_ENABLE_DLAB);
    // Configure the divisor
    outb(SERIAL_DATA_PORT(com),
             divisor & 0x00FF);
    outb(SERIAL_DATA_PORT(com + 1),
            (divisor & 0xFF00) >> 8);
}
void serial_configure_line(unsigned short com) 
{
    // Configure the LCR register to 00000011
    // Divisor Latch Access Bit - 0
    // Set Break Enable - 0
    // Parity select - 000 (no parity)
    // Stop Bit select - 0 (one stop bit)
    // Word Length select - 11 (8 bits long)
    outb(SERIAL_LINE_COMMAND_PORT(com), 0x03);
}


void serial_configure_fifo_buffer(unsigned short com) 
{
    // Configure the FCR register to 11000111
    // Interrupt Trigger Level - 11
    // Enable 64 Byte FIFO - 0
    // Reserved - 0
    // DMA Mode Select - 0
    // Clear Transmit FIFO - 1
    // Clear Receive FIFO - 1
    // Enable FIFOs - 1
    outb(SERIAL_FIFO_COMMAND_PORT(com), 0xC7);
}


void serial_configure_modem(unsigned short com) 
{
    // Configure the MCR register to 00000011
    // Reserved - 00
    // Autoflow Control Enabled - 0
    // Loopback Mode - 0
    // Auxiliary Output 2 - 0
    // Auxiliary Output 1 - 0
    // Request To Send - 1
    // Data Terminal Ready - 1
    outb(SERIAL_MODEM_COMMAND_PORT(com), 0x03);
}

int serial_is_transmit_fifo_empty(unsigned int com)
{
    /* 0x20 = 0010 0000 */
    return inb(SERIAL_LINE_STATUS_PORT(com)) & 0x20;
}

void serial_write_byte(unsigned short port, char byteData) 
{
	outb(port, byteData);
}  

void serial_configure_port(unsigned short com, unsigned short baudRate)
{
    serial_configure_baud_rate(com, baudRate);
    serial_configure_line(com);
    serial_configure_fifo_buffer(com);
    serial_configure_modem(com);
}

void serial_write(unsigned short com, char *buf, unsigned int len)
{
	serial_configure_port(com , Baud_115200);
	unsigned int bufferIndex = 0;
	while (bufferIndex < len) {
		if (serial_is_transmit_fifo_empty(com)) {
			serial_write_byte(com, buf[bufferIndex]);
			bufferIndex++;
		}
	}
}