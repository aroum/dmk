#!/usr/bin/env python3
"""Run the HHKB matrix scan of a built STM32L072 ELF in an instruction-level emulator.

Development aid for scan timing work, not a hardware model. It loads the ELF into Unicorn,
models TIM2->CNT from an estimated cycle count, an ADC that converts in 14 ADC clocks and
returns one fixed reading for every key, and counts instructions with a fixed CPI for RAM and
for flash code. Absolute times came out 10-30 % lower than hardware measurements with
HHKB_SCAN_DIAGNOSTICS; differences between two builds held up well.

Adapted from blandjelly/dmk (MIT), issue #8.
Requires: uv run --with unicorn --with pyelftools tools/hhkb/scan_emu.py ...
and arm-none-eabi-nm on PATH.

Examples:
  scan_emu.py time build/hhkb-script/dmk_hhkb_stm32l072
  scan_emu.py gaps  ELF --offset -3     # release-to-strobe intervals, reading 3 counts below base
  scan_emu.py profile ELF --rt on       # instructions per function for one scan
  scan_emu.py cal ELF                   # baseline capture, full press and release of every key
"""
import argparse
import bisect
import collections
import statistics
import struct
import subprocess

from elftools.elf.elffile import ELFFile
from unicorn import UC_ARCH_ARM, UC_HOOK_CODE, UC_HOOK_MEM_WRITE, UC_MODE_MCLASS, UC_MODE_THUMB, Uc
from unicorn.arm_const import UC_ARM_REG_LR, UC_ARM_REG_R0, UC_ARM_REG_SP

RETURN_ADDRESS = 0x08000100
CYCLES_PER_TIM2_TICK = 8      # 32 MHz core, TIM2 at 4 ticks per microsecond
ADC_CONVERSION_CYCLES = 28    # 14 ADC clocks at PCLK/2
TIM2_CNT, RCC_APB1ENR = 0x40000024, 0x40021038
ADC_ISR, ADC_CR, ADC_DR = 0x40012400, 0x40012408, 0x40012440
GPIOB_BSRR, GPIOB_BRR = 0x50000418, 0x50000428
GPIOC_BSRR, GPIOC_BRR = 0x50000818, 0x50000828


class ScanEmulator:
    def __init__(self, elf_path, cpi_ram=1.3, cpi_flash=2.0, periph_extra=2):
        self.cycles = 0.0
        self.adc_start = -1e9
        self.reading = 0
        self.registers = {0x40000000: 1}  # TIM2 CR1.CEN, as set by the matrix task
        self.events = []
        self.record_events = False
        self.counting = None
        self.symbols = {}
        self.functions = []
        for line in subprocess.check_output(["arm-none-eabi-nm", "-S", "-n", elf_path], text=True).splitlines():
            parts = line.split()
            if len(parts) == 4:
                self.symbols[parts[3]] = int(parts[0], 16)
                if parts[2] in "tTW":
                    self.functions.append((int(parts[0], 16), int(parts[1], 16), parts[3]))
            elif len(parts) == 3:
                self.symbols[parts[2]] = int(parts[0], 16)
        self.function_starts = [f[0] for f in self.functions]

        uc = self.uc = Uc(UC_ARCH_ARM, UC_MODE_THUMB | UC_MODE_MCLASS)
        uc.mem_map(0x08000000, 0x00100000)   # flash and data EEPROM
        uc.mem_write(0x08080000, b"\xff" * 0x2000)
        uc.mem_map(0x20000000, 0x00010000)
        with open(elf_path, "rb") as stream:
            for segment in ELFFile(stream).iter_segments():
                if segment["p_type"] == "PT_LOAD" and segment["p_filesz"]:
                    uc.mem_write(segment["p_vaddr"], segment.data())
        uc.mmio_map(0x40000000, 0x00100000, self._read, None, self._write, None)
        uc.mem_map(0x50000000, 0x00002000)   # GPIO ports
        uc.mem_map(0xE0000000, 0x00100000)   # SCB and NVIC

        def on_code(uc, address, size, user):
            self.cycles += cpi_ram if address >= 0x20000000 else cpi_flash
            if self.counting is not None:
                self.counting[self.function_name(address)] += 1

        def on_write(uc, access, address, size, value, user):
            if 0x40000000 <= address < 0x60000000:
                self.cycles += periph_extra
            if not self.record_events:
                return
            if address == GPIOB_BRR and value & 0x0C07:
                self.events.append(("strobe", self.cycles))
            elif address == GPIOB_BSRR and value & 0x0C07 and value != 0x0C07 and not value >> 16:
                self.events.append(("release", self.cycles))
            elif address == GPIOC_BSRR and value & (1 << 8):
                self.events.append(("discharge", self.cycles))

        uc.hook_add(UC_HOOK_CODE, on_code)
        uc.hook_add(UC_HOOK_MEM_WRITE, on_write)

    def function_name(self, address):
        i = bisect.bisect_right(self.function_starts, address) - 1
        if i >= 0 and address < self.functions[i][0] + self.functions[i][1]:
            return self.functions[i][2]
        return "?"

    def _read(self, uc, offset, size, user):
        address = 0x40000000 + offset
        if address == TIM2_CNT:
            return int(self.cycles / CYCLES_PER_TIM2_TICK) & 0xFFFF
        if address == RCC_APB1ENR:
            return self.registers.get(address, 0) | 1
        if address == ADC_ISR:
            done = self.cycles - self.adc_start >= ADC_CONVERSION_CYCLES
            return 0x1 | (0x4 if done else 0)
        if address == ADC_CR:
            return 0x1
        if address == ADC_DR:
            return self.reading
        return self.registers.get(address, 0)

    def _write(self, uc, offset, size, value, user):
        address = 0x40000000 + offset
        self.registers[address] = value
        if address == ADC_CR and value & 0x4:
            self.adc_start = self.cycles

    def call(self, name, *args):
        uc = self.uc
        uc.reg_write(UC_ARM_REG_SP, 0x2000F000)
        uc.reg_write(UC_ARM_REG_LR, RETURN_ADDRESS | 1)
        for i, value in enumerate(args):
            uc.reg_write(UC_ARM_REG_R0 + i, value)
        uc.emu_start(self.symbols[name] | 1, RETURN_ADDRESS, count=50_000_000)

    def read(self, name, fmt, offset=0):
        size = struct.calcsize(fmt)
        return struct.unpack(fmt, bytes(self.uc.mem_read(self.symbols[name] + offset, size)))

    def setup(self, rt_on, offset):
        self.call("hhkb_calibration_load")
        self.uc.mem_write(self.symbols["s_rt_cfg"], bytes([1 if rt_on else 0]))
        base = self.read("s_rt_keys", "<H")[0]
        self.reading = max(0, min(4095, base + offset))
        self.matrix = 0x2000E000
        self.uc.mem_write(self.matrix, b"\0" * 32)

    def scan(self):
        start = self.cycles
        self.call("hhkb_matrix_scan_raw", self.matrix)
        return (self.cycles - start) / 32


