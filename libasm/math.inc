; libasm - partial implementation of libc in tasm
; uses CDECL as calling convention
; Copyright: 2016, Wasylews, S.Habor
; License: MIT


; TODO: random


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


; int round(float *num, int *res, bool rmode)
;************************************
;
;	rmode == 0 -> floor
;	rmode == 1 -> ceil
;
;************************************
round proc c
	arg @num, @res, @rmode
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
	mov bx, @res
	fist dword ptr[bx]
	ret
round endp


; int ceil(float *num, int *res)
ceil proc c
	arg @num, @res
	push 1
	push @res
	push @num
	call round
	add sp, 6
	ret
ceil endp


; int floor(float *num, int *res)
floor proc c
	arg @num, @res
	push 0
	push @res
	push @num
	call round
	add sp, 6
	ret
floor endp


; float rabs(float *num, float *res)
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


; float sqrt(float *num, float *res)
;************************************
; return error code -> al
;
;	al == 0 -> Ok
;	al == 1 -> num < 0
;
;************************************
sqrt proc c
	arg @num, @res
	uses bx
	mov bx, @num
	fld dword ptr[bx]
	ftst
	fstsw ax
	sahf
	jae @@ns
	mov al, 1
	ret
@@ns:
	fsqrt
	mov bx, @res
	fstp dword ptr[bx]
	xor al, al
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


;void modf(float *fnum, int *inum)
modf proc c
	arg @fnum, @inum
	uses bx
	mov bx, @fnum
	push @inum
	push bx
	fld dword ptr[bx]
	fldz
	fcompp
	fstsw ax
	sahf
	jc @@pos
	jz @@pos
	call ceil
	mov bx, @inum
	fild dword ptr[bx]
	fabs
	mov bx, @fnum
	fld dword ptr[bx]
	fchs
	fsub
	;fld1
	;fadd
	jmp @@endif
@@pos:
	call floor
	mov bx, @inum
	fisub dword ptr[bx]
@@endif:
	mov bx, @fnum
	fstp dword ptr[bx]
	ret
modf endp


; float log10(float *x, float *res) //lg
log10 proc c
	arg @x, @res
	uses bx
	fld1
	mov bx, @x
	fld	dword ptr[bx]
	fyl2x
	fldl2t
	fdiv
	mov bx, @res
	fstp dword ptr[bx]
	ret
log10 endp


; float log(float *x, float *res) //ln
log proc c
	arg @x, @res
	uses bx
	fld1
	mov bx, @x
	fld	dword ptr[bx]
	fyl2x
	fldl2e
	fdiv
	mov bx, @res
	fstp dword ptr[bx]
	ret
log endp


; float log(float *b, float *a, float *res) //log
_loga proc c
	arg @a, @b, @res
	uses bx
	sub sp, 4
	push sp
	push @a
	call log10
	add sp, 4

	sub sp, 4
	push sp
	push @b
	call log10
	add sp, 4

	mov bx, sp
	fld dword ptr[bx]
	add sp, 4
	mov bx, sp
	fld dword ptr[bx]
	add sp, 4
	fdiv

	mov bx, @res
	fstp dword ptr[bx]

	ret
_loga endp


; float sin(float *x, float *res)
sin proc c
	arg @x, @res
	uses bx
	mov bx, @x
	fld dword ptr[bx]
	fsin
	mov bx, @res
	fstp dword ptr[bx]
	ret
sin endp


; float cos(float *x, float *res)
cos proc c
	arg @x, @res
	uses bx
	mov bx, @x
	fld dword ptr[bx]
	fcos
	mov bx, @res
	fstp dword ptr[bx]
	ret
cos endp


; float tan(float *x, float *res)
tan proc c
	arg @x, @res
	uses bx
	mov bx, @x
	fld dword ptr[bx]
	fsincos
	fdiv
	mov bx, @res
	fstp dword ptr[bx]
	ret
tan endp


; float _cot(float *x, float *res)
_cot proc c
	arg @x, @res
	uses bx
	mov bx, @x
	fld dword ptr[bx]
	fsincos
	fdivr
	mov bx, @res
	fstp dword ptr[bx]
	ret
_cot endp


; float atan(float *x, float *res)
atan proc c
	arg @x, @res
	mov bx, @x
	fld dword ptr[bx]
	fld1
	fpatan
	mov bx, @res
	fstp dword ptr[bx]
	fstp st(0)
	ret
atan endp


; float atan2(float *y, float *x, float *res)
atan2 proc c
	arg @y, @x, @res
	mov bx, @y
	fld dword ptr[bx]
	mov bx, @x
	fld dword ptr[bx]
	fpatan
	mov bx, @res
	fstp dword ptr[bx]
	fstp st(0)
	ret
atan2 endp


; float asin(float *x, float *res)
asin proc c
	arg @x, @res
	uses bx
	mov bx, @x
	fld dword ptr[bx]
	fld st(0)
	fld st(0)
	fmul
	fld1
	fsubr
	fsqrt
	fpatan
	mov bx, @res
	fstp dword ptr[bx]
	fstp st(0)
	ret
asin endp


; float acos(float *x, float *res)
acos proc c
	arg @x, @res
	uses bx
	mov bx, @x
	fld1
	fld dword ptr[bx]
	fld st(0)
	fmul
	fsub
	fsqrt
	fld dword ptr[bx]
	fpatan
	mov bx, @res
	fstp dword ptr[bx]
	fstp st(0)
	ret
