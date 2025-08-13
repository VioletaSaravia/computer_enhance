import glob
import sys
import numpy as np
import filecmp as fc


listing = "39" if len(sys.argv) < 2 else sys.argv[1]
test_print = len(sys.argv) > 2 and sys.argv[2] == "print"

binaries = [
    file
    for file in glob.glob("perfaware/part1/**")
    if file.find(".") == -1 and file.find(listing) != -1
]

out = open(f"homework/result_{listing}.asm", "wb")


def printb(*args):
    global out
    print(*args, end="")

    out.write(format(*args).encode("utf-8"))


def finished():
    out.close()
    exit()


with open(binaries[0], "rb") as f:
    binary = f.read()


def bits(char):
    result = []
    for i in range(8):
        bit = char & (1 << i)
        result.append(int(bit >> i))
    result.reverse()
    return result


def print_byte(i):
    printb(f"0x{i:02X} 0b{i:08b}\n")


if test_print:
    for i in binary:
        print_byte(i)

current_byte = 0


def get_byte():
    global current_byte
    if current_byte >= len(binary):
        finished()

    result = binary[current_byte]
    if test_print:
        printb(f"/{result:08b}/ ")
    current_byte += 1
    return result


def decode_reg(bits: int, w) -> str:
    to_reg = ""

    match bits:
        case 0b000:
            to_reg = "AL" if w == 0 else "AX"
        case 0b001:
            to_reg = "CL" if w == 0 else "CX"
        case 0b010:
            to_reg = "DL" if w == 0 else "DX"
        case 0b011:
            to_reg = "BL" if w == 0 else "BX"
        case 0b100:
            to_reg = "AH" if w == 0 else "SP"
        case 0b101:
            to_reg = "CH" if w == 0 else "BP"
        case 0b110:
            to_reg = "DH" if w == 0 else "SI"
        case 0b111:
            to_reg = "BH" if w == 0 else "DI"
        case other:
            print(f"Couldn't match reg/rm {other:08b}")

    return to_reg


def decode_mov_data(w: int) -> np.int8 | np.int16:
    byte1 = get_byte()
    byte2 = 0 if w == 0 else get_byte()

    if w == 1:
        result = np.int16(np.uint16(byte2 << 8 | byte1))
    else:
        result = np.int8(np.uint8(byte1))

    return result


def decode_add_data(w, s: int) -> np.int8 | np.int16:
    byte1 = get_byte()
    byte2 = 0 if w == 0 or s == 1 else get_byte()

    if w == 1 and s == 0:
        result = np.int16(np.uint16(byte2 << 8 | byte1))
    else:
        result = np.int8(np.uint8(byte1))

    return result


