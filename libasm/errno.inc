; libasm - partial implementation of libc in tasm
; uses CDECL as calling convention
; Copyright: 2016, Wasylews, S.Habor
; License: MIT


	ERRNO = 0
	EDOM	equ	1	; Error domain - Ошибка области определения
	ERANGE	equ	2	; Error value range - Результат слишком велик
	;EILSEQ equ		; Error invalid sequence	Ошибочная последовательность байтов

;============================================================
;	Standard POSIX defines the following symbolic error names
;============================================================

	; E2BIG				equ	?	; Список аргументов слишком длинный
	; EACCES			equ	?	; Отказ в доступе
	; EADDRINUSE		equ	?	; Адрес используется
	; EADDRNOTAVAIL		equ	?	; Адрес недоступен
	; EAFNOSUPPORT		equ	?	; Семейство адресов не поддерживается
	; EAGAIN			equ	?	; Ресурс временно недоступен
	; EALREADY			equ	?	; Соединение уже устанавливается
	; EBADF				equ	?	; Неправильный дескриптор файла
	; EBADMSG			equ	?	; Неправильное сообщение
	; EBUSY				equ	?	; Ресурс занят
	; ECANCELED			equ	?	; Операция отменена
	; ECHILD			equ	?	; Нет дочернего процесса
	; ECONNABORTED		equ	?	; Соединение прервано
	; ECONNFRFUSED		equ	?	; Соединение отклонено
	; EDEADLK			equ	?	; Обход тупика ресурсов
	; EDESTADDRREQ		equ	?	; Требуется адрес назначения
	; EDOM				equ	?	; Ошибка области определения
	; EDQUOT			equ	?	; Зарезервировано
	; EEXIST			equ	?	; Файл существует
	; EFAULT			equ	?	; Неправильный адрес
	; EFBIG				equ	?	; Файл слишком велик
	; EHOSTUNREACH		equ	?	; Хост недоступен
	; EIDRM				equ	?	; Идентификатор удален
	; EILSEQ			equ	?	; Ошибочная последовательность байтов
	; EINPROGRESS		equ	?	; Операция в процессе выполнения
	; EINTR				equ	?	; Прерванный вызов функции
	; EINVAL			equ	?	; Неправильный аргумент
	; EIO				equ	?	; Ошибка ввода-вывода
	; EISCONN			equ	?	; Сокет (уже) соединен
	; EISDIR			equ	?	; Это каталог
	; ELOOP				equ	?	; Слишком много уровней символических ссылок
	; EMFILE			equ	?	; Слишком много открытых файлов
	; EMLINK			equ	?	; Слишком много связей
	; EMSGSIZE			equ	?	; Неопределённая длина буфера сообщения
	; EMULTIHOP			equ	?	; Зарезервировано
	; ENAMETOOLONG		equ	?	; Имя файла слишком длинное
	; ENETDOWN			equ	?	; Сеть не работает
	; ENETRESET			equ	?	; Соединение прервано сетью
	; ENETUNREACH		equ	?	; Сеть недоступна
	; ENFILE			equ	?	; Слишком много открытых файлов в системе
	; ENOBUFS			equ	?	; Буферное пространство недоступно
	; ENODEV			equ	?	; Нет такого устройства
	; ENOENT			equ	?	; Нет такого файла или каталога
	; ENOEXEC			equ	?	; Ошибка формата исполняемого файла
	; ENOLCK			equ	?	; Блокировка недоступна
	; ENOLINK			equ	?	; Зарезервировано
	; ENOMEM			equ	?	; Недостаточно памяти
	; ENOMSG			equ	?	; Сообщение нужного типа отсутствует
	; ENOPROTOOPT		equ	?	; Протокол недоступен
	; ENOSPC			equ	?	; Памяти на устройстве не осталось
	; ENOSYS			equ	?	; Функция не реализована
	; ENOTCONN			equ	?	; Сокет не соединен
	; ENOTDIR			equ	?	; Это не каталог
	; ENOTEMPTY			equ	?	; Каталог непустой
	; ENOTSOCK			equ	?	; Это не сокет
	; ENOTSUP			equ	?	; Не поддерживается
	; ENOTTY			equ	?	; Неопределённая операция управления вводом-выводом
	; ENXIO	Нет 		equ	?	;такого устройства или адреса
	; EOPNOTSUPP		equ	?	; Операция сокета не поддерживается
	; EOVERFLOW			equ	?	; Слишком большое значение для типа данных
	; EPERM				equ	?	; Операция не разрешена
	; EPIPE				equ	?	; Разрушенный канал
	; EPROTO			equ	?	; Ошибка протокола
	; EPROTONOSUPPORT	equ	?	; Протокол не поддерживается
	; EPROTOTYPE		equ	?	; Ошибочный тип протокола для сокета
	; ERANGE			equ	?	; Результат слишком велик
	; EROFS				equ	?	; Файловая система только на чтение
	; ESPIPE			equ	?	; Неправильное позиционирование
	; ESRCH				equ	?	; Нет такого процесса
	; ESTALE			equ	?	; Зарезервировано
	; ETIMEDOUT			equ	?	; Операция задержана
	; ETXTBSY			equ	?	; Текстовый файл занят
	; EWOULDBLOCK		equ	?	; Блокирующая операция
	; EXDEV				equ	?	; Неопределённая связь