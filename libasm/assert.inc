; libasm - partial implementation of libc in tasm
; uses CDECL as calling convention
; Copyright: 2016, Wasylews, S.Habor
; License: MIT


; to turn off the test declare macro
; 		NDEBUG equ 0
; before the announcement
;		include assert.inc


	_errs db tab, 'ERROR!!!', endl, 0


; void assert(int expression)
assert macro expression: req
ifndef NDEBUG
	ife expression
		uses ax
		push ds
		mov ax, cs
		mov ds, ax
		puts _errs
		pop ax
		mov ds, ax
		abort
	endif
endif
endm