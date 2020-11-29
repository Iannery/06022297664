section .text
global _start

_start:

; show prompt message
    mov     edx, 19     ; number of bytes to write - one for each letter plus 0Ah (line feed character)
    mov     ecx, txt_prompt    ; move the memory address of our message string into ecx
    mov     ebx, 1      ; write to the STDOUT file
    mov     eax, 4      ; invoke SYS_WRITE (kernel opcode 4)
    int     80h

; read the user input
    mov eax, 3          ; Read user input into str
    mov ebx, 0          ; |
    mov ecx, user_input        ; | <- destination
    mov edx, 100        ; | <- length
    int 80h             ; \

; calculate the length

    mov     ebx, user_input        ; move the address of our message string into EBX
    mov     eax, ebx        ; move the address in EBX into EAX as well (Both now point to the same segment in memory)
    mov     ecx, 0
strlen_nextchar:
    cmp     byte [eax], 0   ; compare the byte pointed to by EAX at this address against zero (Zero is an end of string delimiter)
    jz      strlen_finished        ; jump (if the zero flagged has been set) to the point in the code labeled 'finished'
    inc     eax             ; increment the address in EAX by one byte (if the zero flagged has NOT been set)
    jmp     strlen_nextchar        ; jump to the point in the code labeled 'strlen_nextchar'



strlen_finished:
    sub     eax, ebx        ; subtract the address in EBX from the address in EAX
    add     [reg], eax

; show output message
    mov     edx, 35     ; number of bytes to write - one for each letter plus 0Ah (line feed character)
    mov     ecx, txt_output    ; move the memory address of our message string into ecx
    mov     ebx, 1      ; write to the STDOUT file
    mov     eax, 4      ; invoke SYS_WRITE (kernel opcode 4)
    int     80h

    mov     edx, 35     ; number of bytes to write - one for each letter plus 0Ah (line feed character)
    mov     ecx, reg   ; move the memory address of our message string into ecx
    mov     ebx, 1      ; write to the STDOUT file
    mov     eax, 4      ; invoke SYS_WRITE (kernel opcode 4)
    int     80h

; exit code
    mov     ebx, 0
    mov     eax, 1
    int     80h

section .data
txt_prompt:   db      'Enter your message:', 0Ah
txt_output:   db      'The string length is calculated: 0', 0Ah
reg: dd '00000', 0Ah

section .bss
user_input: resb 100
