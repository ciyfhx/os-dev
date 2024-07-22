#include "interrupts.hpp"

interrupt_callback interrupt_callbacks[INTERRUPTS_DESCRIPTOR_COUNT];

struct IDT_descriptor idt_descriptors[INTERRUPTS_DESCRIPTOR_COUNT];
struct IDT idt;

void interrupts_install_idt()
{
	idt.address = (int) &idt_descriptors;
	idt.size = sizeof(struct IDT_descriptor) * INTERRUPTS_DESCRIPTOR_COUNT;
	load_idt((int) &idt);
}

void interrupts_init_descriptor(unsigned int index, unsigned int address)
{
	idt_descriptors[index].offset_high = (address >> 16) & 0xFFFF; // offset bits 0..15
	idt_descriptors[index].offset_low = (address & 0xFFFF); // offset bits 16..31

	idt_descriptors[index].segment_selector = 0x08; // The second (code) segment selector in GDT: one segment is 64b.
	idt_descriptors[index].reserved = 0x00; // Reserved.

	idt_descriptors[index].type_and_attr =	(0x01 << 7) |			// P
						(0x00 << 6) | (0x00 << 5) |	// DPL
						0xe;				// 0b1110=0xE 32-bit interrupt gate
}


void interrupts_handler(struct cpu_state cpu, unsigned int interrupt_code, struct stack_state stack){
	if(interrupt_callbacks[interrupt_code] != 0)interrupt_callbacks[interrupt_code]();
}

void interrupts_register_callback(unsigned int index, interrupt_callback interrupt_callback){
	interrupt_callbacks[index] = interrupt_callback;
}