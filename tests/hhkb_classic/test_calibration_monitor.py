"""Production monitor callbacks never format from the small matrix stack."""
from pathlib import Path
import subprocess
import tempfile
import unittest
from .test_hybrid_button import ROOT, function

class CalibrationMonitorTest(unittest.TestCase):
    def test_deferred_monitor_and_calibration_schedule(self):
        source=(ROOT/'keyboards/hhkb/modules/ec/ec_settings.c').read_text()
        matrix=(ROOT/'keyboards/hhkb/modules/ec/matrix.c').read_text()
        code=r'''
#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <stdarg.h>
#define MATRIX_COLS 15
#define MATRIX_ROWS ROWS
#define NUM_KEYS (ROWS==5?69:60)
#define RT_MIN_RANGE 150
#define CAL_BASELINE_SAMPLES 16
#define DEBUG_FLAG_CAL 8
#define HHKB_SCAN_TIMER_TICKS_PER_US 4
static unsigned depth,formatted;
static bool in_matrix,enabled=true;
#define taskENTER_CRITICAL() (++depth)
#define taskEXIT_CRITICAL() (--depth)
static uint16_t s_capture_base[15][ROWS],s_capture_peak[15][ROWS],s_cal_press_start_peak[15][ROWS];
static uint32_t s_calibrated_mask[3],s_cal_down_mask[3];
static uint8_t s_cal_baseline_samples;
static bool s_capture_baseline,s_calibration_mode;
static char output[256];
static bool hhkb_get_debug_flag(unsigned f){assert(f==8);return enabled;}
static unsigned calibrated_key_count(void){unsigned n=0;for(unsigned i=0;i<3;i++)n+=__builtin_popcount(s_calibrated_mask[i]);return n;}
static void debug_console_printf(const char *fmt,...){
 assert(!in_matrix && !depth);++formatted;
 va_list ap;va_start(ap,fmt);vsnprintf(output,sizeof(output),fmt,ap);va_end(ap);
}
'''
        code+=source[source.index('enum { CAL_LOG_START'):source.index('void hhkb_calibration_start(void)')]
        for sig in ('void hhkb_calibration_start(', 'void hhkb_cal_accumulate_baseline(',
                    'void hhkb_cal_record_peak(', 'void hhkb_cal_decrement_samples('):
            code+=function(source,sig)+'\n'
        code+=r'''
static uint8_t rate,s_scan_rate_applied,s_scan_phase,s_scan_divider;
static uint16_t s_scan_period_ticks;
static bool s_scan_calibrating;
static uint8_t hhkb_get_effective_scan_rate(void){return rate;}
static bool hhkb_is_calibrating(void){return s_calibration_mode;}
'''+function(matrix,'static void update_scan_period(')+r'''
int main(void){
 rate=4;update_scan_period();assert(s_scan_period_ticks==4000 && s_scan_divider==1);
 in_matrix=true;hhkb_calibration_start();assert(!formatted && !depth);
 in_matrix=false;cal_flush_log();assert(formatted==1 && strstr(output,"Capturing"));
 update_scan_period();assert(s_scan_period_ticks==4000 && s_scan_divider==2);
 in_matrix=true;
 for(unsigned pass=0;pass<16;pass++){
  for(unsigned c=0;c<15;c++)for(unsigned r=0;r<ROWS;r++)hhkb_cal_accumulate_baseline(c,r,400);
  hhkb_cal_decrement_samples();
 }
 assert(!s_capture_baseline && formatted==1);
 in_matrix=false;cal_flush_log();assert(formatted==2 && strstr(output,"Baseline captured"));
 for(unsigned c=0;c<15;c++)for(unsigned r=0;r<ROWS;r++){
#if ROWS==5
  static const unsigned populated[15]={29,27,29,31,29,29,31,29,31,31,31,31,31,31,31};
  if(!(populated[c]&(1u<<r)))continue;
#endif
  unsigned before=formatted;in_matrix=true;
  hhkb_cal_record_peak(c,r,1200);hhkb_cal_record_peak(c,r,1600);hhkb_cal_record_peak(c,r,400);
  assert(formatted==before && !depth);
  in_matrix=false;cal_flush_log();assert(formatted==before+1 && strstr(output,"peak=1600 base=400"));
 }
 assert(calibrated_key_count()==NUM_KEYS);
 unsigned before=formatted;cal_flush_log();assert(formatted==before);
 enabled=false;in_matrix=true;hhkb_cal_record_peak(0,0,2000);in_matrix=false;cal_flush_log();assert(formatted==before);
 rate=3;update_scan_period();assert(s_scan_period_ticks==8000 && s_scan_divider==1);
 rate=0;update_scan_period();assert(s_scan_period_ticks==40000 && s_scan_divider==2);
 rate=4;s_calibration_mode=false;update_scan_period();assert(s_scan_period_ticks==4000 && s_scan_divider==1);
}
'''
        with tempfile.TemporaryDirectory() as tmp:
            p=Path(tmp)/'monitor.c';p.write_text(code)
            for rows in (4,5):
                exe=Path(tmp)/f'monitor-{rows}'
                subprocess.run(['clang','-std=c11','-Wall','-Wextra','-Werror','-fsanitize=address,undefined',f'-DROWS={rows}',str(p),'-o',str(exe)],check=True)
                subprocess.run([str(exe)],check=True)
