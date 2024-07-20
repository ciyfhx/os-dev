
#ifndef INCLUDE_SERIAL_H
#define INCLUDE_SERIAL_H

#include "io.hpp"

/* The I/O ports */

/* All the I/O ports are calculated relative to the data port. This is because
* all serial ports (COM1, COM2, COM3, COM4) have their ports in the same
* order, but they start at different values.
*/

#define SERIAL_COM1_BASE                0x3F8      /* COM1 base port */

#define SERIAL_DATA_PORT(base)          (base)
#define SERIAL_FIFO_COMMAND_PORT(base)  (base + 2)
#define SERIAL_LINE_COMMAND_PORT(base)  (base + 3)
#define SERIAL_MODEM_COMMAND_PORT(base) (base + 4)
#define SERIAL_LINE_STATUS_PORT(base)   (base + 5)

/* The I/O port commands */

/* SERIAL_LINE_ENABLE_DLAB:
    * Tells the serial port to expect first the highest 8 bits on the data port,
    * then the lowest 8 bits will follow
    */
#define SERIAL_LINE_ENABLE_DLAB         0x80

/**
 * Define the standard baud rate use for COM
 */
enum BaudRate { Baud_115200 = 1, Baud_57600, Baud_19200, Baud_9600 };
// enum BaudRate divisor = Baud_115200;

/** 
*  serial_configure_baud_rate:
*  Sets the speed of the data being sent. The default speed of a serial
*  port is 115200 bits/s. The argument is a divisor of that number, hence
*  the resulting speed becomes (115200 / divisor) bits/s.
*
*  @param com      The COM port to configure
*  @param divisor  The divisor
*/
void serial_configure_baud_rate(unsigned short com, unsigned short divisor);

/** 
*  serial_configure_fifo_buffer:
*  Configure the fifo buffer.
*  @param com      The COM port to configure
*/
void serial_configure_fifo_buffer(unsigned short com);

/** 
*  serial_configure_modem:
*  Configure the modem.
*  @param com      The COM port to configure
*/
void serial_configure_modem(unsigned short com);

/**  
* serial_is_transmit_fifo_empty:
*  Checks whether the transmit FIFO queue is empty or not for the given COM
*  port.
*
*  @param  com The COM port
*  @return 0 if the transmit FIFO queue is not empty
*          1 if the transmit FIFO queue is empty
*/
int serial_is_transmit_fifo_empty(unsigned int com);

/**
 * serial_configure_port:
 * Configure the COM port
 * @param com       The COM port to configure
 * @param baudRate  The baud rate for the COM port
 */
void serial_configure_port(unsigned short com, unsigned short baudRate);

/**
 * serial_is_transmit_fifo_empty:
 * Check if the transmitting fifo buffer is empty
 * @param com       The COM port to check if the fifo is empty
 * @return          returns 1 if the fifo is empty else 0
 */
int serial_is_transmit_fifo_empty(unsigned int com);

/**
 * serial_write_byte:
 * Write a single byte to the COM port
 * @param com       The COM port to write
 * @param byteData  The byte data to write to the COM port
 */
void serial_write_byte(unsigned short port, char byteData);

/**
 * serial_write:
 * Write a buffer to a COM port
 * @param com       The COM port to write the buffer
 * @param buf       The buffer array to write to the COM port
 * @param len       The length of the buffer array
 */
void serial_write(unsigned short com, char *buf, unsigned int len);

#endif /* INCLUDE_SERIAL_H */
    