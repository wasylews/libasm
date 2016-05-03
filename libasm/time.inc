; libasm - partial implementation of libc in tasm
; uses CDECL as calling convention
; Copyright: 2016, Wasylews, S.Habor
; License: MIT


; getdate() -> cx(year), dh(month), dl(day), al(day of week)
getdate macro
    mov ah, 2ah
    int 21h
endm


; gettime() -> ch(hours), cl(minutes), dh(seconds)
gettime macro
    mov ah, 2ch
    int 21h
endm


;;; WARN: setdate() and settime() doesn't work on dosbox

; setdate(word year, byte month, byte day, byte dayOfWeek)
setdate macro year, month, day, dayOfWeek
ifnb <year>
    mov cx, year
endif
ifnb <month>
    mov dh, month
endif
ifnb <day>
    mov dl, day
endif
ifnb <dayOfWeek>
    mov al, dayOfWeek
endif
    mov ah, 2bh
    int 21h
endm


; settime(byte hours, byte minutes, byte seconds)
settime macro
ifnb <hours>
   mov ch, hours
endif
ifnb <minutes>
    mov cl, minutes
endif
ifnb <seconds>
    mov dh, seconds
endif
    mov ah, 2dh
    int 21h
endm
