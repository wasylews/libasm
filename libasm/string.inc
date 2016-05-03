; libasm - partial implementation of libc in tasm
; uses CDECL as calling convention
; Copyright: 2016, Wasylews, S.Habor
; License: MIT


; TODO: char *strerror(int)
; TODO: char *strtok(char *, const char *)
; TODO: size_t strxfrm(char *dest, const char *src, size_t n)


; int strlen(char *s) -> ax
strlen proc c
    uses bx
    arg @s

    mov bx, @s
    xor ax, ax

@@count_loop:
    cmp byte ptr[bx], 0
    je @@end_loop

    inc ax
    inc bx
    jmp @@count_loop

@@end_loop:
    ret
strlen endp


; int strcmp(char *s1, char *s2) -> ax
strcmp proc c
    arg @s1, @s2
    uses bx, si, di

    mov si, @s1
    mov di, @s2

@@strcmp_loop:
    mov al, [si]
    mov bl, [di]

    sub al, bl
    jne @@strcmp_done			; return if different
    cmp bl, 0					; on end of both strings, ax = 0
    je @@strcmp_done
    inc si
    inc di
    jmp @@strcmp_loop

@@strcmp_done:
    cbw 						; extend al to ax
    ret
strcmp endp


; int strncmp(const char *s1, const char *s2, size_t n) -> ax
strncmp proc c
	arg @s1, @s2, @n
	push @n
	push @s2
	push @s1
	call memcmp
	add sp, 6
	ret
strncmp endp


; int strcoll(const char *s1, const char *s2) -> ax
strcoll proc c
	arg @s1, @s2
	push @s2
	push @s1
	call memcmp
	add sp, 4
	ret
strcoll endp


; char *strcat(char *dest, const char *src) -> ax
strcat proc c
	arg @dest, @src
	uses si, di
	mov si, @dest
	push si
	call strlen
	add sp, 2
	add si, ax
	mov di, @src
@@cycle:
	mov al, [di]
	cmp al, 0
	je @@end
	mov [si], al
	inc si
	inc di
	jmp @@cycle
@@end:
	mov byte ptr[si], 0
	mov ax, @dest
	ret
strcat endp


; char *strncat(char *dest, const char *src, size_t n) -> ax
strncat proc c
	arg @dest, @src, @n
	uses si, di
	mov si, @dest
	push si
	call strlen
	add sp, 2
	add si, ax
	mov di, @src
	mov cx, @n
@@cycle:
	cmp cx, 0
	je @@end
	mov al, [di]
	mov [si], al
	inc si
	inc di
	dec cx
	jmp @@cycle
@@end:
	mov byte ptr[si], 0
	mov ax, @dest
	ret
strncat endp


; char *strcpy(char *dest, const char *src) *dest -> ax
strcpy proc c
	arg @destr, @src
	uses si, di
	mov si, @src
	mov di, @dest
@@cycle:
	mov al, [si]
	cmp al, 0
	je @@end
	mov [di], al
	inc si
	inc di
	jmp @@cycle
@@end:
	mov byte ptr[di], 0
	mov ax, @dest
	ret
strcpy endp


; char *strncpy(char *dest, const char *src, size_t n) *dest -> ax
strncpy proc c
	arg @dest, @src
	uses bx
	push @n
	push @src
	push @dest
	call memcpy
	add sp, 6
	mov bx, @dest
	add bx, @n
	mov byte ptr[bx], 0
	ret
strncpy endp


; char *strpbrk(const char *s, const char *accept) -> ax
strpbrk proc c
	arg @s, @accept
	uses si, di
	mov si, @s
@@cycle1:
	mov al, [si]
	cmp al, 0
	je @@not_found
	mov di, @accept
@@cycle2:
	mov ah, [di]
	inc di
	cmp ah, 0
	je @@ns
	cmp ah, al
	jne @@cycle2
	mov ax, si
	jmp @@end
@@ns:
	inc si
	jmp @@cycle1
@@not_found:
	mov ax, ax
@@end:
	ret
strpbrk endp


; size_t strcspn(const char *s, const char *accept) -> ax
strcspn proc c
	arg @s, @accept
	push @accept
	push @s
	call strpbrk
	add sp, 4
	cmp ax, 0
	jne @@found
	push @s
	call strlen
	add sp, 2
	ret
@@found:
	sub ax, @s
	ret
strcspn endp


; size_t strspn(const char *s, const char *accept) -> ax
strspn proc c
	arg @s, @accept
	uses bx, cx
	xor cx, cx
	mov bx, @s
	push @accept
