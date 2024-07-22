extern interrupts_handler

%macro interrupt_handler_no_error_code 1
global interrupt_handler_%1
interrupt_handler_%1:
    push dword 0    ; push a 0 value to the stack to act as an error code

    push dword %1   ; push the interrupt number
    jmp common_interrupt_handler ; jump to the common handler
%endmacro

%macro interrupt_handler_with_error_code 1
global interrupt_handler_%1
interrupt_handler_%1:
    push dword %1                ; push the interrupt number
    jmp common_interrupt_handler ; jump to the common handler
%endmacro

common_interrupt_handler: ; the common parts of the generic interrupt handler
    ; save the registers
    pusha
    ; call the C function
    call interrupts_handler
    ; restore the registers
    popa
    ; restore the esp (remove the error code)
    add esp, 8
    ; return to the code that got interrupted
    iret

interrupt_handler_no_error_code 1

global load_idt

; load_idt - Loads the interrupt descriptor table (IDT).
; stack: [esp + 4] the address of the first entry in the IDT
; [esp ] the return address
load_idt:
    mov eax, [esp + 4]
    lidt [eax]
    ret

; interrupt handlers
interrupt_handler_no_error_code 33 ; keyboard