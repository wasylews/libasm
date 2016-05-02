; libasm - partial implementation of libc in tasm
; uses CDECL as calling convention
; Copyright: 2016, Wasylews, S.Habor
; License: MIT


;;; include section
include string.inc
include stdlib.inc

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

    mov si, @@temp
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

    mov si, @@temp

    push @handle
    push 1
    push si
    call fread
    add sp, 6

    test ax, ax
    je @@return

    mov al, byte ptr[si]
@@return:
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
fgets proc c
    arg @s, @size, @handle
    uses si, cx

    mov si, @s
    mov cx, @size

@@read_loop:
    push @handle
    call fgetc
    add sp, 2

    test ax, ax            ; check for EOF
    je @@eof_return

    mov byte ptr[si], al
    inc si

    cmp al, 0ah             ; check for \n
    je @@stop_reading

    loop @@read_loop
@@stop_reading:
    ; add \0 to end of string
    mov byte ptr[si], 0

@@eof_return:
    ret
fgets endp


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
    jnc @@exit
    mov ax, 0               ; return 0 on error
@@exit:
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


; _putint(int n, int radix, FILE handle)
_putint proc c
    arg @n, @radix, @handle
    local @@temp_buffer

    push @radix
    push word ptr[offset @@temp_buffer]
    push @n
    call itoa
    add sp, 6

    push @handle
    push word ptr[offset @@temp_buffer]
    call fputs
    add sp, 4

    ret
_putint endp


; _putfloat(int n, FILE handle)
_putfloat proc c
    arg @n, @handle
    local @@temp_buffer

    push 6                  ; by default
    push word ptr [offset @@temp_buffer]
    push @n
    call ftoa
    add sp, 6

    push @handle
    push word ptr [offset @@temp_buffer]
    call fputs
    add sp, 4

    ret
_putfloat endp


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

    ; '%' found, check for params after it
    inc si
    cmp byte ptr[si], 's'
    je @@print_str

    cmp byte ptr[si], 'x'
    je @@print_int

    cmp byte ptr[si], 'd'
    je @@print_int

    ; TODO: float precision %.xxf (6 digits by default)
    cmp byte ptr[si], 'f'
    je @@print_float

    ; TODO: unknown format handling

    jne @@just_char

@@print_str:
    ; '%s' found, print string
    push word ptr[bp+4]             ; handle
    push word ptr[bp+di]
    call fputs
    add sp, 4               ; restore stack

    add di, 2
    jmp @@next_char

@@print_int:
    ; '%d' or '%x' found, print integer
    push word ptr[bp+4]

    ; check format for hex
    cmp byte ptr[si], 'x'
    je @@hex_int            ; if so, radix = 16

    ; else, print number as decimal
    push 10
    jmp @@print

@@hex_int:
    push 16

@@print:
    push word ptr[bp+di]
    call _putint
    add sp, 6

    add di, 2
    jmp @@next_char

@@print_float:
    ; '%f' found, print float
    push word ptr[bp+4]
    push word ptr[bp+di]
    call _putfloat
    add sp, 4

    add di, 2
    jmp @@next_char
@@just_char:
    fputc bl word ptr[bp+4]

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
