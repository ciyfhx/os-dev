#ifndef INCLUDE_INTERRUPTS_H
#define INCLUDE_INTERRUPTS_H

#include "pic.hpp"
#include "interrupts_handler.hpp"

#define INTERRUPTS_DESCRIPTOR_COUNT 256 

#define INTERRUPT_HANDLER(IRQ) \
        extern "C" void CONCAT(interrupt_handler_, IRQ)(void)

#define INTERRUPT_HANDLER_FUNC(IRQ) CONCAT(interrupt_handler_, IRQ)

#define CONCAT(prefix, name) prefix##name

struct IDT {
    unsigned short size;
	unsigned int address;
} __attribute__((packed));

struct IDT_descriptor {
    unsigned short offset_low;
    unsigned short segment_selector;
    unsigned char reserved;
    unsigned char type_and_attr;
    unsigned short offset_high;
} __attribute__((packed));

struct cpu_state {
    unsigned int edi;
    unsigned int esi;
    unsigned int ebp;
    unsigned int esp;
    unsigned int ebx;
    unsigned int edx;
    unsigned int ecx;
    unsigned int eax;
} __attribute__((packed));

struct stack_state {
    unsigned int error_code;
    unsigned int eip;
    unsigned int cs;
    unsigned int eflags;
} __attribute__((packed));

typedef void (*interrupt_callback)();

extern "C" {
    void interrupts_handler(struct cpu_state cpu, unsigned int interrupt_code, struct stack_state stack);
}

void interrupts_install_idt();
void interrupts_init_descriptor(unsigned int index, unsigned int address);
void interrupts_register_callback(unsigned int index, interrupt_callback interrupt_callback);

#endif /* INCLUDE_INTERRUPTS_H */
    