; libasm - partial implementation of libc in tasm
; uses CDECL as calling convention
; Copyright: 2016, Wasylews
; License: MIT

;;; fopen() modes
RO equ 0                    ; read only
WO equ 1                    ; write only, creates file if doesn't exists
RW equ 2                    ; read/write

;;; fseek() whence values
SEEK_SET equ 00h            ; relative to start
SEEK_CUR equ 01h            ; to current position
SEEK_END equ 02h            ; to end-of-file

;;; standard file handles
STDOUT equ 1

; fputc(char c, FILE handle) -> ax
fputc macro c: req, handle: req
ifnb <c, handle>
    local @@temp
    ; we need to save registers
    push si

    lea si, @@temp
    mov byte ptr[si], c

    push handle
    push 1
    push si
    call fwrite

    add sp, 6

    ; don't forget restore
    pop si
endif
endm


; putc(char c) -> al
putc macro c: req
ifnb <c>
    fputc c STDOUT
endif
endm


; char getc() -> al
getc macro
    mov ah, 01h
    int 21h
endm

; char fgetc(FILE handle) -> al
fgetc proc c
    arg @handle
    local @@temp
    uses si

    lea si, @@temp
    push @handle
    push 1
    push si
    call fread
    add sp, 6

    xor ax, ax
    mov al, byte ptr[si]
    ret
fgetc endp


; int fputs(char *s, FILE handle) -> ax
fputs proc c
    arg @s, @handle

    push @s
    call strlen
    add sp, 2

    push @handle
    push ax
    push @s
    call fwrite
    jnc @@return
    mov ax, 0

@@return:
    add sp, 6
    ret
fputs endp


; int puts(char *s) -> ax
puts macro s: req
ifnb<s>
    push STDOUT
    push offset s
    call fputs
    add sp, 4
endif
endm


; int gets(char *s) -> ax
gets proc c
    arg @s
    uses si

    mov si, @s
@@read_loop:
    getc

    cmp al, 13
    je @@end
    cmp al, 8
    je @@backspace

    mov byte ptr [si], al
    inc si
    jmp @@read_loop

@@backspace:
    ; get current pos in dx(dl - x, dh - y)
    mov ah, 03h
    int 10h

    ; save it
    push dx

    ; erase char
    mov ah, 02h
    mov dl, ' '
    int 21h

    ; restore position
    pop dx
    mov ah, 02h
    int 10h
    ; rewrite char by new
    dec si
    jmp @@read_loop

@@end:
    mov byte ptr [si], 0
    ret
gets endp


; int fgets(char *s, int size, FILE handle)
; TOOD: read string char-by-char, stop when \n\r found or strlen(s) == size
fgets proc c
    uses si, cx
    arg @s, @size, @handle

    mov si, @s
    mov cx, @size

@@read_loop:
    push @handle
    call fgetc
    add sp, 2

    mov byte ptr[si], al
    inc si
    ; TODO: check for EOF, EOL
    cmp al, 0ah
    je @@stop_reading

    ; check for EOF
    test ax, ax
    je @@exit

    loop @@read_loop
@@stop_reading:
    ; skip \r\n
    ; push SEEK_CUR
    ; push 2
    ; push @handle
    ; call fseek
    ; add sp, 6

    ; add \0 to end of string
    mov byte ptr[si], 0
@@exit:
    ret
fgets endp


; itoa(int number, char* buffer, int radix)
itoa proc c
    arg @number, @buffer, @radix
    uses bx, cx, dx, si
    mov ax, @number
    mov si, @radix
    mov bx, @buffer

    xor cx, cx

    cmp si, 1
    ja @@ns1
    mov al, 1
    jmp @@end
@@ns1:
    cmp si, 36
    jna @@ns2
    mov al, 2
    jmp @@end
@@ns2:
    cmp ax, 0
    jnl @@ns3
    neg ax
    mov byte ptr[bx], '-'
    inc bx
@@ns3:
    cwd
    div si
    add dx, 48
    cmp dx, 57
    jna @@less
    add dx, 7
@@less:
    push dx
    inc cx
    cmp ax, 0
    jne @@ns3