while True:
    match bits(get_byte()):
        case [0, 1, 1, 1, j1, j2, j3, j4]:
            jump_op = (j1 << 3) | (j2 << 2) | (j3 << 1) | j4

            op = ""
            match jump_op:
                case 0b0000: op = "jo"
                case 0b0001: op = "jno"
                case 0b0010: op = "jb"
                case 0b0011: op = "jnb"
                case 0b0100: op = "jl"
                case 0b0101: op = "jne"
                case 0b0110: op = "jbe"
                case 0b0111: op = "jnbe"
                case 0b1000: op = "js"
                case 0b1001: op = "jns"
                case 0b1010: op = "jp"
                case 0b1011: op = "jnp"
                case 0b1100: op = "jle"
                case 0b1101: op = "jnl"
                case 0b1111: op = "jnle"

            printb(f"{op} 0x{get_byte():02X}")

        case [1, 1, 1, 0, 0, 0, o1, o2]:
            opcode = (o1 << 1) | o2

            op = ""
            match opcode:
                case 0b00: op = "loopnz"
                case 0b01: op = "loopz"
                case 0b10: op = "loop"
                case 0b11: op = "jcxz"

            printb(f"{op} 0x{get_byte():02X}")

        case [0, 0, a1, a2, a3, 1, 0, w]:
            arithmetic = (a1 << 2) | (a2 << 1) | a3

            imm = decode_mov_data(w)
            reg = "AX" if w == 1 else "AL"

            op = ""
            match arithmetic:
                case 0b000:
                    op = "add"
                case 0b101:
                    op = "sub"
                case other:
                    op = "cmp"

            printb(f"{op} {reg}, {imm}")

        case [1, 0, 0, 0, 0, 0, s, w]:
            byte = get_byte()
            arithmetic = (byte & 0b00011100) >> 2

            match arithmetic:
                case 0b010:
                    printb("adc ")
                case 0b101:
                    printb("sub ")
                case 0b111:
                    printb("cmp ")
                case other:
                    printb("add ")

            mod = byte >> 6
            rm = byte & 0b00000111

            reg02 = ""
            if mod == 0b11:
                reg02 = decode_reg(rm, w)
            else:
                match rm:
                    case 0b000:
                        reg02 = "[BX + SI"
                    case 0b001:
                        reg02 = "[BX + DI"
                    case 0b010:
                        reg02 = "[BP + SI"
                    case 0b011:
                        reg02 = "[BP + DI"
                    case 0b100:
                        reg02 = "[SI"
                    case 0b101:
                        reg02 = "[DI"
                    case 0b110:
                        reg02 = "[BP"  # FIXME: ADRESS == BP ?
                    case 0b111:
                        reg02 = "[BX"

                if mod == 0b01:
                    d8 = get_byte()
                    reg02 += f" + {d8}" if d8 != 0 else ""
                if mod == 0b10:
                    d16lo = get_byte()
                    d16hi = get_byte()
                    d16 = d16hi << 8 | d16lo
                    reg02 += f" + {d16}" if d16 != 0 else ""

                reg02 += "]"

            reg01 = decode_add_data(w, s)

            printb(f"{reg02}, {reg01}")

        case [0, 0, a1, a2, a3, 0, d, w]:
            arithmetic = (a1 << 2) | (a2 << 1) | a3

            match arithmetic:
                case 0b101:
                    printb("sub ")
                case 0b111:
                    printb("cmp ")
                case other:
                    printb("add ")

            byte = get_byte()
            mod = byte >> 6
            reg = (byte & 0b00111000) >> 3
            rm = byte & 0b00000111

            reg01 = decode_reg(reg, w)
            reg02 = ""
            if mod == 0b11:
                reg02 = decode_reg(rm, w)
            else:
                match rm:
                    case 0b000:
                        reg02 = "[BX + SI"
                    case 0b001:
                        reg02 = "[BX + DI"
                    case 0b010:
                        reg02 = "[BP + SI"
                    case 0b011:
                        reg02 = "[BP + DI"
                    case 0b100:
                        reg02 = "[SI"
                    case 0b101:
                        reg02 = "[DI"
                    case 0b110:
                        reg02 = "[BP"  # FIXME: ADRESS == BP ?
                    case 0b111:
                        reg02 = "[BX"

                if mod == 0b01:
                    d8 = get_byte()
                    reg02 += f" + {d8}" if d8 != 0 else ""
                if mod == 0b10:
                    d16lo = get_byte()
                    d16hi = get_byte()
                    d16 = d16hi << 8 | d16lo
                    reg02 += f" + {d16}" if d16 != 0 else ""

                reg02 += "]"

            if d == 0:
                printb(f"{reg02}, {reg01}")
            if d == 1:
                printb(f"{reg01}, {reg02}")

        case [1, 0, 0, 0, 1, 0, d, w]:
            printb("mov ")

            byte = get_byte()
            mod = byte >> 6
            reg = (byte & 0b00111000) >> 3
            rm = byte & 0b00000111

            reg01 = decode_reg(reg, w)
            reg02 = ""
            if mod == 0b11:
                reg02 = decode_reg(rm, w)
            else:
                match rm:
                    case 0b000:
                        reg02 = "[BX + SI"
                    case 0b001:
                        reg02 = "[BX + DI"
                    case 0b010:
                        reg02 = "[BP + SI"
                    case 0b011:
                        reg02 = "[BP + DI"
                    case 0b100:
                        reg02 = "[SI"
                    case 0b101:
                        reg02 = "[DI"
                    case 0b110:
                        reg02 = "[BP"  # FIXME: ADRESS == BP ?
                    case 0b111:
                        reg02 = "[BX"

                if mod == 0b01:
                    d8 = get_byte()
                    reg02 += f" + {d8}" if d8 != 0 else ""
                if mod == 0b10:
                    d16lo = get_byte()
                    d16hi = get_byte()
                    d16 = d16hi << 8 | d16lo
                    reg02 += f" + {d16}" if d16 != 0 else ""

                reg02 += "]"

            if d == 0:
                printb(f"{reg02}, {reg01}")
            if d == 1:
                printb(f"{reg01}, {reg02}")

        case [1, 0, 1, 1, w, r1, r2, r3]:
            printb("mov ")
            reg = r3 + (r2 << 1) + (r1 << 2)

            reg01 = decode_reg(reg, w)
            printb(f"{reg01}, ")

            data = decode_mov_data(w)
            printb(f"{data}")

        case other:
            print(f"\nCouldn't parse byte {other}")
            finished()

    printb("\n")