def intervals(events, first, second):
    out, last = [], None
    for name, cycles in events:
        if name == first:
            last = cycles
        elif name == second and last is not None:
            out.append((cycles - last) / 32)
            last = None
    return out


def main():
    parser = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument("command", choices=["time", "gaps", "profile", "cal"])
    parser.add_argument("elf")
    parser.add_argument("--rt", choices=["on", "off"], default="on")
    parser.add_argument("--offset", type=int, default=3, help="ADC reading relative to the key baseline")
    parser.add_argument("--cpi-ram", type=float, default=1.3)
    parser.add_argument("--cpi-flash", type=float, default=2.0)
    args = parser.parse_args()

    emu = ScanEmulator(args.elf, args.cpi_ram, args.cpi_flash)
    emu.setup(args.rt == "on", args.offset)

    if args.command == "cal":
        emu.call("hhkb_calibration_start")
        for _ in range(18):
            emu.scan()

        def count(name):
            return sum(bin(b).count("1") for b in emu.uc.mem_read(emu.symbols[name], 8))
        print("after baseline capture: calibrated keys", count("s_calibrated_mask"))
        resting = emu.reading
        emu.reading = min(4095, resting + 2000)
        emu.scan(), emu.scan()
        print("after pressing every key: calibrated keys", count("s_calibrated_mask"))
        emu.reading = resting
        emu.scan(), emu.scan()
        print("after release: keys still down", count("s_cal_down_mask"))
        return

    emu.scan()  # first scan builds the cached configuration
    if args.command == "profile":
        emu.counting = collections.Counter()
    emu.record_events = args.command == "gaps"
    us = emu.scan()
    print(f"scan: {us:.1f} us (RT {args.rt}, reading base{args.offset:+d})")
    if args.command == "gaps":
        for first, second in [("release", "strobe"), ("discharge", "strobe")]:
            values = intervals(emu.events, first, second)
            if values:
                print(f"  {first} -> {second}: n={len(values)} min={min(values):.2f} "
                      f"median={statistics.median(values):.2f} max={max(values):.2f} us")
    if args.command == "profile":
        for name, n in emu.counting.most_common(12):
            print(f"  {n:7d} {name}")


if __name__ == "__main__":
    main()
