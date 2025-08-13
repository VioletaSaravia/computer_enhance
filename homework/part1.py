import glob
import sys
from typing import Annotated, List
import numpy as np
from dataclasses import dataclass

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
    print("\n", regs)
    exit()


with open(binaries[0], "rb") as f:
    binary = f.read()


def bits(char: int) -> Annotated[List[np.uint8], (8)]:
    result = []
    for i in range(8):
        bit = char & (1 << i)
        result.append(int(bit >> i))
    result.reverse()
    return result


def print_byte(i):
    printb(f"0x{i:02X} 0b{i:08b}\n")


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
            to_reg = "al" if w == 0 else "ax"
        case 0b001:
            to_reg = "cl" if w == 0 else "cx"
        case 0b010:
            to_reg = "dl" if w == 0 else "dx"
        case 0b011:
            to_reg = "bl" if w == 0 else "bx"
        case 0b100:
            to_reg = "ah" if w == 0 else "sp"
        case 0b101:
            to_reg = "ch" if w == 0 else "bp"
        case 0b110:
            to_reg = "dh" if w == 0 else "si"
        case 0b111:
            to_reg = "bh" if w == 0 else "di"
        case other:
            print(f"Couldn't match reg/rm {other:08b}")

    return to_reg


def decode_reg02(rm, mod, w):
    if mod == 0b11:
        return decode_reg(rm, w)

    result = ""
    match rm:
        case 0b000:
            result = "[BX + SI"
        case 0b001:
            result = "[BX + DI"
        case 0b010:
            result = "[BP + SI"
        case 0b011:
            result = "[BP + DI"
        case 0b100:
            result = "[SI"
        case 0b101:
            result = "[DI"
        case 0b110:
            result = "[BP"  # FIXME: ADRESS == BP ?
        case 0b111:
            result = "[BX"

    if mod == 0b01:
        d8 = get_byte()
        result += f" + {d8}" if d8 != 0 else ""
    if mod == 0b10:
        d16lo = get_byte()
        d16hi = get_byte()
        d16 = d16hi << 8 | d16lo
        result += f" + {d16}" if d16 != 0 else ""

    result += "]"
    return result


def decode_imm_data(w: int) -> np.int8 | np.int16:
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


def decode_arithmetic_op(code: int) -> str:
    match code:
        case 0b010:
            return "adc"
        case 0b101:
            return "sub"
        case 0b111:
            return "cmp"
        case _:
            return "add"


@dataclass
class Registers:
    ah = np.uint8(0)
    al = np.uint8(0)
    bh = np.uint8(0)
    bl = np.uint8(0)
    ch = np.uint8(0)
    cl = np.uint8(0)
    dh = np.uint8(0)
    dl = np.uint8(0)
    sp = np.uint16(0)
    bp = np.uint16(0)
    si = np.uint16(0)
    di = np.uint16(0)

    cf = False  # carry flag
    af = False  # aux carry flag
    sf = False  # sign flag
    zf = False  # zero flag
    pf = False  # parity flag
    of = False  # overflow flag

    def __repr__(self) -> str:
        return f"""
Registers(
    AX = 0x{self.ax:04X},
    BX = 0x{self.bx:04X},
    CX = 0x{self.cx:04X},
    DX = 0x{self.dx:04X},
    SP = 0x{self.sp:04X},
    BP = 0x{self.bp:04X},
    SI = 0x{self.si:04X},
    DI = 0x{self.di:04X},
)

Flags(
    CF = { "set" if self.cf else "unset" }
    AF = { "set" if self.af else "unset" }
    SF = { "set" if self.sf else "unset" }
    ZF = { "set" if self.zf else "unset" }
    PF = { "set" if self.pf else "unset" }
    OF = { "set" if self.of else "unset" }
)
"""

    def _get_ax(self) -> np.uint16:
        return np.uint16(np.uint16(self.al) << 8) | np.uint16(self.ah)

    def _set_ax(self, val: np.uint16):
        self.ah = np.uint8(val % 256)
        self.al = np.uint8((val >> 8) % 256)

    def _del_ax(self):
        self.ah = np.uint8(0)
        self.al = np.uint8(0)

    ax = property(fget=_get_ax, fset=_set_ax, fdel=_del_ax, doc="")

    def _get_bx(self) -> np.uint16:
        return np.uint16(np.uint16(self.bl) << 8) | np.uint16(self.bh)

    def _set_bx(self, vbl: np.uint16):
        self.bh = np.uint8(vbl % 256)
        self.bl = np.uint8((vbl >> 8) % 256)

    def _del_bx(self):
        self.bh = np.uint8(0)
        self.bl = np.uint8(0)

    bx = property(fget=_get_bx, fset=_set_bx, fdel=_del_bx, doc="")

    def _get_cx(self) -> np.uint16:
        return np.uint16(np.uint16(self.cl) << 8) | np.uint16(self.ch)

    def _set_cx(self, vcl: np.uint16):
        self.ch = np.uint8(vcl % 256)
        self.cl = np.uint8((vcl >> 8) % 256)

    def _del_cx(self):
        self.ch = np.uint8(0)
        self.cl = np.uint8(0)

    cx = property(fget=_get_cx, fset=_set_cx, fdel=_del_cx, doc="")

    def _get_dx(self) -> np.uint16:
        return np.uint16(np.uint16(self.dl) << 8) | np.uint16(self.dh)

    def _set_dx(self, vdl: np.uint16):
        self.dh = np.uint8(vdl % 256)
        self.dl = np.uint8((vdl >> 8) % 256)

    def _del_dx(self):
        self.dh = np.uint8(0)
        self.dl = np.uint8(0)

    dx = property(fget=_get_dx, fset=_set_dx, fdel=_del_dx, doc="")


