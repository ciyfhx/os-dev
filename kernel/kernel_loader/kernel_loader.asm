KERNEL_LOCATION equ 0x00000000C0100000

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
    lgdt [GDT32.GDT_descriptor]
    mov eax, cr0
    or eax, 1
    mov cr0, eax                                      ; enter 32-mode
    jmp GDT32.GDT_kernel_code:protected_mode            ; far jump to protected section

[bits 32]
protected_mode:
    ; Set up segment registers
	mov ax, GDT32.GDT_kernel_data
	mov ds, ax
	mov ss, ax
    mov es, ax
    mov fs, ax
    mov gs, ax

    ; Set up stack
    mov ebp, 0x90000
    mov esp, ebp


    ; read kernel from disk
    [extern kernel_loader_main]
    call kernel_loader_main

    call check_cpu_id
    call check_long_mode

    ; setup temporary paging to enable long mode (the actual paging will be setup by the kernel)
    ; mov eax, 0x1A0000
    ; mov cr3, eax
    mov eax, cr4                 ; Set the A-register to control register 4.
    or eax, 1 << 5               ; Set the PAE-bit, which is the 6th bit (bit 5).
    mov cr4, eax                 ; Set control register 4 to the A-register.

    mov ecx, 0xC0000080          ; Set the C-register to 0xC0000080, which is the EFER MSR.
    rdmsr                        ; Read from the model-specific register.
    or eax, 1 << 8               ; Set the LM-bit which is the 9th bit (bit 8).
    wrmsr                        ; Write to the model-specific register.
    mov eax, cr0                 ; Set the A-register to control register 0.
    or eax, 1 << 31              ; Set the PG-bit, which is the 32nd bit (bit 31).
    mov cr0, eax                 ; Set control register 0 to the A-register.
    
    lgdt [GDT.GDT_descriptor]
    jmp GDT.GDT_kernel_code:_main

check_long_mode:
    mov eax, 0x80000000    ; Set the A-register to 0x80000000.
    cpuid                  ; CPU identification.
    cmp eax, 0x80000001    ; Compare the A-register with 0x80000001.
    jb .NoLongMode         ; It is less, there is no long mode.
    mov eax, 0x80000001    ; Set the A-register to 0x80000001.
    cpuid                  ; CPU identification.
    test edx, 1 << 29      ; Test if the LM-bit, which is bit 29, is set in the D-register.
    jz .NoLongMode         ; They aren't, there is no long mode.
    ret
.NoLongMode:
    mov si, no_long_mode_msg
    call print
    jmp $

check_cpu_id:
    ; Check if CPUID is supported by attempting to flip the ID bit (bit 21) in
    ; the FLAGS register. If we can flip it, CPUID is available.

    ; Copy FLAGS in to EAX via stack
    pushfd
    pop eax

    ; Copy to ECX as well for comparing later on
    mov ecx, eax

    ; Flip the ID bit
    xor eax, 1 << 21

    ; Copy EAX to FLAGS via the stack
    push eax
    popfd

    ; Copy FLAGS back to EAX (with the flipped bit if CPUID is supported)
    pushfd
    pop eax

    ; Restore FLAGS from the old version stored in ECX (i.e. flipping the ID bit
    ; back if it was ever flipped).
    push ecx
    popfd

    ; Compare EAX and ECX. If they are equal then that means the bit wasn't
    ; flipped, and CPUID isn't supported.
    xor eax, ecx
    jz .NoCPUID
    ret
.NoCPUID:
    mov si, no_cpu_id_msg
    call print
    jmp $

    
[bits 64]
_main:
    mov rax, qword KERNEL_LOCATION
    jmp rax
    jmp $
    ; hlt


print:
	mov edi, 0xb8000
.repeat:
	lodsb
	or al, al
	jz .done
	mov byte [edi], al
	inc edi
	mov byte [edi], 0x0F
	inc edi
	call .repeat
.done:
	ret

;=============================================================================
; ATA read sectors (LBA mode) 
;
; @param EAX Logical Block Address of sector
; @param CL  Number of sectors to read
; @param RDI The address of buffer to put data obtained from disk
;
; @return None
;=============================================================================
_ata_lba_read:
               pushfq
               and rax, 0x0FFFFFFF
               push rax
               push rbx
               push rcx
               push rdx
               push rdi

               mov rbx, rax         ; Save LBA in RBX
               
               mov edx, 0x01F6      ; Port to send drive and bit 24 - 27 of LBA
               shr eax, 24          ; Get bit 24 - 27 in al
               or al, 11100000b     ; Set bit 6 in al for LBA mode
               out dx, al

               mov edx, 0x01F2      ; Port to send number of sectors
               mov al, cl           ; Get number of sectors from CL
               out dx, al
               
               mov edx, 0x1F3       ; Port to send bit 0 - 7 of LBA
               mov eax, ebx         ; Get LBA from EBX
               out dx, al

               mov edx, 0x1F4       ; Port to send bit 8 - 15 of LBA
               mov eax, ebx         ; Get LBA from EBX
               shr eax, 8           ; Get bit 8 - 15 in AL
               out dx, al


               mov edx, 0x1F5       ; Port to send bit 16 - 23 of LBA
               mov eax, ebx         ; Get LBA from EBX
               shr eax, 16          ; Get bit 16 - 23 in AL
               out dx, al

               mov edx, 0x1F7       ; Command port
               mov al, 0x20         ; Read with retry.
               out dx, al

