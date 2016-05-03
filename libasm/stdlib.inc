; libasm - partial implementation of libc in tasm
; uses CDECL as calling convention
; Copyright: 2016, Wasylews, S.Habor
; License: MIT


	NULL 			equ 0
	EXIT_FAILURE	equ 0
	EXIT_SUCCESS 	equ 0
	;MB_CUR_MAX		equ ?
	;RAND_MAX		equ ?
	
	
; void exit(int value)
ifndef 'exit'
exit macro v
	ifnb <v>
		mov ax, v
	else
		mov ax, EXIT_SUCCESS
	endif
		mov ah, 4ch
		int 21h
endm
endif


; itoa(int number, char* buffer, int radix) -> buffer
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


; FIXME: float -0.xx..x -> string '0.xx..x'
; ftoa(float *number, char* buffer, int precession) -> buffer
ftoa proc c
    arg @number, @buffer, @precession
    local @@temp
    uses bx, cx, dx

    mov bx, @number
    fld dword ptr[bx]
    mov bx, @@temp

    ; set round mode
    mov word ptr[bx], 0ffffh    ; round mode(only integer)
    fldcw word ptr[bx]

    ; extract integer -> ax
    fist dword ptr[bx]
    mov ax, word ptr[bx]

    ; extract fraction
    fabs
    fild dword ptr[bx]
    fabs
    fsub
    mov word ptr[bx], 0f333h ; 0.1
	mov word ptr[bx+2], 3dcch
    xor dx, dx

    ; if(fration >= 0.1) goto @@ns
    fcom dword ptr[bx]

    push ax
    fstsw ax                ; 286+ instruction
    sahf
    jnc @@ns

    ; else if(fraction == 0) goto @@ns
    fldz                    ; 286+ instruction
    fcomp
    fstsw ax
    sahf
    jz @@ns

    ; else fraction += 0.1
    fadd dword ptr[bx]
    inc dx
@@ns:
    pop ax
    fimul @precession    ; make it int

    fistp dword ptr[bx]
    mov cx, word ptr[bx]    ; fraction -> cx

    ; integer part
    mov bx, @buffer
    push 10
    push bx
    push ax
    call itoa
    add sp, 6

    ; if(fraction == 0) goto @@end
    cmp cx, 0
    je @@end

    ; search position for '.'
    push bx
    call strlen
    add sp, 2

    add bx, ax
    mov byte ptr[bx], '.'
    inc bx
    push bx

    ; fraction part
    push 10
    push bx
    push cx
    call itoa
    add sp, 6

    pop bx
    cmp dl, 0
    je @@end
    dec byte ptr[bx]
@@end:
    ret
ftoa endp


; float atof(char *a, float *num) -> num
; return error code -> bl
;
;   bl == 0 -> Ok
;   bl == 1 -> illegal character
;
;************************************
atof proc c
    arg @a, @num
    uses ax, cx, dx, si
    mov bx, @a
    xor si, si

    ; search position '.'
@@beg_cycle:
    cmp byte ptr[bx], '.' ; if(a[i] != '.')
    je @@end_cycle
    cmp byte ptr[bx], 0 ; if(a[i] != '\0')
    jne @@not_end_str
    inc si
    jmp @@end_cycle
@@not_end_str:
    inc bx
    jmp @@beg_cycle

@@end_cycle:

    mov byte ptr[bx], 0 ; substitute '.' -> '\0'

    ; convert integer part -> num
    push bx
    push @a
    call atoi
    add sp, 2

    ; if(bl != 0) error
    cmp bl, 0
    jne @@end

    mov bx, @num
    cwd
    mov word ptr[bx], ax
    mov word ptr[bx+2], dx
    fild dword ptr[bx] ; num -> st(0)

    ; if(fractional == 0) goto @@endif
    cmp si, 0
    jne @@endif

    ; recovery '.'
    pop bx
    mov byte ptr[bx], '.'

    inc bx

    ; convert fraction part -> num
    push bx
    call atoi

    ; if(bl != 0) error
    cmp bl, 0
    jne @@end

    cwd
    mov bx, @num
    mov word ptr[bx], ax
    mov word ptr[bx+2], dx

    ; counting the number of characters in the fractional part -> cx
    call strlen
    mov cx, ax
    add sp, 2

;********************************
    ; divider: 10^cx -> st(0)
    mov ax, 1
    cwd
    mov bx, 10
@@pow:
    cmp cx, 0
    je @@end_pow
    mul bx
    dec cx
    jmp @@pow
@@end_pow:
;********************************

    sub sp, 4 ; tmp
    mov bx, sp
    mov word ptr[bx], ax
    mov word ptr[bx+2], dx
    fild dword ptr[bx]
    add sp, 4

    ; loading fractional part in st(0)
    mov bx, @num
    fild dword ptr[bx]

    fdivr ; div fraction, 10^cx

    fxch
    fldz                    ; 286+ instruction
    fcomp
    fstsw ax
    sahf
    jc @@more
    fabs
    fadd
    fchs
    jmp @@endif
@@more:
    fadd ; integer + fraction -> st(0)
@@endif:
    fst dword ptr[bx] ; result -> num

    xor bx, bx ; Ok
@@end:
    ret
atof endp


; int atoi(char *buffer) -> ax
;************************************
; return error code -> bl
;
;   bl == 0 -> Ok
;   bl == 3 -> total error
;
;************************************
atoi proc c
    arg @buffer
    push 10
    push @buffer
    call xatoi
    add sp, 4
    ret
atoi endp


; FIXME: don't use bx for error code! All errors return in ax
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
    uses cx, dx
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
    
@@end:
    ret
xatoi endp