org 7C00h

print:
mov al, 'H'
mov ah, 0Eh
int 10h

mov al, 'E'
int 10h

mov al, 'L'
int 10h

mov al, 'L'
int 10h

mov al, 'O'
int 10h

mov al, ' '
int 10h

mov al, 'D'
int 10h

mov al, 'A'
int 10h

mov al, 'N'
int 10h

mov al, 'N'
int 10h

mov al, 'Y'
int 10h

mov al, ' '
int 10h

jmp print

jmp halt
halt: hlt

times 510-($-$$) nop
dw 0AA55h
