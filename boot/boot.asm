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
ebr_volume_label:           db 'ZIHENG OS  '          ; 11 bytes, padded with spaces
ebr_system_id:              db 'FAT12   '           ; 8 bytes


start:
    ; setup data segments
    xor	ax, ax				; null segments
	mov	ds, ax
	mov	es, ax

    mov [BOOT_DISK], dl

    ; setup stack
    mov ss, ax
	mov	bp, 0xF000			; stack begins at 0xF000
	mov	sp, bp

    mov ah, 0x0             ; text mode (clear screen)
    mov al, 0x3
    int 0x10

    ; ; read kernel from disk
    ; mov si, msg_read_kernel
    ; call puts               

    ; calculate file allocation table sector size (bdb_fat_count * bdb_sectors_per_fat) 
    mov al, [bdb_fat_count]     
    mov cx, [bdb_sectors_per_fat]
    mul cx
    ; calculate LBA of root directory
    add ax, word [bdb_reserved_sectors]
    push ax

    ; calculate root directory sector size (bdb_dir_entries_count * 32 + bdb_bytes_per_sector - 1) / bdb_bytes_per_sector
    mov ax, [bdb_dir_entries_count] ; 0x7c69
    mov bx, [bdb_bytes_per_sector]
    dec bx
    shl ax, 5
    add ax, bx
    ; xor dx, dx
    div word [bdb_bytes_per_sector]
    push ax

    ; read root directory
    xor dx, dx
    mov bx, buffer          ; write kernel to this location (location = es * 16  + bx) ;0x7c7b
    mov ax, [bp - 2]        ; LBA address of root directory
    mov cx, [bp - 4]        ; root directory sector size
    mov dl, [BOOT_DISK]     ; drive number
    call read_from_disk

    ; search kernel from root directory
    ; compare two string buffers located at ds:si and es:di
    xor bx, bx
    mov di, buffer
.search_kernel:
    mov si, file_kernel_bin
    mov cx, 11                          ; compare up to 11 characters
    push di
    repe cmpsb                          ; repeat until z-flag is triggered = 1
    pop di
    je .found_kernel                    ; copy kernel file content to KERNEL_LOCATION

    add di, 32                          ; move to the next entry
    inc bx                              
    cmp bx, [bdb_dir_entries_count]
    jl .search_kernel

    ; kernel not found
    jmp .kernel_not_found_error

.kernel_not_found_error:
    mov si, msg_kernel_not_found
    call puts
    hlt

.found_kernel:
    mov ax, [bp - 2]        ; LBA address of root directory
    mov cx, [bp - 4]        ; root directory sector size
    add ax, cx              ; LBA address of data
    push ax

    xor ax, ax
    mov ax, [di + 26]       ; get cluster number
    push ax

    ; load FAT from disk into memory
    xor dx, dx
    mov bx, buffer
    mov ax, [bdb_reserved_sectors]  ; LBA of fat
    mov cl, [bdb_sectors_per_fat]   ; fat sector size
    mov dl, [BOOT_DISK]
    call read_from_disk



    mov cx, KERNEL_LOCATION     ;7cc7
.read_content_loop:
    pop ax              ; cluster number
    ; read cluster
    mov si, [bp - 6]    ; LBA data
    mov di, cx
    call read_cluster
    ; get next cluster
    mov di, buffer  ;7cd3
    push cx
    call get_next_cluster
    ; check if end of cluster
    cmp cx, 0xFF8
    mov ax, cx
    pop cx
    add cx, [bdb_bytes_per_sector]
    jb .read_content_loop

    ; boot kernel
.boot_kernel:
    jmp KERNEL_LOCATION

; Read the cluster
; Parameters:
;   - ax: cluster number
;   - si: cluster data location
;   - di: write destination
;
read_cluster:
    pusha
    mov bx, di
    sub ax, 2
    mul byte [bdb_sectors_per_cluster]
    add ax, si
    mov cl, [bdb_sectors_per_cluster]
    mov dl, [BOOT_DISK]
    call read_from_disk
    popa
    ret

; Returns the next cluster from the FAT
; Parameters:
;   - ax: current cluster
;   - di: pointer to fat buffer
; Returns:
;   - cx: next cluster
;
get_next_cluster:
    push bx
    push ax
    mov bx, di
    mov cx, 3
    mul cx
    shr ax, 1   ; ax * 3 / 2
    mov si, ax
    test ax, ax
    mov ax, [bx + si]
    jnz .done
; .even:
;     ror ax, 8
;     jmp .done
.odd:
    shr ax, 4
.done:
    and ax, 0x0FFF
    mov cx, ax
    pop ax
    pop bx
    ret


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
    push bp
    mov bp, sp
    push ax
    push es
    push bx
    push dx
    push cx

    call lba_to_chs         ; convert lba to chs
    push dx
    mov dx, [bp - 10]        ; sectors to read
    mov ah, 0x02            ; 0x02 = read from disk
    mov al, dl
    pop dx                  ; head number
    mov dl, [bp - 8]        ; drive number
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
    mov sp, bp
    pop bp
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
msg_read_failed:        db 'Read from disk failed!', ENDL, 0
msg_kernel_not_found:   db 'Unable to find KERNEL BIN from root directory', ENDL, 0
file_kernel_bin:        db 'KERNEL  BIN'

times 510-($-$$) db 0              
dw 0xaa55

buffer: