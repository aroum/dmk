"""Run the production scanner against a sequenced ADC and GPIO model."""
from pathlib import Path
import subprocess
import tempfile
import unittest
from .test_hybrid_button import ROOT, function


class ScanPipelineTest(unittest.TestCase):
    def test_ansi_jis_rt_debug_calibration_and_timeout(self):
        source = (ROOT/'keyboards/hhkb/modules/ec/classic_matrix.c').read_text()
        pending_type = source[source.index('typedef struct {'):source.index('// Normalize once')]
        processing = function(source, 'void scan_prepare_pending(') + '\n' + function(source, 'bool scan_process_pending(')
        reporting = function(source, 'void scan_report_pending(')
        scanner = function(source, 'bool hhkb_matrix_scan_raw(')
        harness = r'''
#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include "rapid_trigger.h"
#define MATRIX_COLS 15
#ifdef HHKB_JIS
#define MATRIX_ROWS 5
#define HHKB_ROW_BITS {1,2,4,1024,2048}
#define HHKB_ROW_MASK 0xc07
#define KEYS 69
#else
#define MATRIX_ROWS 4
#define HHKB_ROW_BITS {1,2,4,1024}
#define HHKB_ROW_MASK 0x407
#define KEYS 60
#endif
#include "scan_cache.h"
#define ADC_CFGR1_AUTOFF 1
#define DEBUG_FLAG_TOPRE 1
#define __builtin_expect(x,y) (x)
typedef uint16_t matrix_row_t;
static struct {uint32_t BSRR,BRR;} b,c,d;
#define GPIOB (&b)
#define GPIOC (&c)
#define GPIOD (&d)
static struct {uint32_t CFGR1;} adc;
#define ADC1 (&adc)
static struct {uint16_t CNT;} timer;
#define TIM2 (&timer)
static rt_key_t s_rt_keys[MATRIX_COLS][MATRIX_ROWS];
static rt_key_t reference[MATRIX_COLS][MATRIX_ROWS];
static hhkb_scan_cache_t s_scan_cache;
static uint16_t s_cal_samples[MATRIX_COLS][MATRIX_ROWS];
static uint16_t raw[KEYS];
static uint8_t columns[KEYS],rows[KEYS];
static unsigned primask,starts,finishes,reports,captures,peaks,restarts,decrements;
static unsigned next_index,reported_index,debug_index,fault=1000;
static bool converting,debug,calibrating,capture;
static rt_cfg_t cfg;
const uint16_t *ec_curve_lut;
uint32_t ec_curve_revision;
static int keyboard_get_flat_key_index(unsigned row,unsigned col){
#ifdef HHKB_JIS
 static const unsigned populated[15]={29,27,29,31,29,29,31,29,31,31,31,31,31,31,31};
 if(!(populated[col]&(1u<<row)))return -1;
#endif
 return col*MATRIX_ROWS+row;
}
static bool hhkb_hybrid_on_battery(void){return false;}
static void scan_adc_prepare(void){}
static void scan_adc_idle(void){assert(!primask && !converting);}
static void hhkb_record_matrix_scan(void){}
static bool hhkb_cal_is_capturing_baseline(void){return capture;}
static bool hhkb_is_calibrating(void){return calibrating;}
static rt_cfg_t hhkb_get_scan_cfg(void){return cfg;}
static rt_key_t *hhkb_get_rt_key(unsigned col,unsigned row){return &s_rt_keys[col][row];}
static bool hhkb_get_debug_flag(unsigned f){assert(f==1);return debug;}
static unsigned __get_PRIMASK(void){return primask;}
static void __disable_irq(void){primask=1;}
static void __set_PRIMASK(unsigned p){primask=p;}
static void scan_wait_since(unsigned start,unsigned us){(void)start;assert((us==2 || us==4) && !primask && !converting);assert(GPIOC->BSRR==(1u<<8));++timer.CNT;}
static void scan_delay_us(unsigned us){assert(primask && (us==2 || us==3 || us==25));++timer.CNT;}
static bool adc_start_sample(void){
 assert(primask && !converting);assert(starts<KEYS);
 static const unsigned bits[MATRIX_ROWS]=HHKB_ROW_BITS;
 assert(GPIOB->BRR==bits[rows[starts]]);
 next_index=starts++;converting=next_index!=fault;
 return converting;
}
static uint16_t adc_finish_sample(void){
 assert(converting);assert(primask==!(adc.CFGR1 & ADC_CFGR1_AUTOFF));
 if(next_index){unsigned col=columns[next_index-1],row=rows[next_index-1];assert(s_rt_keys[col][row].down==reference[col][row].down);}
 converting=false;++finishes;return raw[next_index];
}
static void hhkb_cal_accumulate_baseline(unsigned col,unsigned row,unsigned val){
 assert(!primask && !converting && GPIOC->BRR==(1u<<7));assert(finishes==KEYS-(fault<KEYS));while(reported_index==fault)++reported_index;assert(col==columns[reported_index] && row==rows[reported_index]);
 assert(val==raw[reported_index]);++captures;++reported_index;
}
static void hhkb_cal_record_peak(unsigned col,unsigned row,unsigned val){
 assert(!primask && !converting && GPIOC->BRR==(1u<<7));assert(finishes==KEYS-(fault<KEYS));while(reported_index==fault)++reported_index;assert(col==columns[reported_index] && row==rows[reported_index]);
 assert(val==raw[reported_index]);++peaks;++reported_index;
}
static void hhkb_debug_feed_sample(unsigned col,unsigned row,unsigned val,unsigned travel,bool down){
 assert(!primask && !converting);
 while(debug_index==fault)++debug_index;
 assert(col==columns[debug_index] && row==rows[debug_index]);
 assert(val==raw[debug_index]);
 assert(travel==rt_normalize_curve(&reference[col][row],val,ec_curve_lut));
 assert(down==reference[col][row].down);++debug_index;++reports;
}
static void hhkb_calibration_start(void){assert(!primask && !converting);++restarts;}
static void hhkb_cal_decrement_samples(void){assert(!primask && !converting);++decrements;}
'''
        harness += pending_type + processing + '\n' + reporting + '\n' + scanner
        harness += r'''
int main(void){
 (void)hhkb_hybrid_on_battery;(void)d;
 unsigned n=0;
 for(unsigned col=0;col<MATRIX_COLS;++col)for(unsigned row=0;row<MATRIX_ROWS;++row){
#ifdef HHKB_JIS
  static const unsigned populated[15]={29,27,29,31,29,29,31,29,31,31,31,31,31,31,31};
  if(!(populated[col]&(1u<<row)))continue;
#endif
  columns[n]=col;rows[n++]=row;
 }
 assert(n==KEYS);
 matrix_row_t matrix[MATRIX_ROWS]={0},expected[MATRIX_ROWS]={0};
 uint16_t lut[126];for(unsigned i=0;i<126;++i)lut[i]=i*8;
 for(unsigned pass=0;pass<36;++pass){
  cfg=(rt_cfg_t){.rt_enabled=(pass/3)%2,.act_pm=750,.release_pm=100,.reset_pm=50,.press_pm=25};
  if(pass%9==0){
   for(unsigned col=0;col<MATRIX_COLS;++col)for(unsigned row=0;row<MATRIX_ROWS;++row){
    rt_key_set_range(&s_rt_keys[col][row],300,2300);rt_key_reset(&s_rt_keys[col][row]);
    rt_key_set_range(&reference[col][row],300,2300);rt_key_reset(&reference[col][row]);
   }
  }
  if(pass%12==0){ec_curve_lut=ec_curve_lut?NULL:lut;++ec_curve_revision;
   for(unsigned col=0;col<MATRIX_COLS;++col)for(unsigned row=0;row<MATRIX_ROWS;++row){rt_key_reset(&s_rt_keys[col][row]);rt_key_reset(&reference[col][row]);}}
  adc.CFGR1=(pass%2)?ADC_CFGR1_AUTOFF:0;
  debug=pass%4!=0;capture=pass%6==0;calibrating=capture || pass%6==1;
  fault=pass%12==0?(pass?0:KEYS-1):(pass%7==0?pass*17%KEYS:1000);
  bool changed=false;
  for(unsigned i=0;i<KEYS;++i){
   unsigned col=columns[i],row=rows[i];
   raw[i]=(i*83+pass*307)%4096;
   bool down=i==fault?reference[col][row].down:rt_update_curve(&reference[col][row],&cfg,raw[i],ec_curve_lut);
   matrix_row_t previous=expected[row];
   if(down)expected[row]|=1u<<col;else expected[row]&=~(1u<<col);
   changed|=previous!=expected[row];
  }
  starts=finishes=reports=captures=peaks=0;reported_index=debug_index=0;
  unsigned before_restart=restarts,before_decrement=decrements;
  assert(hhkb_matrix_scan_raw(matrix)==changed);
  assert(starts==KEYS && finishes==KEYS-(fault<KEYS));
  assert(reports==(debug?KEYS-(fault<KEYS):0));
  assert(captures==(capture?KEYS-(fault<KEYS):0));assert(peaks==(calibrating && !capture?KEYS-(fault<KEYS):0));
  assert(restarts-before_restart==(capture && fault<KEYS));
  assert(decrements-before_decrement==(capture && fault>=KEYS));
  assert(!memcmp(matrix,expected,sizeof(matrix)));
  assert(!primask && !converting);
 }
 printf("%u-key pipeline passed order, same-frame flush, RT, calibration, debug and timeout checks\n",KEYS);
}
'''
        with tempfile.TemporaryDirectory() as tmp:
            src=Path(tmp)/'pipeline.c';src.write_text(harness)
            for jis in (False,True):
                out=Path(tmp)/('jis' if jis else 'ansi')
                args=['clang','-std=c11','-Wall','-Wextra','-Werror','-O2']
                if jis:args+=['-DHHKB_JIS']
                args+=['-I',str(ROOT/'keyboards/hhkb/modules/ec'),'-I',str(ROOT/'dmk_core/include'),
                       str(ROOT/'keyboards/hhkb/modules/ec/rapid_trigger.c'),str(src),'-o',str(out)]
                subprocess.run(args,check=True)
                subprocess.run([str(out)],check=True)
