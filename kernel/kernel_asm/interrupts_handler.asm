[bits 64]
extern interrupts_handler

%macro pushaq 0
    push rax
    push rcx
    push rdx
    push rbx
    push rbp
    push rsi
    push rdi
%endmacro # pushaq

%macro popaq 0
    pop rdi
    pop rsi
    pop rbp
    pop rbx
    pop rdx
    pop rcx
    pop rax
%endmacro # popaq

%macro interrupt_handler_no_error_code 1
global interrupt_handler_%1
interrupt_handler_%1:
    push qword 0    ; push a 0 value to the stack to act as an error code

    mov rdi, %1     ; pass the interrupt number
    jmp common_interrupt_handler ; jump to the common handler
%endmacro

%macro interrupt_handler_with_error_code 1
global interrupt_handler_%1
interrupt_handler_%1:
    mov rdi, %1     ; pass the interrupt number
    jmp common_interrupt_handler ; jump to the common handler
%endmacro

common_interrupt_handler: ; the common parts of the generic interrupt handler
    ; save the registers
    ;pushaq
    ; call the C function
    call interrupts_handler
    ; restore the registers
    ;popaq
    ; restore the rsp (remove the error code)
    add rsp, 8
    ; return to the code that got interrupted
    iretq

interrupt_handler_no_error_code 1

global load_idt

; load_idt - Loads the interrupt descriptor table (IDT).
; params: [rdi] the address of the first entry in the IDT
load_idt:
    lidt [rdi]
    ret

; interrupt handlers
interrupt_handler_no_error_code 33 ; keyboard