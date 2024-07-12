[org 0x7c00]

KERNEL_LOCATION equ 0x1000

; FAT12  header 
jmp short start
nop

bdb_oem:                    db 'MSWIN4.1'           ; 8 bytes
bdb_bytes_per_sector:       dw 512
bdb_sectors_per_cluster:    db 1
bdb_reserved_sectors:       dw 1
bdb_fat_count:              db 2
bdb_dir_entries_count:      dw 0E0h
bdb_total_sectors:          dw 2880                 ; 2880 * 512 = 1.44MB
bdb_media_descriptor_type:  db 0F0h                 ; F0 = 3.5" floppy disk
bdb_sectors_per_fat:        dw 9                    ; 9 sectors/fat
bdb_sectors_per_track:      dw 18
bdb_heads:                  dw 2
bdb_hidden_sectors:         dd 0
bdb_large_sector_count:     dd 0

; extended boot record
ebr_drive_number:           db 0                    ; 0x00 floppy, 0x80 hdd, useless
                            db 0                    ; reserved
ebr_signature:              db 29h
ebr_volume_id:              db 12h, 34h, 56h, 78h   ; serial number, value doesn't matter
ebr_volume_label:           db 'ZIHENG OS'          ; 11 bytes, padded with spaces
ebr_system_id:              db 'FAT12   '           ; 8 bytes


start:
    mov [BOOT_DISK], dl

    ; setup data segments
    xor	ax, ax				; null segments
	mov	ds, ax
	mov	es, ax

    ; setup stack
    mov ss, ax
	mov	bp, 0x8000			; stack begins at 0x8000-0xffff
	mov	sp, bp

    mov ah, 0x0             ; text mode (clear screen)
    mov al, 0x3
    int 0x10

    ; read kernel from disk
    mov bx, KERNEL_LOCATION ; write kernel to this location (location = es * 16  + bx)
    mov ax, 1               ; LBA=1, coorespond to the second sector
    mov cl, 1               ; number of sectors
    mov dl, [BOOT_DISK]     ; drive number
    call read_from_disk
    
    mov si, msg_boot_kernel
    call puts

    jmp KERNEL_LOCATION

; Prints a string to the screen
; Params:
;   - ds:si points to string
;
puts:
    ; save registers we will modify
    push si
    push ax
    push bx

.loop:
    lodsb               ; loads next character in al
    or al, al           ; verify if next character is null?
    jz .done

    mov ah, 0x0E        ; call bios interrupt
    mov bh, 0           ; set page number to 0
    int 0x10

    jmp .loop

.done:
    pop bx
    pop ax
    pop si    
    ret
    

; Linear Block Addressing to Cyclinder Head Sector
; Parameters:
;   - ax: LBA address
; Returns:
;   - cx [bits 0-5]: sector number
;   - cx [bits 6-15]: cylinder
;   - dh: head
;
lba_to_chs:
    push ax
    push dx

    xor dx, dx                          ; dx = 0
    div word [bdb_sectors_per_track]    ; ax = LBA / SectorsPerTrack
                                        ; dx = LBA % SectorsPerTrack

    inc dx                              ; dx = (LBA % SectorsPerTrack + 1) = sector
    mov cx, dx                          ; cx = sector

    xor dx, dx                          ; dx = 0
    div word [bdb_heads]                ; ax = (LBA / SectorsPerTrack) / Heads = cylinder
                                        ; dx = (LBA / SectorsPerTrack) % Heads = head
    mov dh, dl                          ; dh = head
    mov ch, al                          ; ch = cylinder (lower 8 bits)
    shl ah, 6
    or cl, ah                           ; put upper 2 bits of cylinder in CL

    pop ax
    mov dl, al                          ; restore DL
    pop ax
    ret

;
; Reads sectors from a disk
; Parameters:
;   - ax: LBA address
;   - cl: number of sectors to read (up to 128)
;   - dl: drive number
;   - es:bx: memory address where to store read data
;
read_from_disk:
    push ax
    push dx
    push es
    push bx
    push cx

    call lba_to_chs         ; convert lba to chs
    pop ax                  ; pop the value of cx to ax
    mov ah, 0x02            ; 0x02 = read from disk
    mov di, 3
.read:
    pusha                   ; push all registers
    stc                     ; set the carry flag
    int 0x13                ; read disk
    popa                    ; pop all registers
    jnc .done
.fail:
    dec di
    test di, di
    call disk_reset
    jnz .read
    jmp floppy_error
.done:
    pop bx
    pop es
    pop dx
    pop ax
    ret

;
; Resets disk controller
; Parameters:
;   dl: drive number
;
disk_reset:
    pusha
    mov ah, 0
    stc
    int 13h
    jc floppy_error
    popa
    ret

;
; Error handlers
;

floppy_error:
    mov si, msg_read_failed
    call puts
    jmp wait_key_and_reboot

wait_key_and_reboot:
    mov ah, 0
    int 16h                     ; wait for keypress
    jmp 0FFFFh:0                ; jump to beginning of BIOS, should reboot

.halt:
    cli                         ; disable interrupts, this way CPU can't get out of "halt" state
    hlt

BOOT_DISK: db 0
%define ENDL 0x0D, 0x0A

; strings
msg_read_failed:        db 'Read from disk failed!', 0
msg_boot_kernel:        db 'Booting into kernel', 0

times 510-($-$$) db 0              
dw 0xaa55