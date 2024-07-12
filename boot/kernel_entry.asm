[bits 16]
protected_mode:
    cli ; clear interrupt flag
    lgdt [GDT_descriptor]
    mov eax, cr0
    or eax, 1
    mov cr0, eax                        ; enter 32-mode
    jmp CODE_SEG:start_protected_mode   ; far jump to protected section

[bits 32]
start_protected_mode:
    ; Set up segment registers
	mov ax, DATA_SEG
	mov ds, ax
	mov ss, ax

    ; Set up stack
    mov ebp, 0x90000
    mov esp, ebp
    
    jmp _main

_main:
    mov si, welcomeMsg 
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

CODE_SEG equ GDT_code - GDT_start
DATA_SEG equ GDT_data - GDT_start

                                    ; Global Descriptor Table (GPT)       
GDT_start:                          ; must be at the end of real mode code

    GDT_null:                       ; First Entry of the table will always be null 
        dd 0x0                      ; 32-bits of 0 
        dd 0x0                      ; 32-bits of 0 

    GDT_code:                       ; Second Entry (Kernal Code Segment)
        dw 0xffff                   ; Segment Limit (15:0)
        dw 0x0                      ; Base Address (15:0)
        db 0x0                      ; Base Address (23:16)
        db 0b10011010               ; Type (4-bits, Accessed Bit, Readable Bit, Conforming Bit, Code Bit = 1), System Segment Bit, Descriptor privilege level (2-Bits), Present Bit
        db 0b11001111               ; Segment Limit (19:16), Accessed Bit, Unused Bit, D/B Bit, Granularity Bit
        db 0x0                      ; Base Address (24:31)

    GDT_data:                       ; Third Entry (Kernal Data Segment)
        dw 0xffff                   ; Segment Limit (15:0)
        dw 0x0                      ; Base Address (15:0)
        db 0x0                      ; Base Address (23:16)
        db 0b10010010               ; Type (4-bits, Accessed Bit, Writable Bit, Expansion Direction Bit, Data Bit = 0), System Segment Bit, Descriptor privilege level (2-Bits), Present Bit
        db 0b11001111               ; Segment Limit (19:16), Accessed Bit, Unused Bit, Default/Big Bit, Granularity Bit
        db 0x0                      ; Base Address (24:31)

GDT_end:

GDT_descriptor:
    dw GDT_end - GDT_start - 1
    dd GDT_start
DATA:
    welcomeMsg db "Welcome, to Zi Heng's OS", 0