@@cycle:
    pop ax
    mov byte ptr[bx], al
    inc bx
    dec cx
    cmp cx, 0
    jne @@cycle
    xor al, al
@@end:
    mov byte ptr[bx], 0
    ret
itoa endp



; ftoa(float *number, char* buffer, int setprecession)
; FIXME: invalid result if number is like xxx.0xxx
ftoa proc c
    arg @number, @buffer, @setprecession
    local @@temp
    uses bx, cx

    mov bx, @number
    fld dword ptr[bx]

    mov bx, @@temp

    ; set round mode
    mov word ptr[bx], 0f7ffh; round mode(only integer)
    fldcw word ptr[bx]

    fist dword ptr[bx]
    mov ax, word ptr[bx]

    ; fraction
    fisub dword ptr[bx]
    fimul @setprecession    ; make it int

    fistp dword ptr[bx]
    mov cx, word ptr[bx]    ; fraction in cx

    ; integer part
    mov bx, @buffer
    push 10
    push bx
    push ax
    call itoa
    add sp, 6

    push bx
    call strlen
    add sp, 2

    add bx, ax
    mov byte ptr[bx], '.'
    inc bx

    ; fraction part
    push 10
    push bx
    push cx
    call itoa
    add sp, 6

    ret
ftoa endp


; int atoi(char *buffer) -> ax
atoi proc c
    arg @buffer
    uses bx, si

    mov si, @buffer         ; buffer
    xor ax, ax              ; ax = 0

@@trans_loop:
    xor bx, bx
    mov bl, byte ptr[si]    ; bl <- buffer[esi]

    cmp bl, '0'
    jl @@exit               ; bl < '0' (unsigned)

    cmp bl, '9'
    jg @@exit               ; bl > '9' (unsigned)

    cmp bl, 10
    je @@exit

    cmp bl, 0
    je @@exit

    sub bl, '0'             ; convert bl to integer(bl = bl - '0')
    add ax, bx              ; ax = ax + bx
    mov bx, 10
    mul bx                  ; ax = ax * 10
    inc si

    jmp @@trans_loop

@@exit:
    mov bx, 10
    div bx
    ret
atoi endp

; int xatoi(char *buffer, int radix) -> ax
;************************************
; return error code -> bl
;
;   bl == 0 -> Ok
;   bl == 1 -> radix <= 1
;   bl == 2 -> radix > 36
;   bl == 3 -> total error
;
;************************************
xatoi proc c
    arg @buffer, @radix
    xor ax, ax
    mov bx, @buffer
    mov dx, @radix
    xor cx, cx
    cmp dl, 1
    ja @@ns1
    mov bl, 1
    jmp @@end
    @@ns1:
    cmp dl, 36
    jna @@ns2
    mov bl, 2
    jmp @@end
    @@ns2:
    mov cl, byte ptr[bx]
    cmp cl, '-'
    jne @@cycle
    inc bx

    @@cycle:
    mov cl, byte ptr[bx]
    cmp cl, 0
    je @@ns
    sub cl, 48
    cmp cl, dl
    jb @@ns4
    sub cl, 7
    cmp cl, dl
    jb @@ns4
    mov bl, 3
    jmp @@end
    @@ns4:
    mul dl
    add ax, cx
    inc bx
    jmp @@cycle
    @@ns:
    mov bx, @buffer
    cmp byte ptr[bx], '-'
    jne @@ns5
    neg ax
    @@ns5:
    xor bl, bl
    @@end: ret
xatoi endp



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


; fputn(int n, FILE handle) - converts number to sring and prints it to file
; custom function, for fprintf
fputn proc c
    arg @n, @handle
    local @@temp_buffer

    push 10
    push offset @@temp_buffer
    push @n
    call itoa
    add sp, 6

    push @handle
    push offset @@temp_buffer
    call fputs
    add sp, 4

    ret
fputn endp


; int fprintf(FILE handle, char *fmt, ...)
fprintf proc c
    push bp
    mov bp, sp

    push si
    push di
    push bx

    mov si, [bp+6]          ; fmt
    xor bx, bx
    mov di, 8               ; first arg on [bp+08]