acos endp


; float _acot(float *x, float *res)
_acot proc c
	arg @x, @res
	uses bx
	mov bx, @x
	fld dword ptr[bx]
	fld1
	fpatan
	mov bx, @res
	mov dword ptr[bx], 2
	fild dword ptr[bx]
	fldpi
	fdivr
	fsubr
	mov bx, @res
	fstp dword ptr[bx]
	ret
_acot endp


; float _asec(float *x, float *res)
_asec proc c
	arg @x, @res
	uses bx
	mov bx, @x
	fld1
	fld dword ptr[bx]
	fdiv
	mov bx, @res
	fstp dword ptr[bx]
	push @res
	push @res
	call acos
	add sp, 4
	ret
_asec endp


; float _acosec(float *x, float *res)
_acosec proc c
	arg @x, @res
	uses bx
	mov bx, @x
	fld1
	fld dword ptr[bx]
	fdiv
	mov bx, @res
	fstp dword ptr[bx]
	push @res
	push @res
	call asin
	add sp, 4
	ret
_acosec endp


; float _sch(float *x, float *res, bool mode)
;************************************
;
;	rmode == 0 -> sinh
;	rmode == 1 -> _cosh
;
;************************************
_sch proc c
	arg @x, @res, @mode
	uses bx
	push @res
	push @x
	call exp
	mov bx, @x
	fld dword ptr[bx]
	fchs
	mov bx, sp
	fstp dword ptr[bx]
	push bx
	push bx
	call exp
	add sp, 4
	fld dword ptr[bx]
	mov dword ptr[bx], 2
	mov bx, @res
	fld dword ptr[bx]
	cmp @mode, 0
	je @@sinh
	fadd
	jmp @@ns
@@sinh:
	fsubr
@@ns:
	mov bx, sp
	fild dword ptr[bx]
	add sp, 4
	fdiv
	mov bx, @res
	fstp dword ptr[bx]
	ret
_sch endp


; float sinh(float *x, float *res)
sinh proc c
	arg @x, @res
	uses bx
	push 0
	push @res
	push @x
	call _sch
	add sp, 6
	ret
sinh endp


; float _cosh(float *x, float *res)
_cosh proc c
	arg @x, @res
	uses bx
	push 1
	push @res
	push @x
	call _sch
	add sp, 6
	ret
_cosh endp


; float tanh(float *x, float *res)
tanh proc c
	arg @x, @res
	uses bx
	push 0
	push @res
	push @x
	call _sch
	add sp, 6

	mov bx, @res
	fld dword ptr[bx]

	push 1
	push @res
	push @x
	call _sch
	add sp, 6

	fld dword ptr[bx]
	fdiv
	fstp dword ptr[bx]
	ret
tanh endp


; float _coth(float *x, float *res)
_coth proc c
	arg @x, @res
	uses bx
	push @res
	push @x
	call tanh
	add sp, 4
	fld1
	mov bx, @res
	fld dword ptr[bx]
	fdiv
	fstp dword ptr[bx]
	ret
_coth endp


; float _sech(float *x, float *res)
_sech proc c
	arg @x, @res
	uses bx
	push @res
	push @x
	call _cosh
	add sp, 4
	fld1
	mov bx, @res
	fld dword ptr[bx]
	fdiv
	fstp dword ptr[bx]
	ret
_sech endp


; float _cosech(float *x, float *res)
_cosech proc c
	arg @x, @res
	uses bx
	push @res
	push @x
	call sinh
	add sp, 4
	fld1
	mov bx, @res
	fld dword ptr[bx]
	fdiv
	fstp dword ptr[bx]
	ret
_cosech endp


; float exp(float *x, float *res)
exp proc c
	arg @x, @res
	uses bx
	sub sp, 4
	mov bx, sp
	mov dword ptr[bx], 402df867h ; e = 2.718281828459
	push @res
	push @x
	push bx
	call pow
	add sp, 10
	ret
exp endp


; float pow(float *x, float *y, float *res)
pow proc c
	arg @x, @y, @res
	uses bx

	; st0=X, st1=Y
	mov bx, @y
	fld dword ptr[bx]
	mov bx, @x
	fld dword ptr[bx]

	ftst                   ; st0=X=0 ?
	fstsw ax
	sahf
	jz	@@zero
	mov	bl, ah
	ja	@@positiveX

	fxch
	fld	st(0)
	frndint
	fcomp
	fstsw ax
	sahf
	jnz	@@error

	fld1
	fld1
	fadd
	fld	st(1)
	fprem
	ftst
	fstsw ax
	fstp st(0)
	fstp st(0)
	fxch
@@positiveX:
	fabs
	fyl2x
	fld	st(0)
	frndint
	fsub	st(1),st(0)
	fld1
	fscale
	fstp	st(1)
	fincstp
	f2xm1
	fmul	st(0),st(7)
	fdecstp
	fadd

	test	bl,1
	jz	@@end
	sahf
	jz	@@end
	fchs
@@end:
	mov bx, @res
	fstp dword ptr[bx]
	ret
@@error:
	fldz
	fstp	st(1)
	stc
@@zero:
	fstp	st(1)
	ret
pow endp