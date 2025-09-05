mov cx, 200	; Flags = 0b00000000
mov bx, cx	; Flags = 0b00000000
add cx, 1000	; Flags = 0b00000001
mov bx, 2000	; Flags = 0b00000001
sub cx, bx	; Flags = 0b10000001
