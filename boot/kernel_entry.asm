global _start
[bits 16]
_start:
    cli ; clear interrupt flag
    xor     ax, ax ; reset segment registers to match kernel location
    mov     ds, ax
    mov     es, ax
    mov     fs, ax
    mov     gs, ax

    ; Setup the GDT
    lgdt [GDT_descriptor]
    mov eax, cr0
    or eax, 1
    mov cr0, eax                        ; enter 32-mode
    jmp CODE_SEG:protected_mode         ; far jump to protected section

[bits 32]
protected_mode:
    ; Set up segment registers
	mov ax, DATA_SEG
	mov ds, ax
	mov ss, ax
    mov es, ax
    mov fs, ax
    mov gs, ax

    ; Set up stack
    mov ebp, 0x90000
    mov esp, ebp

    jmp _main
    
paging:
    

_main:
    ; mov si, welcomeMsg 
    ; call print
    ; push byte 'A'
    ; call write_char
    [extern main]
    call main

    jmp $
    ; hlt


write_char:
    mov edi, 0xb8000
    push ebp
    mov ebp, esp
    mov ah, 0x0F
    mov al, [ebp+8]
    mov [edi], ax
    pop ebp
    ret 4

; print:
; 	mov edi, 0xb8000
; .repeat:
; 	lodsb
; 	or al, al
; 	jz .done
; 	mov byte [edi], al
; 	inc edi
; 	mov byte [edi], 0x0F
; 	inc edi
; 	call .repeat
; .done:
; 	ret

CODE_SEG equ GDT_kernel_code - GDT_start
DATA_SEG equ GDT_kernel_data - GDT_start

                                    ; Global Descriptor Table (GDT)       
GDT_start:                          ; must be at the end of real mode code

    GDT_null:                       ; First Entry of the table will always be null 
        dd 0x0                      ; 32-bits of 0 
        dd 0x0                      ; 32-bits of 0 

    GDT_kernel_code:                ; Second Entry (Kernal Code Segment) Base = 0, Limit = 0xFFFFF, Access = 1001 1010, Flags = 1100
        dw 0xffff                   ; Segment Limit (15:0)
        dw 0x0                      ; Base Address (15:0)
        db 0x0                      ; Base Address (23:16)
        db 0b10011010               ; Present Bit, Descriptor privilege level (2-Bits), System Segment Bit, Type (4-bits, Code Bit = 1, Conforming Bit, Readable Bit, Accessed Bit)
        db 0b11001111               ; Granularity Bit, Default/Big Bit, Unused Bit, Accessed Bit, Segment Limit (19:16)
        db 0x0                      ; Base Address (24:31)

    GDT_kernel_data:                ; Third Entry (Kernal Data Segment) Base = 0, Limit = 0xFFFFF, Access = 1001 0010 , Flags = 1100
        dw 0xffff                   ; Segment Limit (15:0)
        dw 0x0                      ; Base Address (15:0)
        db 0x0                      ; Base Address (23:16)
        db 0b10010010               ; Present Bit, Descriptor privilege level (2-Bits), System Segment Bit, Type (4-bits, Data Bit = 0, Expansion Direction Bit, Writable Bit, Accessed Bit)
        db 0b11001111               ; Granularity Bit, Default/Big Bit, Unused Bit, Accessed Bit, Segment Limit (19:16)
        db 0x0                      ; Base Address (24:31)

    ; GDT_user_code:                  ; Fourth Entry (User Code Segment) Base = 0, Limit = 0xFFFFF, Access = 1111 1010, Flags = 1100
    ;     dw 0xffff                   ; Segment Limit (15:0)
    ;     dw 0x0                      ; Base Address (15:0)
    ;     db 0x0                      ; Base Address (23:16)
    ;     db 0b11111010               ; Present Bit, Descriptor privilege level (2-Bits), System Segment Bit, Type (4-bits, Code Bit = 1, Conforming Bit, Readable Bit, Accessed Bit)
    ;     db 0b11001111               ; Granularity Bit, Default/Big Bit, Unused Bit, Accessed Bit, Segment Limit (19:16)
    ;     db 0x0                      ; Base Address (24:31)

    ; ; GDT_user_data:                  ; Fifth Entry (User Data Segment) Base = 0, Limit = 0xFFFFF, Access = 1111 0010 , Flags = 1100
    ;     dw 0xffff                   ; Segment Limit (15:0)
    ;     dw 0x0                      ; Base Address (15:0)
    ;     db 0x0                      ; Base Address (23:16)
    ;     db 0b11110010               ; Present Bit, Descriptor privilege level (2-Bits), System Segment Bit, Type (4-bits, Data Bit = 0, Expansion Direction Bit, Writable Bit, Accessed Bit)
    ;     db 0b11001111               ; Granularity Bit, Default/Big Bit, Unused Bit, Accessed Bit, Segment Limit (19:16)
    ;     db 0x0                      ; Base Address (24:31)

GDT_end:

GDT_descriptor:
    dw GDT_end - GDT_start - 1
    dd GDT_start

DATA:
    welcomeMsg db "Welcome, to Zi Heng's OS", 0