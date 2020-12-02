global _start
section .bss
	NBUFFER resb 5
	AUX_INPUT resd 1
	OLD_DATA resd 1
	NEW_DATA resd 1
	TMP_DATA resd 1
section .data
	DOIS dd 2
	_msg_input db 'Numero de caracteres lidos: '
	_msg_input_size equ $-_msg_input
	_breakline db '', 0DH, 0AH
	_breakline_size equ $-_breakline
	_msg_overflow db 'Overflow!!', 0DH, 0AH
	_msg_overflow_size equ $-_msg_overflow
section .text
_start:
	mov ecx, OLD_DATA
	mov edx, 5
	call LeerString
	mov esi, OLD_DATA
	call ConverteInteiro
	mov [OLD_DATA], eax
	mov eax, [OLD_DATA]
L1:
	cdq
	mov ecx, [DOIS]
	idiv ecx
	mov [NEW_DATA], eax
	mov ecx, [DOIS]
	imul ecx
	jo _overflow_err
	mov [TMP_DATA], eax
	mov eax, [OLD_DATA]
	sub eax, [TMP_DATA]
	mov [TMP_DATA], eax
	mov eax, [TMP_DATA]
	lea esi, [NBUFFER]
	call EscreverInteiro
	mov ebx, [NEW_DATA]
	mov [OLD_DATA], ebx
	mov eax, [OLD_DATA]
	cmp eax, 0
	jg L1
	mov eax, 1
	mov ebx, 0
	int 80h




;########### PROCEDURES ###########


;#### INICIO DA ROTINA PARA LER CHAR ####
LeerChar:
	enter 0,0
	mov eax, 3
	mov ebx, 0
	mov edx, 2
	int 80h
	mov [AUX_INPUT], eax
	mov ecx, _msg_input
	mov edx, _msg_input_size
	call EscreverString
	mov eax, [AUX_INPUT]
	lea esi, [NBUFFER]
	call EscreverInteiro
	mov ecx, _breakline
	mov edx, _breakline_size
	call EscreverString
	mov eax, [AUX_INPUT]
	leave
	ret
;########################################
;#### INICIO DA ROTINA P ESCREVER CHAR ####
EscreverChar:
	enter 0,0
	mov eax, 4
	mov ebx, 1
	mov edx, 2
	int 80h
	leave
	ret
;##########################################


;#### INICIO DA ROTINA PARA LER STRING ####
LeerString:
	enter 0,0
	mov eax, 3
	mov ebx, 0
	int 80h
	mov [AUX_INPUT], eax
	mov ecx, _msg_input
	mov edx, _msg_input_size
	call EscreverString
	mov eax, [AUX_INPUT]
	lea esi, [NBUFFER]
	call EscreverInteiro
	mov ecx, _breakline
	mov edx, _breakline_size
	call EscreverString
	mov eax, [AUX_INPUT]
	leave
	ret
;########################################


;#### INICIO DA ROTINA P ESCREVER STRING ####
EscreverString:
	enter 0,0
	mov eax, 4
	mov ebx, 1
	int 80h
	leave
	ret
;##########################################


;#### INICIO DA ROTINA P ESCREVER INT ####
EscreverInteiro:
	enter 0,0
	mov BYTE [esi], 0
	mov BYTE [esi+1], 0
	mov BYTE [esi+2], 0
	mov BYTE [esi+3], 0
	mov BYTE [esi+4], 0
	mov edx, 0
	; flag de numero negativo
	mov edi, 0
	cmp eax, 0
	jnl _init
	; tratamento para negativo
	sub edx, eax
	mov eax, edx
	mov edi, 1
	mov edx, 0
_init:
	add esi, 5
	dec esi
	mov ebx, 10
_convert:
	xor edx, edx
	div ebx
	add dl, 48
	dec esi
	mov [esi], dl
	test eax, eax
	jne _convert
	cmp edi, 0
	je _print_int
	; se o numero for negativo
	xor edx, edx
	add dl, '-'
	dec esi
	mov [esi], dl
_print_int:
	mov dl, BYTE [_breakline + 1]
	mov [NBUFFER + 4], dl
	mov eax, 4
	mov ebx, 1
	mov ecx, NBUFFER
	mov edx, 5
	int 80h
	leave
	ret
;#######################################


;#### INICIO DA ROTINA P CONVERTER STR P INT ####
ConverteInteiro:
	mov eax, 0
	mov ebx, 10
	mov edx, 0
	; flag numero negativo
	mov edi, 0
	movzx edx, BYTE [esi]
	cmp edx, '-'
	jne _convert2
	; tratamento para negativo
	inc esi
	mov edi, 1
_convert2:
	movzx edx, BYTE [esi]
	inc esi
	cmp edx, 48
	jl _neg_test
	cmp edx, 57
	jg _neg_test
	; se ainda for um numero
	sub edx, 48
	imul eax, ebx
	add eax, edx
	jmp _convert2
_neg_test:
	cmp edi, 0
	je _return
	; se for numero negativo
	mov edx, 0
	sub edx, eax
	mov eax, edx
_return:
	ret
;##########################################
;#### INICIO DA ROTINA DE OVERFLOW ####
_overflow_err:
	mov eax, 4
	mov ebx, 1
	mov ecx, _msg_overflow
	mov edx, _msg_overflow_size
	int 80h
	mov eax, 1
	mov ebx, 0
	int 80h
;######################################