.still_going:  in al, dx
               test al, 8           ; the sector buffer requires servicing.
               jz .still_going      ; until the sector buffer is ready.

               mov rax, 256         ; to read 256 words = 1 sector
               xor bx, bx
               mov bl, cl           ; read CL sectors
               mul bx
               mov rcx, rax         ; RCX is counter for INSW
               mov rdx, 0x1F0       ; Data port, in and out
               rep insw             ; in to [RDI]

               pop rdi
               pop rdx
               pop rcx
               pop rbx
               pop rax
               popfq
               ret

                                    ; Global Descriptor Table (GDT)  32     
GDT32:                          ; must be at the end of real mode code

    .GDT_null: equ $ - GDT32        ; First Entry of the table will always be null 
        dq 0x0                      ; 64-bits of 0

    .GDT_kernel_code: equ $ - GDT32 ; Second Entry (Kernal Code Segment) Base = 0, Limit = 0xFFFFF, Access = 1001 1010, Flags = 1100
        dw 0xffff                   ; Segment Limit (15:0)
        dw 0x0                      ; Base Address (15:0)
        db 0x0                      ; Base Address (23:16)
        db 0b10011010               ; Present Bit, Descriptor privilege level (2-Bits), System Segment Bit, Type (4-bits, Code Bit = 1, Conforming Bit, Readable Bit, Accessed Bit)
        db 0b11001111               ; Granularity Bit, Default/Big Bit, Long Mode Bit, Unused Bit, Segment Limit (19:16)
        db 0x0                      ; Base Address (24:31)

    .GDT_kernel_data: equ $ - GDT32 ; Third Entry (Kernal Data Segment) Base = 0, Limit = 0xFFFFF, Access = 1001 0010 , Flags = 1100
        dw 0xffff                   ; Segment Limit (15:0)
        dw 0x0                      ; Base Address (15:0)
        db 0x0                      ; Base Address (23:16)
        db 0b10010010               ; Present Bit, Descriptor privilege level (2-Bits), System Segment Bit, Type (4-bits, Data Bit = 0, Expansion Direction Bit, Writable Bit, Accessed Bit)
        db 0b11001111               ; Granularity Bit, Default/Big Bit, Long Mode Bit, Unused Bit, Segment Limit (19:16)
        db 0x0                      ; Base Address (24:31)
    .GDT_descriptor:
        dw $ - GDT32 - 1
        dd GDT32

; Global Descriptor Table (GDT)  64     
GDT:

    .GDT_null: equ $ - GDT          ; First Entry of the table will always be null 
        dq 0x0                      ; 64-bits of 0 

    .GDT_kernel_code: equ $ - GDT ; Second Entry (Kernal Code Segment) Base = 0, Limit = 0xFFFFF, Access = 1001 1010, Flags = 1010
        dw 0xffff                   ; Segment Limit (15:0)
        dw 0x0                      ; Base Address (15:0)
        db 0x0                      ; Base Address (23:16)
        db 0b10011010               ; Present Bit, Descriptor privilege level (2-Bits), System Segment Bit, Type (4-bits, Code Bit = 1, Conforming Bit, Readable Bit, Accessed Bit)
        db 0b10101111               ; Granularity Bit, Default/Big Bit, Long Mode Bit, Unused Bit, Segment Limit (19:16)
        db 0x0                      ; Base Address (24:31)
        ; dd 0                        ; Base (bits 32-63)
        ; dd 0                        ; Unused

    .GDT_kernel_data: equ $ - GDT ; Third Entry (Kernal Data Segment) Base = 0, Limit = 0xFFFFF, Access = 1001 0010 , Flags = 1100
        dw 0xffff                   ; Segment Limit (15:0)
        dw 0x0                      ; Base Address (15:0)
        db 0x0                      ; Base Address (23:16)
        db 0b10010010               ; Present Bit, Descriptor privilege level (2-Bits), System Segment Bit, Type (4-bits, Data Bit = 0, Expansion Direction Bit, Writable Bit, Accessed Bit)
        db 0b10101111               ; Granularity Bit, Default/Big Bit, Long Mode Bit, Unused Bit, Segment Limit (19:16)
        db 0x0                      ; Base Address (24:31)
        ; dd 0                        ; Base (bits 32-63)
        ; dd 0                        ; Unused

    ; ;GDT_user_code:                  ; Fourth Entry (User Code Segment) Base = 0, Limit = 0xFFFFF, Access = 1111 1010, Flags = 1100
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
    .TSS:                           ; TSS Entry (TSS Segment) Base = TSS Address, Limit = TSS Size = 0x68, Access = 0x89, Flags = 0x40
        dw 0x68                     ; TSS Size (15:0)
        dw 0x0                      ; TSS Address (15:0)
        db 0x0                      ; TSS Address (23:16)
        db 0x89                     ; Present Bit, Descriptor privilege level (2-Bits), System Segment Bit, Type (4-bits, Data Bit = 0, Expansion Direction Bit, Writable Bit, Accessed Bit)
        db 0x40                     ; Granularity Bit, Default/Big Bit, Long Mode Bit, Unused Bit, TSS Size (19:16)
        db 0x0                      ; TSS Address (24:31)
    .GDT_descriptor:
        dw $ - GDT - 1
        dd GDT

DATA:
    welcomeMsg db "Welcome, to Zi Heng's OS", 0
    no_cpu_id_msg db "No CPUID detected", 0
    no_long_mode_msg db "No long mode detected", 0