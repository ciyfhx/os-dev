global memcpy

; memcpy
; stack: [esp + 12] number of bytes to copy
;        [esp + 8] src address
;        [esp + 4] dest address
;        [esp    ] return address
memcpy:
    mov edi, [esp + 4]
    mov esi, [esp + 8]
    mov ecx, [esp + 12]
    rep movsb
    ret

global memcmp

; memcmp
; stack: [esp + 12] number of bytes to compare
;        [esp + 8] src address
;        [esp + 4] dest address
;        [esp    ] return address
memcmp:
    mov edi, [esp + 4]
    mov esi, [esp + 8]
    mov ecx, [esp + 12]
    xor eax, eax
    cld
    repe cmpsb
    setnz al
    ret

