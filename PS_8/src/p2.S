BITS 64
        
        section .data
str:    db      "Hi", 10
str_len:equ     $ - str

        section .text
        global  _start
_start:
        mov     rax, 1       ; write
        mov     rdi, 1       ; stdout
        mov     rsi, str     ; string
        mov     rdx, str_len ; length of string
        syscall

        mov     eax, 60      ; exit
        xor     rdi, 0       ; 0
        syscall