@@format_loop:
    mov bl, byte ptr[si]
    cmp bl, 0
    je @@printf_exit

    cmp bl, '%'
    jne @@just_char         ; not begin of string escape

; TODO: use switch operator
    ; '%' found, check for params after it
    inc si
    cmp byte ptr[si], 's'
    je @@print_str
    cmp byte ptr[si], 'd'
    je @@print_int

    ; TODO: float/double

    ; TODO: unknown format handling

    jne @@just_char

@@print_str:
    ; '%s' found, print string
    push [bp+4]             ; handle
    push [bp+di]
    call fputs
    add sp, 4               ; restore stack

    add di, 2
    jmp @@next_char

@@print_int:
    ; '%d' found, print integer
    push [bp+4]
    push [bp+di]
    call fputn
    add sp, 4

    add di, 2
    jmp @@next_char

@@just_char:
    fputc bl [bp+4]

@@next_char:
    inc si
    jmp @@format_loop

@@printf_exit:
    pop bx
    pop di
    pop si
    pop bp

    ret
fprintf endp


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


; upper(char c)
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



; lower(char c)
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


; TOOD: scanf()

;;; functions for work with files
; TODO: fprintf()/fscanf()

; FILE fopen(char *path, char* mode) -> ax
fopen proc c
    arg @path, @mode
    uses bx, cx, dx, si

    mov dx, @path

    mov bx, @mode
    cmp bx, WO
    jne @@open_file

    mov ah, 3ch
    xor cx, cx
    int 21h
    jmp @@ret_handle

@@open_file:
    mov ah, 3dh
    mov al, bl
    mov dx, @path
    int 21h

@@ret_handle:
    jnc @@return
    mov ax, 0

@@return:
    ret
fopen endp


; int fclose(FILE handle) -> ax
fclose proc c
    arg @handle
    uses bx

    mov ah, 3eh
    mov bx, @handle
    int 21h
    jc @@error

    mov ax, 0               ; no error
@@error:
    ret
fclose endp


; int fread(void *ptr, int size, FILE handle) -> ax
fread proc c
    arg @ptr, @size, @handle
    uses bx, cx, dx

    mov ah, 3fh
    mov bx, @handle
    mov cx, @size
    mov dx, @ptr
    int 21h

    ret
fread endp


; int fwrite(const void *ptr, int size, FILE handle) -> ax
fwrite proc c
    arg @ptr, @size, @handle
    uses bx, cx, dx

    mov ah, 40h
    mov bx, @handle
    mov cx, @size
    mov dx, @ptr
    int 21h

    ret
fwrite endp


; int fseek(FILE handle, long offset, int whence)
fseek proc c
    arg @handle, @offset, @whence
    uses bx, cx, dx

    mov ah, 42h
    mov al, byte ptr[@whence]
    mov bx, @handle

    mov dx, word ptr[@offset]
    mov cx, word ptr [@offset+2]
    int 21h
    ret
fseek endp


;;; custom macro, for passing params to fprintf/scanf and so on
; pushb(byte data) - pushes byte to stack
; WARN: cleans bx register and not restore it
pushb macro data: req
ifnb <data>
    xor bx, bx
    mov bl, data
    push bx
endif
endm


;;; few macro for work with date and time
; getdate() -> cx(year), dh(month), dl(day), al(day of week)
getdate macro
    mov ah, 2ah
    int 21h
endm

; gettime() -> ch(hours), cl(minutes), dh(seconds)
gettime macro
    mov ah, 2ch
    int 21h
endm


;;; WARN: setdate() and settime() doesn't work on dosbox
; setdate(word year, byte month, byte day, byte dayOfWeek)
setdate macro year, month, day, dayOfWeek
ifnb <year>
    mov cx, year
endif
ifnb <month>
    mov dh, month
endif
ifnb <day>
    mov dl, day
endif
ifnb <dayOfWeek>
    mov al, dayOfWeek
endif
    mov ah, 2bh
    int 21h
endm


; settime(byte hours, byte minutes, byte seconds)
settime macro
ifnb <hours>
   mov ch, hours
endif
ifnb <minutes>
    mov cl, minutes
endif
ifnb <seconds>
    mov dh, seconds
endif
    mov ah, 2dh
    int 21h
endm
