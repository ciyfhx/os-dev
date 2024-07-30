[bits 16]
[org 0x0000]

KERNEL_LOADER_LOCATION equ 0x1000 ; relative to 0x7C00
FAT32_HEADER_SIZE      equ start - 3

; FAT32  header 
jmp short start
nop

bdb_oem:                    db 'mkfs.fat'           ; 8 bytes
bdb_bytes_per_sector:       dw 512                  ; 0x00 20
bdb_sectors_per_cluster:    db 1                    ; 0x01
bdb_reserved_sectors:       dw 32                   ; 0x20 00

bdb_fat_count:              db 2                    ; 0x02
bdb_dir_entries_count:      dw 0                    ; 0x00 00
bdb_total_sectors:          dw 0                    ; 0x00 00
bdb_media_descriptor_type:  db 0xF8                 ; F8 = Fixed disk
bdb_sectors_per_fat:        dw 0                    ; 0x00 00
bdb_sectors_per_track:      dw 32                   ; 0x20 00
bdb_heads:                  dw 4                    ; 0x40 00
bdb_hidden_sectors:         dd 0                    ; 0x00 00 00 00

bdb_large_sector_count:     dd 131072               ; 0x00 00 02 00

; extended boot record
ebr_sectors_per_fat         dd 1009                 ; 0xF1 03 00 00
ebr_flags                   dw 0                    ; 0x00 00
ebr_fat_version_no          dw 0                    ; 0x00 00
ebr_root_dir_cluster_no     dd 2                    ; 0x02 00 00 00
ebr_fsinfo_sector           dw 1                    ; 0x01 00
ebr_backup_boot_sector      dw 6                    ; 0x06 00
; reserved
TIMES 12 DB 0          
ebr_drive_number:           db 128                  ; 0x00 floppy, 0x80 hdd, useless
                            db 0                    ; reserved
ebr_signature:              db 29h
ebr_volume_id:              db 12h, 34h, 56h, 78h   ; serial number, value doesn't matter
ebr_volume_label:           db 'ZIHENG OS  '        ; 11 bytes, padded with spaces
ebr_system_id:              db 'FAT32   '           ; 8 bytes


start:
    ; setup data segments
    mov     ax, 0x07C0      ; code begin at 0x7c00 ;7c5a
    mov     ds, ax
    mov     es, ax
    mov     fs, ax
    mov     gs, ax

    mov [BOOT_DISK], dl
    ; save partition lba
    mov [PARITION_LBA], di

    ; setup stack
    xor ax, ax
    mov ss, ax
	mov	bp, 0xF000			; stack begins at 0xF000
	mov	sp, bp

    mov ah, 0x0             ; text mode (clear screen)
    mov al, 0x3
    int 0x10        

    ; calculate file allocation table sector size (bdb_fat_count * ebr_sectors_per_fat) 
    mov al, [bdb_fat_count]     
    mov cx, [ebr_sectors_per_fat]
    mul cx
    mov [FAT_SIZE], ax
    ; calculate LBA for data
    add ax, word [bdb_reserved_sectors]
    add ax, word [PARITION_LBA]
    mov [DATA_LBA], ax      ; save LBA for data (bp - 2)

    ; read root directory content
    xor dx, dx
    mov bx, buffer          ; write directory content to this location (location = es * 16  + bx)
    mov ax, [ebr_root_dir_cluster_no]       
    sub ax, 2
    add ax, [DATA_LBA]
    mov cl, 1               ; read one sector 
    mov dl, [BOOT_DISK]     ; drive number
    call read_from_disk

    ; search kernel loader from root directory
    ; compare two string buffers located at ds:si and es:di
    xor bx, bx
    mov di, buffer
.search_kernel_loader:
    mov si, file_kernel_loader_bin
    mov cx, 11                          ; compare up to 11 characters
    push di
    repe cmpsb                          ; repeat until z-flag is triggered = 1
    pop di
    je .found_kernel_loader             ; copy kernel file content to KERNEL_LOADER_LOCATION (7cba)

    add di, 32                          ; move to the next entry
    inc bx                              
    cmp bx, 16
    jl .search_kernel_loader

    ; kernel loader not found
    jmp .kernel_loader_not_found_error

.kernel_loader_not_found_error:
    mov si, msg_kernel_loader_not_found
    call puts
    hlt

