"""Reject unusable captures and list physical missing keys for ANSI and JIS."""
import subprocess
import tempfile
import unittest
from pathlib import Path
from .test_hybrid_button import ROOT, function


class CalibrationRangesTest(unittest.TestCase):
    def test_finish_and_missing(self):
        source=(ROOT/'keyboards/hhkb/modules/ec/ec_settings.c').read_text()
        code=r'''
#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include "rapid_trigger.h"
#define MATRIX_COLS 15
#define MATRIX_ROWS ROWS
#define NUM_KEYS (ROWS==5 ? 69 : 60)
#define HHKB_CAL_BASE_MIN 20u
#define DEBUG_FLAG_CAL 8
static bool s_calibration_mode=true,s_custom_calibration;
static unsigned dirty;
#define CAL_LOG_FINISH 4
static void cal_log(unsigned e,unsigned k){(void)e;(void)k;}
static uint32_t s_calibrated_mask[3];
static uint16_t s_cal_baseline[15][ROWS],s_cal_peak[15][ROWS];
static uint32_t s_capture_base[15][ROWS];
static uint16_t s_capture_peak[15][ROWS];
static rt_key_t s_rt_keys[15][ROWS];
static char output[2048];
static void debug_console_printf(const char *format,...){
 va_list args;va_start(args,format);vsnprintf(output+strlen(output),sizeof(output)-strlen(output),format,args);va_end(args);
}
static bool hhkb_get_debug_flag(unsigned flag){(void)flag;return false;}
static unsigned calibrated_key_count(void){return 0;}
enum { CAL_LOG_FINISH = 5 };
static void cal_log_push(uint8_t kind,uint8_t row,uint8_t col,uint8_t count,uint16_t a,uint16_t b){
 (void)kind;(void)row;(void)col;(void)count;(void)a;(void)b;
}
static void hhkb_settings_mark_dirty(void){++dirty;}
void rt_key_set_range(rt_key_t *k,uint16_t b,uint16_t p){k->base=b;(void)p;}
void rt_key_reset(rt_key_t *k){k->down=false;}
static int keyboard_get_flat_key_index(unsigned row,unsigned col){
#if ROWS==5
 static const unsigned populated[15]={29,27,29,31,29,29,31,29,31,31,31,31,31,31,31};
 if(!(populated[col]&(1u<<row)))return -1;
#endif
 return col*ROWS+row;
}
''' + function(source,'void hhkb_calibration_finish(') + function(source,'void hhkb_cal_print_keys(') + r'''
int main(void){
 for(unsigned c=0;c<15;c++)for(unsigned r=0;r<ROWS;r++){
  s_cal_baseline[c][r]=800;s_cal_peak[c][r]=2800;
  s_capture_base[c][r]=99;s_capture_peak[c][r]=2500;
  unsigned i=c*ROWS+r;s_calibrated_mask[i/32]|=1u<<(i%32);
 }
 s_capture_base[0][0]=19; // rejected floor
 s_capture_base[1][0]=2001; // rejected ceiling
 s_capture_peak[2][0]=200; // rejected range
 s_capture_peak[3][0]=4096; // rejected ADC peak
 s_capture_base[4][0]=20; // boundary is valid
 hhkb_calibration_finish();assert(!s_calibration_mode && s_custom_calibration && dirty==1);
 assert(s_cal_baseline[0][0]==800 && s_cal_baseline[1][0]==800);
 assert(s_cal_baseline[2][0]==800 && s_cal_baseline[3][0]==800);
 assert(s_cal_baseline[4][0]==20 && s_cal_baseline[5][0]==99);
 hhkb_cal_print_keys();assert(strstr(output,"unchanged: R0 C0 R0 C1 R0 C2 R0 C3"));
 memset(s_calibrated_mask,0,sizeof(s_calibrated_mask));output[0]=0;s_calibration_mode=true;
 hhkb_cal_print_keys();assert(strstr(output,"missing:"));
 unsigned count=0;for(char *p=output;*p;p++)if(*p=='R' && p[1]>='0' && p[1]<='9')++count;
 assert(count==NUM_KEYS);
}
'''
        with tempfile.TemporaryDirectory() as tmp:
            c=Path(tmp)/'ranges.c';c.write_text(code)
            for rows in (4,5):
                out=Path(tmp)/f'ranges-{rows}'
                subprocess.run(['clang','-std=c11','-Wall','-Wextra','-Werror',f'-DROWS={rows}',
                    '-I',str(ROOT/'keyboards/hhkb/modules/ec'),str(c),'-o',str(out)],check=True)
                subprocess.run([str(out)],check=True)
