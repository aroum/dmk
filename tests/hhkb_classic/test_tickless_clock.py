"""Run the real port clock-update routine against host SysTick registers."""
import pathlib
import subprocess
import tempfile
import unittest

ROOT = pathlib.Path(__file__).resolve().parents[2]


class TicklessClockTest(unittest.TestCase):
    def test_battery_usb_round_trip(self):
        source = (ROOT / 'platforms/stm32l072/freertos_port/port.c').read_text()
        start = source.index('void vPortUpdateSysTickClock(')
        body = source.index('{', start)
        depth = 1
        end = body + 1
        while depth:
            depth += (source[end] == '{') - (source[end] == '}')
            end += 1
        harness = r'''
#include <stdint.h>
#include <assert.h>
#define configTICK_RATE_HZ 1000
#define portMAX_24_BIT_NUMBER 0xffffffUL
#define portMISSED_COUNTS_FACTOR 94UL
#define portNVIC_SYSTICK_CLK_BIT_CONFIG 4UL
#define portNVIC_SYSTICK_INT_BIT 2UL
#define portNVIC_SYSTICK_ENABLE_BIT 1UL
static uint32_t ulTimerCountsForOneTick, xMaximumPossibleSuppressedTicks;
static uint32_t ulStoppedTimerCompensation;
static volatile uint32_t portNVIC_SYSTICK_CTRL_REG;
static volatile uint32_t portNVIC_SYSTICK_LOAD_REG;
static volatile uint32_t portNVIC_SYSTICK_CURRENT_VALUE_REG;
'''
        harness += source[start:end]
        harness += r'''
int main(void) {
    const uint32_t clocks[] = {32000000, 4000000, 32000000};
    const uint32_t reloads[] = {31999, 3999, 31999};
    const uint32_t limits[] = {524, 4194, 524};
    for (unsigned i = 0; i < 3; ++i) {
        portNVIC_SYSTICK_CURRENT_VALUE_REG = 123;
        vPortUpdateSysTickClock(clocks[i]);
        assert(portNVIC_SYSTICK_LOAD_REG == reloads[i]);
        assert(ulTimerCountsForOneTick == reloads[i] + 1);
        assert(xMaximumPossibleSuppressedTicks == limits[i]);
        assert(ulStoppedTimerCompensation == 94);
        assert(portNVIC_SYSTICK_CURRENT_VALUE_REG == 0);
        assert(portNVIC_SYSTICK_CTRL_REG == 7);
    }
}
'''
        with tempfile.TemporaryDirectory() as tmp:
            c_file = pathlib.Path(tmp) / 'clock.c'
            binary = pathlib.Path(tmp) / 'clock'
            c_file.write_text(harness)
            subprocess.run(['cc', '-std=c11', '-Wall', '-Wextra', '-Werror',
                            str(c_file), '-o', str(binary)], check=True)
            subprocess.run([str(binary)], check=True)


if __name__ == '__main__':
    unittest.main()
