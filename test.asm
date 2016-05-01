.286

.model small
.stack 256h

.data
filename db 'test.asm', 0
handle dw ?
buffer db 20 dup('')

.code

LOCALS

include stdio.inc

main proc c
    mov ax, @data
    mov ds, ax

    push RO
    push offset filename
    call fopen
    add sp, 4

    test ax, ax
    je @@exit

    mov handle, ax
@@read_loop:
    push handle
    push 20
    push offset buffer
    call fgets
    add sp, 6

    test ax, ax
    je @@close_file

    puts buffer
    jmp @@read_loop

@@close_file:
    push handle
    call fclose
    add sp, 2

@@exit:
    mov ax, 4c00h
    int 21h

    ret
main endp

end main
