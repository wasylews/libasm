; libasm - partial implementation of libc in tasm
; uses CDECL as calling convention
; Copyright: 2016, Wasylews, S.Habor
; License: MIT


;int isupper(int c)
;=============================================
;	if(c >= 'A' && c <= 'Z') ax = true;
;	else ax = false;
;=============================================
isupper proc c
	arg @c
	mov ax, @c
	cmp al, 'A'
	jb @@false
	cmp al, 'Z'
	ja @@false
	mov ax, 1
	ret
@@false:
	xor ax, ax
	ret
isupper endp


;int islower(int c)
;=============================================
;	if(c >= 'a' && c <= 'z') ax = true;
;	else ax = false;
;=============================================
islower proc c
	arg @c
	mov ax, @c
	cmp al, 'a'
	jb @@false
	cmp al, 'z'
	ja @@false
	mov ax, 1
	ret
@@false:
	xor ax, ax
	ret
islower endp


;int isdigit(int c)
;=============================================
;	if(c >= '0' && c <= '9') ax = true;
;	else ax = false;
;=============================================
isdigit proc c
	arg @c
	mov ax, @c
	cmp al, '0'
	jb @@false
	cmp al, '9'
	ja @@false
	mov ax, 1
	ret
@@false:
	xor ax, ax
	ret
isdigit endp


;int isxdigit(int c)
;=============================================
;	if((c >= '0' && c <= '9') ||(c >= 'A' && c <= 'F') ||
;	   (c >= 'a' && c <= 'f') ax = true;
;	else ax = false;
;=============================================
isxdigit proc c
	arg @c
	mov ax, @c
	cmp al, '0'
	jb @@false
	cmp al, '9'
	jbe @@true
	cmp al, 'A'
	jb @@false
	cmp al, 'F'
	jbe @@true
	cmp al, 'a'
	jb @@false
	cmp al, 'f'
	ja @@false
@@true:
	mov ax, 1
	ret
@@false:
	xor ax, ax
	ret
isxdigit endp


;int isalpha(int c)
;=============================================
;	if(isupper(c) || islover(c)) ax = true;
;	else ax = false;
;=============================================
isalpha proc c
	arg @c
	push @c
	call isupper
	cmp ax, 0
	jnz @@return
	call islower
@@return:
	add sp, 2
	ret
isalpha endp


; int isalnum(int c) -> ax
;=============================================
;	if(isalpha(c) || isdigit(c)) ax = true;
;	else ax = false;
;=============================================
isalnum proc c
	arg @c
	push @c
	call isalpha
	cmp ax, 0
	jnz @@return
	call isdigit
@@return:
	add sp, 2
	ret
isalnum endp


;int isspace(int c)
;=============================================
;	if(c == '\v' || c == '\n' || c == '\t' ||
;	   c == '\r' || c == '\f' || c == ' ') ax = true;
;	else ax = false;
;=============================================
isspace proc c
	arg @c
	mov ax, @c
	cmp al, 9h
	jb @@false
	cmp al, 0dh
	jbe @@true
	cmp al, ' '
	jne @@false
@@true:
	mov ax, 1
	ret
@@false:
	xor ax, ax
	ret
isspace endp


;int iscntrl(int c)
;=============================================
;	if(c <= 1fh) ax = true;
;	else ax = false;
;=============================================
iscntrl proc c
	arg @c
	mov ax, @c
	cmp al, 1fh
	ja @@false
	mov ax, 1
	ret
@@false:
	xor ax, ax
	ret
iscntrl endp


;int ispunct(int c)
;=============================================
;	if((c == {!"#$%&'()*+,=./:;<=>?@[]^_`{|}~) ax = true;
;	else ax = false;
;=============================================
ispunct proc c
	arg @c
	mov ax, @c
	cmp al, 21h
	jb @@false
	cmp al, 2fh
	jbe @@true
	cmp al, 3ah
	jb @@false
	cmp al, 40h
	jbe @@true
	cmp al, 5bh
	jb @@false
	cmp al, 60h
	jbe @@true
	cmp al, 7bh
	jb @@false
	cmp al, 7eh
	ja @@false
@@true:
	mov ax, 1
	ret
@@false:
	xor ax, ax
	ret
ispunct endp


; int isgraph(int c) -> ax
;=============================================
;	if(c >= '!' && c <= '~') ax = true;
;	else ax = false;
;=============================================
isgraph proc c
	arg @c
	mov ax, @c
	cmp al, '!'
	jb @@false
	cmp al, '~'
	ja @@false
	mov ax, 1
	ret
@@false:
	xor ax, ax
	ret
isgraph endp


; int isprint(int c) -> ax
;=============================================
;	if(c >= ' ' && c <= '~') ax = true;
;	else ax = false;
;=============================================
isprint proc c
	arg @c
	mov ax, @c
	cmp al, ' '
	jb @@false
	cmp al, '~'
	ja @@false
	mov ax, 1
	ret
@@false:
	xor ax, ax
	ret
isprint endp


; char toupper(char c) -> al
toupper macro c
    mov ax, c
    cmp al, 'a'
    jb @@end
    cmp al, 'z'
    ja @@end
    sub al, 32
@@end:
endm


; char tolower(char c) -> al
tolower macro c
    mov ax, c
    cmp al, 'A'
    jb @@end
    cmp al, 'Z'
    ja @@end
    add al, 32
@@end:
endm


; int isblank(int ch)
;=============================================
;				C99
;	if(isspace(c)) ax = true;
;	else ax = false;
;=============================================
isblank proc c
	arg @c
	push @c
	call isspace
	add sp, 2
	ret
isblank endp


; char *_toupper(char *s) -> ax
_toupper proc c
    uses bx, dx
    arg @s
    mov bx, @s
@@beg:
    mov dl, [bx]
    toupper dx
    add sp, 2
    mov [bx], al
    inc bx
    cmp byte ptr[bx], 0
    jne @@beg
	mov ax, @s
    ret
_toupper endp

 
; char *_tolower(char *s) -> ax
_tolower proc c
    uses bx
    arg @s
    mov bx, @s
@@beg:
    mov al, [bx]
	tolower ax
    mov [bx], al
    inc bx
    cmp byte ptr[bx], 0
    jne @@beg
	mov ax, @s
    ret
_tolower endp