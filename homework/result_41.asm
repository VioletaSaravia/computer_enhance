add BX, [BX + SI]
add BX, [BP]
add SI, 2
add BP, 2
add CX, 8
add BX, [BP]
add CX, [BX + 2]
add BH, [BP + SI + 4]
add DI, [BP + DI + 6]
add [BX + SI], BX
add [BP], BX
add [BP], BX
add [BX + 2], CX
add [BP + SI + 4], BH
add [BP + DI + 6], DI
add [BX], 34
add [BP + SI + 1000], 29
add AX, [BP]
add AL, [BX + SI]
add AX, BX
add AL, AH
add AX, 1000
add AL, -30
add AL, 9
sub BX, [BX + SI]
sub BX, [BP]
add SI, 2
add BP, 2
adc CX, 8
sub BX, [BP]
sub CX, [BX + 2]
sub BH, [BP + SI + 4]
sub DI, [BP + DI + 6]
sub [BX + SI], BX
sub [BP], BX
sub [BP], BX
sub [BX + 2], CX
sub [BP + SI + 4], BH
sub [BP + DI + 6], DI
add [BX], 34
adc [BX + DI], 29
sub AX, [BP]
sub AL, [BX + SI]
sub AX, BX
sub AL, AH
sub AX, 1000
sub AL, -30
sub AL, 9
cmp BX, [BX + SI]
cmp BX, [BP]
cmp SI, 2
cmp BP, 2
add CX, 8
cmp BX, [BP]
cmp CX, [BX + 2]
cmp BH, [BP + SI + 4]
cmp DI, [BP + DI + 6]
cmp [BX + SI], BX
cmp [BP], BX
cmp [BP], BX
cmp [BX + 2], CX
cmp [BP + SI + 4], BH
cmp [BP + DI + 6], DI
cmp [BX], 34
cmp [BP], -30
add BL, [DI]
cmp AX, [BP]
cmp AL, [BX + SI]
cmp AX, BX
cmp AL, AH
cmp AX, 1000
cmp AL, -30
cmp AL, 9
jne 0x02
jne 0xFC
jne 0xFA
jne 0xFC
jl 0xFE
jle 0xFC
 0xFA
jb 0xF8
jbe 0xF6
jp 0xF4
jo 0xF2
js 0xF0
jne 0xEE
jnl 0xEC
jnle 0xEA
jnb 0xE8
jnbe 0xE6
jnp 0xE4
jno 0xE2
jns 0xE0
loop 0xDE
loopz 0xDC
loopnz 0xDA
jcxz 0xD8
