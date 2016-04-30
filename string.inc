; libasm - partial implementation of libc in tasm
; uses CDECL as calling convention
; Copyright: 2016, Wasylews, S.Habor
; License: MIT


; int strlen(char *s) -> ax
strlen proc c
    uses bx
    arg @s

    mov bx, @s
    xor ax, ax

@@count_loop:
    cmp byte ptr[bx], 0
    je @@end_loop

    inc ax
    inc bx
    jmp @@count_loop

@@end_loop:
    ret
strlen endp


; int strcmp(char *s1, char *s2) -> ax
strcmp proc c
    arg @s1, @s2
    uses bx, si, di

    mov si, @s1
    mov di, @s2

@@strcmp_loop:
    xor ax, ax
    xor bx, bx

    mov al, [si]
    mov bl, [di]

    sub al, bl
    jne @@strcmp_done         ; return if different
    cmp bl, 0                 ; on end of both strings, ax = 0
    je @@strcmp_done
    inc si
    inc di
    jmp @@strcmp_loop

@@strcmp_done:
    cbw                     ; extend al to ax
    ret
strcmp endp


; char upper(char c) -> al
upper proc c
    arg @@c
    mov ax, @@c
    cmp al, 'a'
    jb @@end
    cmp al, 'z'
    ja @@end
    sub al, 32
@@end:
    ret
upper endp


; toupper(char *s)
toupper proc c
    uses bx, dx
    arg @s
    mov bx, @s
@@beg:
    mov dl, byte ptr[bx]
    push dx
    call upper
    add sp, 2
    mov byte ptr[bx], al
    inc bx
    cmp byte ptr[bx], 0
    jne @@beg
    ret
toupper endp


; char lower(char c) -> al
lower proc c
    arg @@c
    mov ax, @@c
    cmp al, 'A'
    jb @@end
    cmp al, 'Z'
    ja @@end
    add al, 32
@@end:
    ret
lower endp


; tolower(char *s)
tolower proc c
    uses bx, dx
    arg @s
    mov bx, @s
@@beg:
    mov dl, byte ptr[bx]
    push dx
    call lower
    add sp, 2
    mov byte ptr[bx], al
    inc bx
    cmp byte ptr[bx], 0
    jne @@beg
    ret
tolower endp
