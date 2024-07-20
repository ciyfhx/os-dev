#ifndef INCLUDE_IO_H
#define INCLUDE_IO_H
    
extern "C" {
    /** outb:
     *  Sends the given data to the given I/O port. Defined in io.s
     *
     *  @param port The I/O port to send the data to
     *  @param data The data to send to the I/O port
     */
    void outb(unsigned short port, unsigned char data);

    /** inb:
     *  Read a single byte from the I/O port.
     *
     *  @param port The I/O port to read the data from
     *  @return     The read byte from the I/O port
     */
    unsigned char inb(unsigned short port);
}

#endif /* INCLUDE_IO_H */
    