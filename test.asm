.286

.model small
.stack 256h

.data

s db 'hello world', 0

.code

LOCALS

include stdio.inc

main proc
    mov ax, @data
    mov ds, ax

    puts s

    mov ax, 4c00h
    int 21h

    ret
main endp

end main