.found_kernel_loader:
    mov ax, [ebr_root_dir_cluster_no]        ; LBA root directory
    ; mov cx, [bp - 4]        ; root directory sector size
    ; add ax, cx              ; LBA address for data
    ; push ax                 ; save LBA for data (bp - 6)

    xor ax, ax
    mov ax, [di + 26]       ; get cluster number
    mov [READING_CLUSTER_LBA], ax                 ; save cluster number

    ; load FAT from disk into memory
    xor dx, dx
    mov bx, buffer
    mov ax, [bdb_reserved_sectors]  
    add ax, [PARITION_LBA] ; LBA of fat
    mov cl, 1   ; fat sector size (only can load up to 127 sectors)
    mov dl, [BOOT_DISK]

    mov di, 2
.read_fat:
    call read_from_disk
    add bx, word [bdb_bytes_per_sector]
    inc ax
    dec di
    test di, di
    jnz .read_fat

    mov cx, KERNEL_LOADER_LOCATION ;7cfb
    push cx                 ; save kernel loader write desination (bp - 2)

    ; calculate cluster byte size
    mov ax, [bdb_bytes_per_sector]
    xor cx, cx
    mov cl, [bdb_sectors_per_cluster]
    mul cx ;7cd4
    mov [CLUSTER_SIZE], ax                 ; cluster byte size (bp - 12)

.read_content_loop:
    ; read cluster
    mov ax, [READING_CLUSTER_LBA]    ; cluster number
    mov si, [DATA_LBA]    ; LBA data
    mov di, [bp - 2]   ; kernel loader destination
    call read_cluster
    ; update kernel loader destination
    add di, [CLUSTER_SIZE]   ; should add bdb_bytes_per_sector * bdb_sectors_per_cluster
    mov [bp - 2], di

    ; get next cluster
    mov di, buffer      ; FAT location 
    call get_next_cluster
    ; check if end of cluster
    cmp cx, 0xFFFF      ; only can load until 0xFFFE
    ; update cluster number
    mov [READING_CLUSTER_LBA], cx
    jb .read_content_loop

    ; boot kernel loader
.boot_kernel:
    jmp KERNEL_LOADER_LOCATION

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
    push ax
    push bx
    push si
    mov bx, di
    mov cx, 4
    mul cx
    mov si, ax
    mov ax, [bx + si]
    mov cx, ax
    pop si
    pop bx
    pop ax
    ret


; Prints a string to the screen
; Params:
;   - ds:si points to string
;
puts:
    pusha
.loop:
    lodsb               ; loads next character in al
    or al, al           ; verify if next character is null?
    jz .done

    mov ah, 0x0E        ; call bios interrupt
    mov bh, 0           ; set page number to 0
    int 0x10

    jmp .loop

.done:
    popa
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
    pusha
    mov bp, sp
    call lba_to_chs         ; convert lba to chs
    push dx
    mov dx, [bp + 12]       ; sectors to read
    mov ah, 0x02            ; 0x02 = read from disk
    mov al, dl
    pop dx                  ; head number
    mov dl, [bp + 10]       ; drive number
    ; mov di, 3
.read:
    pusha                   ; push all registers
    stc                     ; set the carry flag
    int 0x13                ; read disk
    popa                    ; pop all registers
    jnc .done
.fail:
    ; dec di
    ; test di, di
    ; call disk_reset
    ; jnz .read
    jmp read_error
.done:
    popa
    ret

; ;
; ; Resets disk controller
; ; Parameters:
; ;   dl: drive number
; ;
; disk_reset:
;     pusha
;     mov ah, 0
;     stc
;     int 13h
;     jc read_error
;     popa
;     ret

;
; Error handlers
;

read_error:
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


%define ENDL 0x0D, 0x0A

; strings
msg_read_failed:               db 'Read disk failed!', ENDL, 0
msg_kernel_loader_not_found:   db 'Unable to find LOADER', ENDL, 0
file_kernel_loader_bin:        db 'LOADER  BIN'

times 510-($-$$) db 0              
dw 0xaa55

PARITION_LBA dw 0
BOOT_DISK: db 0
FAT_SIZE:  dw 0
DATA_LBA:  dw 0
READING_CLUSTER_LBA:  dw 0
CLUSTER_SIZE: dw 0

buffer: