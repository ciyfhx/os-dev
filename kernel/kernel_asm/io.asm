[bits 64]
global outb

; outb - send a byte to an I/O port
; param: [rsi] the data byte
;        [rdi] the I/O port

outb:
    mov rax, rsi    ; move the data to be sent into the al register
    mov rdx, rdi    ; move the address of the I/O port into the dx register
    out dx, al           ; send the data to the I/O port
    ret                  ; return to the calling function

global inb

; inb - returns a byte from the given I/O port
; param: [rdi] The address of the I/O port

inb:
    mov rdx, rdi           ; move the address of the I/O port to the dx register
    in  al, dx              ; read a byte from the I/O port and store it in the al register
    ret                     ; return the read byte
    