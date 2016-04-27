; libasm - partial implementation of libc in tasm
; uses CDECL as calling convention
; Copyright: 2016, Wasylews, S.Habor
; License: MIT



; int iabs(word num)
iabs proc c
	arg @num
	mov ax, @num
	cmp ax, 0
	jnl @@end
	neg ax
@@end:
	ret
iabs endp

;int round(float *num, int rmode)
;************************************
;
;	rmode == 0 -> floor
;	rmode == 1 -> ceil
;
;************************************
round proc c
	arg @num, @rmode
	uses ax, bx
	mov bx, @num
	fld dword ptr[bx]
	; set round mode
	cmp @rmode, 0
	je @@floor
	mov ax, 0fbffh
	jmp @@ns
@@floor:
	mov ax, 0f7ffh
@@ns:
    mov word ptr[bx], ax
    fldcw word ptr[bx]
	fist dword ptr[bx]
	ret
round endp


;int ceil(float *num)
ceil proc c
	arg @num
	push 1
	push @num
	call round
	add sp, 4
	ret
ceil endp



;int floor(float *num)
floor proc c
	arg @num
	push 0
	push @num
	call round
	add sp, 4
	ret
floor endp



;float rabs(float *num, float *res)
rabs proc c
	arg @num, @res
	uses bx
	mov bx, @num
	fld dword ptr[bx]
	fabs
	mov bx, @res
	fst dword ptr[bx]
	ret
rabs endp 



;float sqrt(float *num, float *res)
;************************************
; return error code -> al
;
;	al == 0 -> Ok
;	al == 3 -> num < 0
;
;************************************
sqrt proc c
	arg @num, @res
	uses bx
	mov bx, @num
	fld dword ptr[bx]
	fldz
	fcom
	fstsw ax
	sahf
	fsqrt
	jnl @@ns
	mov al, 1
	jmp @@end
@@ns:
	mov bx, @res
	fst dword ptr[bx]
	xor al, al
@@end:
	ret
sqrt endp 



;void frexp(float *num, float *m, int *p)
frexp proc c
	arg @num, @m, @p
	uses bx
	mov bx, @num
	fld dword ptr[bx]
	fxtract
	mov bx, @m
	fstp dword ptr[bx]
	mov bx, @p
	fstp dword ptr[bx]
	push @p
	call floor
	add sp, 2
	ret
frexp endp