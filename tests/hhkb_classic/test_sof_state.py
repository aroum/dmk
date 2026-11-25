"""Exercise the SOF ISR estimator and reset path, beyond pure phase math."""
import subprocess
import tempfile
import unittest
from pathlib import Path
from .test_hybrid_button import ROOT, function

class SofStateTest(unittest.TestCase):
    def test_period_range_and_disconnect_reset(self):
        source=(ROOT/'keyboards/hhkb/modules/ec/sof_lock.c').read_text()
        code=r'''
#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#define HHKB_SCAN_DIAGNOSTICS 1
#define CRS_ISR_SYNCOKF 1
#define CRS_ISR_SYNCWARNF 2
#define CRS_ICR_SYNCOKC 1
#define CRS_ICR_SYNCWARNC 2
static struct {uint32_t ISR,ICR;} crs;
static struct {uint16_t CNT;} timer;
#define CRS (&crs)
#define TIM2 (&timer)
static volatile bool s_report_pending;
static volatile uint16_t s_sof_cnt;
static volatile uint32_t s_sof_p_q4=64000;
static volatile uint8_t s_sof_good,s_in_n;
''' + function(source,'void RCC_CRS_IRQHandler(') + function(source,'static bool fresh(') + function(source,'void sof_lock_reset(') + r'''
int main(void){
 crs.ISR=CRS_ISR_SYNCOKF;
 // The entire accepted period range must fit Q4, including >4095 ticks.
 for(unsigned period=3820;period<=4180;period+=180){
  sof_lock_reset();s_sof_cnt=0;timer.CNT=0;
  for(unsigned i=0;i<100;i++){timer.CNT+=period;RCC_CRS_IRQHandler();}
  assert(fresh(timer.CNT));
  unsigned measured=(s_sof_p_q4+8)>>4;
  assert(measured>=period-1 && measured<=period+1);
 }
 timer.CNT+=16000;assert(!fresh(timer.CNT));
 s_report_pending=true;s_in_n=4;sof_lock_reset();
 assert(!s_report_pending && !s_sof_good && !s_in_n && !fresh(timer.CNT));
 crs.ISR=0;unsigned before=s_sof_cnt;timer.CNT+=4000;RCC_CRS_IRQHandler();assert(s_sof_cnt==before);
 crs.ISR=CRS_ISR_SYNCWARNF;
 for(unsigned i=0;i<8;i++){timer.CNT+=4000;RCC_CRS_IRQHandler();}
 assert(!fresh(timer.CNT)); // First interval was invalid, seven good samples only.
 timer.CNT+=4000;RCC_CRS_IRQHandler();assert(fresh(timer.CNT));
}
'''
        declaration=next(line for line in source.splitlines() if 'static volatile' in line and 's_sof_p_q4' in line)
        code=code.replace('static volatile uint32_t s_sof_p_q4=64000;',declaration)
        with tempfile.TemporaryDirectory() as tmp:
            src=Path(tmp)/'sof.c';src.write_text(code);out=Path(tmp)/'sof'
            subprocess.run(['clang','-std=c11','-Wall','-Wextra','-Werror','-fsanitize=undefined',str(src),'-o',str(out)],check=True)
            subprocess.run([str(out)],check=True)
