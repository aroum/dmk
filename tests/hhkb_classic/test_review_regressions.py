"""Host checks for boot-switch debounce, ADC timeout and debug persistence."""
from pathlib import Path
import subprocess
import tempfile
import unittest
from .test_hybrid_button import ROOT, function


class ReviewRegressionTest(unittest.TestCase):
    def run_c(self, code):
        with tempfile.TemporaryDirectory() as tmp:
            c = Path(tmp) / 'test.c'; c.write_text(code)
            exe = Path(tmp) / 'test'
            subprocess.run(['cc', '-std=c11', '-Wall', '-Wextra', '-Werror',
                            str(c), '-o', str(exe)], check=True)
            subprocess.run([str(exe)], check=True)

    def test_sw6_requires_ten_consecutive_on_samples(self):
        source = (ROOT / 'keyboards/hhkb/modules/dip_switch/dip_switch.c').read_text()
        self.run_c(r'''
#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
static unsigned calls,high_sample;
static struct {uint32_t IDR;} gpio;
static unsigned SystemCoreClock=1000;
static void __NOP(void){}
static void *next_gpio(void){gpio.IDR=(++calls==high_sample)?16:0;return &gpio;}
#define GPIOC ((__typeof__(&gpio))next_gpio())
''' + function(source, 'static bool sw6_stable_on(') + r'''
int main(void){
 for(high_sample=1;high_sample<=10;high_sample++){
  calls=0;assert(!sw6_stable_on());assert(calls==high_sample);
 }
 high_sample=0;calls=0;assert(sw6_stable_on() && calls==10);
}
''')

    def test_adc_timeout_does_not_return_stale_dr(self):
        source = (ROOT / 'keyboards/hhkb/modules/ec/classic_matrix.c').read_text()
        # Model write-one-to-clear ISR semantics in the memory-backed register mock.
        sample = function(source, 'static inline uint16_t adc_read_sample(').replace(
            'ADC1->ISR = ADC_ISR_EOC;', 'ADC1->ISR = 0;')
        self.run_c(r'''
#include <assert.h>
#include <stdint.h>
#define ADC_CR_ADSTP 1
#define ADC_CR_ADSTART 2
#define ADC_CFGR1_AUTOFF 1
#define ADC_ISR_EOC 1
static struct {uint32_t CR,ISR,CFGR1,DR;} adc;
#define ADC1 (&adc)
static void __set_PRIMASK(uint32_t p){(void)p;}
''' + sample + r'''
int main(void){
 adc.DR=1234;adc.CR=ADC_CR_ADSTP;
 assert(adc_read_sample(0)==UINT16_MAX);
 adc.CR=0;adc.ISR=ADC_ISR_EOC; // Stale completion must also be discarded.
 assert(adc_read_sample(0)==UINT16_MAX);
}
''')

    def test_all_debug_flags_mark_settings_dirty_only_on_change(self):
        source = (ROOT / 'keyboards/hhkb/modules/ec/ec_settings.c').read_text()
        self.run_c(r'''
#include <stdint.h>
#include <stdbool.h>
#include <assert.h>
static uint8_t s_debug_flags;
static unsigned dirty;
static void hhkb_settings_mark_dirty(void){++dirty;}
''' + function(source, 'void hhkb_set_debug_flag(') + r'''
int main(void){
 for(unsigned flag=1;flag<=8;flag<<=1){
  unsigned before=dirty;
  hhkb_set_debug_flag(flag,true);assert(dirty==before+1);
  hhkb_set_debug_flag(flag,true);assert(dirty==before+1);
  hhkb_set_debug_flag(flag,false);assert(dirty==before+2);
 }
}
''')

    def test_incomplete_baseline_restarts_instead_of_averaging_missing_sample(self):
        source = (ROOT / 'keyboards/hhkb/modules/ec/classic_matrix.c').read_text()
        block = function(source, 'if (capture) {\n        // A missing sample')
        self.run_c(r'''
#include <stdbool.h>
#include <assert.h>
static unsigned restarts,decrements;
static void hhkb_calibration_start(void){++restarts;}
static void hhkb_cal_decrement_samples(void){++decrements;}
static void frame(bool capture,bool capture_failed){
''' + block + r'''
}
int main(void){
 frame(true,true);assert(restarts==1 && decrements==0);
 frame(true,false);assert(restarts==1 && decrements==1);
 frame(false,true);assert(restarts==1 && decrements==1);
}
''')
