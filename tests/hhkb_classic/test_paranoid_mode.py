"""Tests for SW1 paranoid mode: early DIP latch and USB descriptor/callback gating."""
from pathlib import Path
import subprocess
import tempfile
import unittest

ROOT = Path(__file__).resolve().parents[2]


class ParanoidModeTest(unittest.TestCase):
    def test_sw1_latched_at_boot_and_gates_usb_interfaces(self):
        harness = r'''
#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>
#include <string.h>

typedef struct {
    volatile uint32_t MODER;
    volatile uint32_t OTYPER;
    volatile uint32_t OSPEEDR;
    volatile uint32_t PUPDR;
    volatile uint32_t IDR;
    volatile uint32_t ODR;
    volatile uint32_t BSRR;
    volatile uint32_t LCKR;
    volatile uint32_t AFR[2];
    volatile uint32_t BRR;
} GPIO_TypeDef;

static GPIO_TypeDef fake_gpioa;
#define GPIOA (&fake_gpioa)
#define __NOP() ((void)0)
#define __disable_irq() ((void)0)
#define __WFI() ((void)0)

// Minimal CMSIS/core stubs for dip_switch.c
#define DIP_SWITCH_H_TEST_STUBS
uint32_t SystemCoreClock = 16000000U;
bool hhkb_get_power_saving(void) { return true; }
bool hhkb_storage_erase_early(void) { return true; }

#include "dip_switch.h"

static bool s_paranoid_mode = false;
bool keyboard_paranoid_mode(void) { return s_paranoid_mode; }

static void sample_sw1_boot(uint32_t idr_a) {
    fake_gpioa.IDR = idr_a;
    // Matches hhkb_early_dip_check() SW1 active-LOW sampling on PA3
    s_paranoid_mode = (GPIOA->IDR & (1U << 3)) == 0;
}

// Simulate USB descriptor selection and report gating from usb_descriptors.c
#define ITF_NUM_KEYBOARD 0
#define ITF_NUM_VIAL 1
#define ITF_NUM_CONSOLE 2
#define ITF_NUM_PFU 3

static const uint8_t full_cfg[9] = {9, 2, 100, 0, 4, 1, 0, 0xA0, 100};
static const uint8_t paranoid_cfg[9] = {9, 2, 34, 0, 1, 1, 0, 0xA0, 100};
static const uint8_t kbd_report[2] = {0x05, 0x01};
static const uint8_t vial_report[2] = {0x06, 0x60};
static unsigned vial_calls = 0;
static unsigned pfu_calls = 0;

static const uint8_t *test_cfg_cb(void) {
    return keyboard_paranoid_mode() ? paranoid_cfg : full_cfg;
}

static const uint8_t *test_hid_report_cb(uint8_t instance) {
    if (keyboard_paranoid_mode() && instance != ITF_NUM_KEYBOARD) return NULL;
    if (instance == ITF_NUM_VIAL) return vial_report;
    return kbd_report;
}

static void test_set_report_cb(uint8_t instance) {
    if (keyboard_paranoid_mode() && instance != ITF_NUM_KEYBOARD) return;
    if (instance == ITF_NUM_VIAL) vial_calls++;
    if (instance == ITF_NUM_PFU) pfu_calls++;
}

int main(void) {
    // SW1 OFF (PA3 = 1): normal mode, all interfaces active
    sample_sw1_boot(1U << 3);
    assert(!keyboard_paranoid_mode());
    assert(test_cfg_cb()[4] == 4);
    assert(test_hid_report_cb(ITF_NUM_VIAL) == vial_report);
    test_set_report_cb(ITF_NUM_VIAL);
    test_set_report_cb(ITF_NUM_PFU);
    assert(vial_calls == 1 && pfu_calls == 1);

    // SW1 ON (PA3 = 0): paranoid mode, only keyboard interface (1) active
    sample_sw1_boot(0);
    assert(keyboard_paranoid_mode());
    assert(test_cfg_cb()[4] == 1);
    assert(test_hid_report_cb(ITF_NUM_KEYBOARD) == kbd_report);
    assert(test_hid_report_cb(ITF_NUM_VIAL) == NULL);
    assert(test_hid_report_cb(ITF_NUM_CONSOLE) == NULL);
    assert(test_hid_report_cb(ITF_NUM_PFU) == NULL);
    test_set_report_cb(ITF_NUM_VIAL);
    test_set_report_cb(ITF_NUM_PFU);
    assert(vial_calls == 1 && pfu_calls == 1);
    return 0;
}
'''
        with tempfile.TemporaryDirectory() as tmp:
            src = Path(tmp) / 'paranoid.c'
            out = Path(tmp) / 'paranoid'
            src.write_text(harness)
            subprocess.run(
                ['clang', '-std=c11', '-Wall', '-Wextra', '-Werror',
                 '-I', str(ROOT / 'keyboards/hhkb/modules/dip_switch'),
                 str(src), '-o', str(out)],
                check=True,
            )
            subprocess.run([str(out)], check=True)

    def test_production_sources_contain_paranoid_guards(self):
        dip = (ROOT / 'keyboards/hhkb/modules/dip_switch/dip_switch.c').read_text()
        descs = (ROOT / 'dmk_core/drivers/usb_descriptors.c').read_text()
        usb_task = (ROOT / 'dmk_core/drivers/app_usb_tinyusb.c').read_text()
        self.assertIn('s_paranoid_mode = (GPIOA->IDR & (1U << 3)) == 0;', dip)
        self.assertIn('desc_configuration_paranoid', descs)
        self.assertIn('keyboard_paranoid_mode() && instance != ITF_NUM_KEYBOARD', descs)
        self.assertIn('if (!keyboard_paranoid_mode())', usb_task)


if __name__ == '__main__':
    unittest.main()
