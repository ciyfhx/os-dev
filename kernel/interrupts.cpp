#include "interrupts.hpp"

interrupt_callback interrupt_callbacks[INTERRUPTS_DESCRIPTOR_COUNT];

IDT_descriptor idt_descriptors[INTERRUPTS_DESCRIPTOR_COUNT];
IDT idt;

void interrupts_install_idt()
{
	idt.address = (uint64_t) &idt_descriptors;
	idt.size = sizeof(IDT_descriptor) * INTERRUPTS_DESCRIPTOR_COUNT;
	load_idt((uint64_t) &idt);
}

void interrupts_init_descriptor(std::size_t index, uint64_t address)
{
	idt_descriptors[index].OffsetHigh = (address >> 32) & 0xFFFFFFFF; // offset bits 63..32
	idt_descriptors[index].OffsetMiddle = (address >> 16) & 0xFFFF; // offset bits 31..16
	idt_descriptors[index].OffsetLow = (address & 0xFFFF);  // offset bits 15..0
	idt_descriptors[index].AlwaysZero = 0;
	idt_descriptors[index].Reserved1 = 0;
	idt_descriptors[index].Reserved2 = 0;
	idt_descriptors[index].SegmentSelector = 0x08; // The second (code) segment selector in GDT: one segment is 128b.
	idt_descriptors[index].Present = 1;
	idt_descriptors[index].DPL = 0b00;
	idt_descriptors[index].GateType = 0xE; 
	idt_descriptors[index].IST = 0;

}


void interrupts_handler(uint64_t interrupt_code){
	if(interrupt_callbacks[interrupt_code] != 0)interrupt_callbacks[interrupt_code]();
}

void interrupts_register_callback(std::size_t index, interrupt_callback interrupt_callback){
	interrupt_callbacks[index] = interrupt_callback;
}