@@cycle:
	push bx
	call strpbrk
	add sp, 2
	cmp ax, 0
	je @@end
	cmp ax, bx
	jne @@end
	inc bx
	inc cx
	jmp @@cycle
@@end:
	add sp, 2
	mov ax, cx
	ret
strspn endp


; char *strchr(const char *s, int c) -> ax
strchr proc c
	arg @s, @c
	uses bx
	mov bx, @s
	mov ax, @c
	mov ah, al
@@cycle:
	mov al, [bx]
	cmp al, ah
	je @@end
	inc bx
	cmp al, 0
	jne @@cycle
	xor bx, bx
@@end:
	mov ax, bx
	ret
strchr endp


; char *strrchr(const char *s, int c) -> ax
strrchr proc c
	arg @s, @c
	uses bx, si
	push @s
	call strlen
	add sp, 2
	inc ax
	mov si, ax
	mov bx, @s
	mov ax, @c
	mov ah, al
@@cycle:
	cmp si, -1
	je @@not_found
	mov al, [bx+si]
	dec si
	cmp al, ah
	jne @@cycle
	mov ax, bx
	inc ax
	add ax, si
	jmp @@end
@@not_found:
	xor ax, ax
@@end:
	ret
strrchr endp


; char *strstr(const char *s1, const char *s2)
strstr proc c
	arg @s1, @s2
	uses bx, cx, dx, si
	push @s2
	call strlen
	add sp, 2
	mov cx, ax					; cx = strlen(s2)
	mov si, @s1					; si = *s1
	push 0
	push si
	call strchr
	add sp, 4
	mov di, ax					; di = *s1[end]
	mov bx, @s2
	mov dl, [bx]				; dl = s2[0]
@@cycle:
	mov ax, si
	add ax, cx
	cmp ax, di
	ja @@not_found
	push dx
	push si
	call strchr
	add sp, 4
	cmp ax, 0
	jz @@not_found
	mov si, ax
	push cx
	push @s2
	push si
	call strncmp
	add sp, 6
	cmp ax, 0
	jz @@found
	inc si
	jmp @@cycle
@@found:
	mov ax, si
	ret
@@not_found:
	xor ax, ax
	ret
strstr endp


; void *memset(void *memptr, int val, size_t num )
memset proc c
	arg @memptr, @val, @num
	uses ax, bx, cx
	mov ax, @val
	mov bx, @memptr
	mov cx, @num
@@cycle:
	cmp cx, 0
	je @@end
	mov [bx], al
	inc bx
	dec cx
	jmp @@cycle
@@end:
	ret
memset endp


; void *memcpy(void *dest, const void *src, size_t n) *dest -> ax
memcpy proc c
	arg @dest, @src, @n
	uses cx, si, di
	mov cx, @n
	mov si, @src
	mov di, @dest
@@cycle:
	cmp cx, 0
	je @@end
	mov al, [si]
	mov [di], al
	inc si
	inc di
	dec cx
	jmp @@cycle
@@end:
	add sp, 4
	mov ax, @dest
	ret
memcpy endp


; void *memmove(void *dest, const void *src, size_t n)
memmove proc c
	arg @dest, @src, @n
	uses bx, cx
	mov cx, @n
	sub sp, cx
	mov bx, sp
	push cx
	push @src
	push bx
	call memcpy
	add sp, 4
	push bx
	push @dest
	call memcpy
	add sp, 6
	add sp, cx
	ret
memmove endp


; void *memchr(const char *s, char c, size_t n) -> ax
memchr proc c
	arg @s, @c, @n
	uses bx, cx
	mov bx, @s
	mov cx, @n
	mov ax, @c
	mov ah, al
@@cycle:
	cmp cx, 0
	je @@not_found
	mov al, [bx]
	inc bx
	dec cx
	cmp al, ah
	jne @@cycle
	dec bx
	mov ax, bx
	jmp @@end
@@not_found:
	xor ax, ax
@@end:
	ret
memchr endp


; int memcmp(const char *s1, const char *s2, size_t n) -> ax
memcmp proc c
	arg @s1, @s2, @n
	uses cx, si, di
	mov cx, @n
	mov si, @s1
	mov di, @s2
@@cycle:
    cmp cx, 0
    je @@e
	mov al, [si]
	mov ah, [di]
	sub al, ah
	jne @@strcmp_done
    inc si
    inc di
	dec cx
    jmp @@cycle
@@e:
	xor ax, ax
@@strcmp_done:
    cbw							; extend al to ax
    ret
memcmp endp