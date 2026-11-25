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
static struct {uint32_t IDR;} gpio,a;
#define GPIOA (&a)
static unsigned SystemCoreClock=1000;
static void __NOP(void){}
static void *next_gpio(void){gpio.IDR=(++calls==high_sample)?16:0;return &gpio;}
#define GPIOC ((__typeof__(&gpio))next_gpio())
''' + function(source, 'static bool recovery_switches_stable_on(') + function(source, 'static bool sw6_stable_on(') + r'''
int main(void){
 for(high_sample=1;high_sample<=10;high_sample++){
  calls=0;assert(!sw6_stable_on());assert(calls==high_sample);
 }
 high_sample=0;calls=0;assert(sw6_stable_on() && calls==10);
}
''')

    def test_sw5_reset_requires_both_switches_stably_on(self):
        source = (ROOT / 'keyboards/hhkb/modules/dip_switch/dip_switch.c').read_text()
        self.run_c(r'''
#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
static unsigned calls,high_sample;
static struct {uint32_t IDR;} a,c;
static unsigned SystemCoreClock=1000;
static void __NOP(void){}
static void *next_a(void){a.IDR=(++calls==high_sample)?128:0;return &a;}
#define GPIOA ((__typeof__(&a))next_a())
#define GPIOC (&c)
''' + function(source, 'static bool recovery_switches_stable_on(') + function(source, 'static bool sw5_stable_on(') + r'''
int main(void){
 for(high_sample=1;high_sample<=10;high_sample++){
  calls=0;c.IDR=0;assert(!sw5_stable_on() && calls==high_sample);
 }
 high_sample=0;calls=0;assert(sw5_stable_on() && calls==10);
 calls=0;c.IDR=16;assert(!sw5_stable_on() && calls==0);
}
''')

    def test_sw56_clears_before_recovery_and_stops_on_write_failure(self):
        source = (ROOT / 'keyboards/hhkb/modules/dip_switch/dip_switch.c').read_text()
        code = r'''
#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <setjmp.h>
#define HHKB_SW6_REBOOT_TO_BANK1 1
#define RCC_IOPENR_GPIOAEN 1
#define RCC_IOPENR_GPIOCEN 2
#define HHKB_EEPROM_BANK2_STATUS_ADDR 0x08080348
#define HHKB_EEPROM_BANK2_CRC_ADDR 0x08080370
static struct {uint32_t IDR,MODER,PUPDR;} a,c;
static struct {uint32_t IOPENR;} rcc;
#define GPIOA (&a)
#define GPIOC (&c)
#define RCC (&rcc)
static unsigned SystemCoreClock=1000,clear_calls,metadata_calls,pages;
static uint32_t hhkb_recovery_flash_error;
static bool clear_ok;
static jmp_buf done;
static void __NOP(void){}
static void __disable_irq(void){}
static void __WFI(void){longjmp(done,2);}
static bool hhkb_storage_erase_early(void){assert(metadata_calls==0 && pages==0);++clear_calls;return clear_ok;}
static void write_eeprom_byte(uint32_t address,uint8_t value){
#ifdef HHKB_HYBRID
 assert(address==0x08080360 && value==0);
#else
 assert((address==0x08080348 && value==1)||((address==0x08080370||address==0x08080371)&&value==0));
#endif
 ++metadata_calls;
}
static void erase_bank2_vector_page_ram(void){++pages;longjmp(done,1);}
'''
        code += function(source, 'static bool recovery_switches_stable_on(')
        code += function(source, 'static bool sw6_stable_on(')
        code += function(source, 'static bool sw5_stable_on(')
        code += function(source, 'void hhkb_enter_bank1(')
        code += function(source, 'void hhkb_early_dip_check(')
        code += r'''
int main(void){
 for(unsigned sw5=0;sw5<2;++sw5)for(unsigned sw6=0;sw6<2;++sw6)for(unsigned ok=0;ok<2;++ok){
  a.IDR=sw5?0:128;c.IDR=sw6?0:16;clear_ok=ok;
  clear_calls=metadata_calls=pages=0;hhkb_recovery_flash_error=0;
  int path=setjmp(done);if(!path)hhkb_early_dip_check();
  assert(clear_calls==(sw5&&sw6));
  if(!sw6){assert(!metadata_calls && !pages);continue;}
  if(sw5&&!ok){assert(path==2 && !metadata_calls && !pages && hhkb_recovery_flash_error==0x40000000);continue;}
#ifdef HHKB_HYBRID
  assert(metadata_calls==1);
#else
  assert(metadata_calls==3);
#endif
  assert(path==1 && pages==1);
 }
}
'''
        self.run_c(code)
        self.run_c('#define HHKB_HYBRID 1\n' + code)

    def test_adc_timeout_does_not_return_stale_dr(self):
        source = (ROOT / 'keyboards/hhkb/modules/ec/classic_matrix.c').read_text()
        # Model write-one-to-clear ISR semantics in the memory-backed register mock.
        sample = function(source, 'static inline bool adc_start_sample(').replace(
            'ADC1->ISR = ADC_ISR_EOC;', 'ADC1->ISR = 0;')
        sample += function(source, 'static inline uint16_t adc_finish_sample(')
        self.run_c(r'''
#include <assert.h>
#include <stdint.h>
#include <stdbool.h>
#define ADC_CR_ADSTP 1
#define ADC_CR_ADSTART 2
#define ADC_CFGR1_AUTOFF 1
#define ADC_ISR_EOC 1
static struct {uint32_t CR,ISR,CFGR1,DR;} adc;
#define ADC1 (&adc)
''' + sample + r'''
int main(void){
 adc.DR=1234;adc.CR=ADC_CR_ADSTP;
 assert(!adc_start_sample());
 adc.CR=0;adc.ISR=ADC_ISR_EOC; // Stale completion must also be discarded.
 assert(adc_start_sample());assert(adc_finish_sample()==UINT16_MAX);
 adc.ISR=ADC_ISR_EOC;assert(adc_finish_sample()==1234);
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
 for(unsigned flag=1;flag<=16;flag<<=1){
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
