add bx, 30000 | CF unset | ZF unset
add bx, 10000 | CF set | ZF unset
adc bx, 5000 | ZF unset
adc bx, 5000 | ZF unset
mov bx, 1
mov cx, 100
add bx, cx | CF set | ZF unset
mov dx, 10
sub cx, dx | CF set | ZF unset
add bx, -25536