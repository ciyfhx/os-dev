#ifndef INCLUDE_INTERRUPTS_H
#define INCLUDE_INTERRUPTS_H

#include <cstdint>
#include "pic.hpp"
#include "interrupts_handler.hpp"

#define INTERRUPTS_DESCRIPTOR_COUNT 256 

#define INTERRUPT_HANDLER(IRQ) \
        extern "C" void CONCAT(interrupt_handler_, IRQ)(void)

#define INTERRUPT_HANDLER_FUNC(IRQ) CONCAT(interrupt_handler_, IRQ)

#define CONCAT(prefix, name) prefix##name

struct IDT {
    uint16_t size;
	uint64_t address;
} __attribute__((packed));

// struct IDT_descriptor {
//     uint16_t offset_low;
//     uint16_t segment_selector;
//     uint8_t reserved;
//     uint8_t type_and_attr;
//     uint16_t offset_high;
// } __attribute__((packed));

typedef struct _IDT_descriptor
{
    union
    {
        struct
        {
            uint64_t OffsetLow : 16;           // A 64-bit value, split in three parts. It represents the address of the entry point of the Interrupt Service Routine.
            uint64_t SegmentSelector : 16;     // Selector: A Segment Selector with multiple fields which must point to a valid code segment in your GDT.
            uint64_t IST : 3;                  // A 3-bit value which is an offset into the Interrupt Stack Table, which is stored in the Task State Segment. If the bits are all set to zero, the Interrupt Stack Table is not used.
            uint64_t Reserved1 : 5;            // Unused
            uint64_t GateType : 4;             // A 4-bit value which defines the type of gate this Interrupt Descriptor represents. In long mode there are two valid type values: 0b1110 or 0xE: 64-bit Interrupt Gate | 0b1111 or 0xF: 64-bit Trap Gate
            uint64_t AlwaysZero : 1;           // This bit must be always zero
            uint64_t DPL : 2;                  //  A 2-bit value which defines the CPU Privilege Levels which are allowed to access this interrupt via the INT instruction. Hardware interrupts ignore this mechanism.
            uint64_t Present : 1;              // Present bit. Must be set (1) for the descriptor to be valid.
            uint64_t OffsetMiddle: 16;         // A 64-bit value, split in three parts. It represents the address of the entry point of the Interrupt Service Routine.
        };
        uint64_t ValueLow;
    };
    union
    {
        struct
        {
            uint64_t OffsetHigh : 32;         // A 64-bit value, split in three parts. It represents the address of the entry point of the Interrupt Service Routine.
            uint64_t Reserved2 : 32;          // Unused
        };
        uint64_t ValueHigh;
    };
} IDT_descriptor, *IDT_descriptor_p;

typedef void (*interrupt_callback)();

extern "C" {
    void interrupts_handler(uint64_t interrupt_code);
}

void interrupts_install_idt();
void interrupts_init_descriptor(std::size_t index, uint64_t address);
void interrupts_register_callback(std::size_t index, interrupt_callback interrupt_callback);

#endif /* INCLUDE_INTERRUPTS_H */
    