# type Bit = np.bool
# type Byte = Annotated[npt.NDArray[np.bool], (8)]


def bits_to_byte(*args: int) -> int:
    result: int = 0
    for n, i in enumerate(args):
        result |= i << (len(args) - n - 1)
    return result


regs = Registers()
while True:
    match bits(get_byte()):
        case [0, 1, 1, 1, j1, j2, j3, j4]:
            jump_op = (j1 << 3) | (j2 << 2) | (j3 << 1) | j4

            op = ""
            match jump_op:
                case 0b0000:
                    op = "jo"
                case 0b0001:
                    op = "jno"
                case 0b0010:
                    op = "jb"
                case 0b0011:
                    op = "jnb"
                case 0b0100:
                    op = "jl"
                case 0b0101:
                    op = "jne"
                case 0b0110:
                    op = "jbe"
                case 0b0111:
                    op = "jnbe"
                case 0b1000:
                    op = "js"
                case 0b1001:
                    op = "jns"
                case 0b1010:
                    op = "jp"
                case 0b1011:
                    op = "jnp"
                case 0b1100:
                    op = "jle"
                case 0b1101:
                    op = "jnl"
                case 0b1111:
                    op = "jnle"

            printb(f"{op} 0x{get_byte():02X}")

        case [1, 1, 1, 0, 0, 0, o1, o2]:
            opcode = (o1 << 1) | o2

            op = ""
            match opcode:
                case 0b00:
                    op = "loopnz"
                case 0b01:
                    op = "loopz"
                case 0b10:
                    op = "loop"
                case 0b11:
                    op = "jcxz"

            printb(f"{op} 0x{get_byte():02X}")

        case [0, 0, a1, a2, a3, 1, 0, w]:
            arithmetic = (a1 << 2) | (a2 << 1) | a3
            # arithmetic = bits_to_byte(a1, a2, a3)

            imm = decode_imm_data(w)
            reg: str = "AX" if w == 1 else "AL"

            op = decode_arithmetic_op(arithmetic)

            printb(f"{op} {reg}, {imm}")

            match op:
                case "add":
                    exec(f"regs.{reg} += {imm}")
                case "adc":
                    exec(f"regs.{reg} += {imm}")
                case "sub":
                    exec(f"regs.{reg} -= {imm}")
                case "cmp":
                    exec(f"cmp = regs.{reg} - {imm}")
                    exec("")
            exec(f"regs.zf = regs.{reg} == 0")
            printb(f" | ZF {"set" if regs.zf else "unset"}")

        case [1, 0, 0, 0, 0, 0, s, w]:
            byte = get_byte()
            arithmetic = (byte & 0b00011100) >> 2

            op = decode_arithmetic_op(arithmetic)

            mod = byte >> 6
            rm = byte & 0b00000111

            reg02 = decode_reg02(rm, mod, w)

            reg01 = decode_add_data(w, s)

            printb(f"{op} {reg02}, {reg01}")

            match op:
                case "add":
                    exec(f"regs.{reg02} += {reg01}")

                    exec(f"regs.cf = {reg01} < regs.{reg02}")
                    printb(f" | CF {"set" if regs.cf else "unset"}")
                case "adc":
                    exec(f"regs.{reg02} += {reg01}")
                case "sub":
                    exec(f"regs.{reg02} -= {reg01}")
                case "cmp":
                    exec(f"regs.cf = regs.{reg02} < {reg01}")
                    printb(f" | CF {"set" if regs.cf else "unset"}")

                    exec(f"cmp = regs.{reg02} - {reg01}")

            exec(f"regs.zf = regs.{reg02} == 0")
            printb(f" | ZF {"set" if regs.zf else "unset"}")

        case [0, 0, a1, a2, a3, 0, d, w]:
            arithmetic = (a1 << 2) | (a2 << 1) | a3

            op = decode_arithmetic_op(arithmetic)

            byte = get_byte()
            mod = byte >> 6
            reg = (byte & 0b00111000) >> 3
            rm = byte & 0b00000111

            reg01 = decode_reg(reg, w)
            reg02 = decode_reg02(rm, mod, w)

            if d == 0:
                printb(f"{op} {reg02}, {reg01}")
                match op:
                    case "add":
                        exec(f"regs.{reg02} += regs.{reg01}")

                        exec(f"regs.cf = regs.{reg01} < regs.{reg02}")
                        printb(f" | CF {"set" if regs.cf else "unset"}")
                    case "adc":
                        exec(f"regs.{reg02} += regs.{reg01}")
                    case "sub":
                        exec(f"regs.cf = regs.{reg01} < regs.{reg02}")
                        printb(f" | CF {"set" if regs.cf else "unset"}")

                        exec(f"regs.{reg02} -= regs.{reg01}")
                    case "cmp":
                        exec(f"regs.cf = regs.{reg02} < regs.{reg01}")
                        printb(f" | CF {"set" if regs.cf else "unset"}")

                        exec(f"cmp = regs.{reg02} - regs.{reg01}")
                exec(f"regs.zf = regs.{reg02} == 0")
                printb(f" | ZF {"set" if regs.zf else "unset"}")
            if d == 1:
                printb(f"{op} {reg01}, {reg02}")
                print(" EXECUTING")
                match op:
                    case "add":
                        exec(f"regs.{reg01} += regs.{reg02}")
                    case "adc":
                        exec(f"regs.{reg01} += regs.{reg02}")
                    case "sub":
                        exec(f"regs.{reg01} -= regs.{reg02}")
                    case "cmp":
                        exec(f"regs.cf = regs.{reg01} < regs.{reg02}")
                        printb(f" | CF {"set" if regs.cf else "unset"}")

                        exec(f"cmp = regs.{reg01} - regs.{reg02}")

                exec(f"regs.zf = regs.{reg01} == 0")
                printb(f" | ZF {"set" if regs.zf else "unset"}")

        case [1, 0, 0, 0, 1, 0, d, w]:
            printb("mov ")

            byte = get_byte()
            mod = byte >> 6
            reg = (byte & 0b00111000) >> 3
            rm = byte & 0b00000111

            reg01 = decode_reg(reg, w)
            reg02 = decode_reg02(rm, mod, w)

            if d == 0:
                printb(f"{reg02}, {reg01}")
                try:
                    exec(f"regs.{reg02} = regs.{reg01}")
                except:
                    print(" [Evaluation failed]")
                    continue
            if d == 1:
                printb(f"{reg01}, {reg02}")
                try:
                    exec(f"regs.{reg01} = regs.{reg02}")
                except:
                    print(" [Evaluation failed]")
                    continue
        case [1, 0, 1, 1, w, r1, r2, r3]:
            printb("mov ")
            reg = r3 + (r2 << 1) + (r1 << 2)

            reg01 = decode_reg(reg, w)
            printb(f"{reg01}, ")

            data = decode_imm_data(w)
            printb(f"{data}")

            try:
                exec(f"regs.{reg01} = {data}")  # hacky hacky
            except:
                print(" [Evaluation failed]")
                continue

        case other:
            print(f"\nCouldn't parse byte {other}")
            finished()

    printb("\n")


# NOTE: end of program is in finished()
