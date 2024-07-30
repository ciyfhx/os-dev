#ifndef INCLUDE_INTERRUPTS_HANDLER_H
#define INCLUDE_INTERRUPTS_HANDLER_H
    
extern "C" {
    /**
     * Load the IDT address
     */
    void load_idt(uint32_t idt_address);
}


#endif /* INCLUDE_INTERRUPTS_HANDLER_H */
    

