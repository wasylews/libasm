.model small
.stack 256h

.data

s db 'hello world', 0

.code

LOCALS

include string.inc

main proc
    mov ax, @data
    mov ds, ax

    push offset s
    call strlen

    mov ax, 4c00h
    int 21h

    ret
main endp

end main
