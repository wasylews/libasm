.model small
.386

.stack 256h

.data
fmt db 'Ubuntu %f', 0ah, 0dh, 0
float_number dd 12.123

filename db 'LICENSE', 0
handle dw ?
buffer db 20 dup('')

.code

LOCALS

include stdio.inc

main proc c
    mov ax, @data
    mov ds, ax

    push offset float_number
    push offset fmt
    push STDOUT
    call fprintf

    ; open file for read
    push RO
    push offset filename
    call fopen
    add sp, 4

    ; check for errors
    test ax, ax
    je @@exit

    ; save handle
    mov handle, ax

    ; skip first 2 chars
    push SEEK_CUR
    push 0
    push 2
    push handle
    call fseek
    add sp, 6

    ; read file line by line
@@read_loop:
    push handle
    push 20
    push offset buffer
    call fgets
    add sp, 6

    test ax, ax
    je @@close_file

    ; print to console
    puts buffer
    jmp @@read_loop

    ; close file
@@close_file:
    push handle
    call fclose
    add sp, 2

@@exit:
    exit
main endp

end main
