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
#define HHKB_SOF_LOCK 1
#define taskENTER_CRITICAL() ((void)0)
#define taskEXIT_CRITICAL() ((void)0)
static volatile bool s_enabled=true;
static volatile int16_t s_last_err;
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
''' + function(source,'void RCC_CRS_IRQHandler(') + function(source,'static bool fresh(') + function(source,'void sof_lock_reset(') + function(source,'bool sof_lock_enabled(') + function(source,'bool sof_lock_set_enabled(') + r'''
int main(void){
 assert(sof_lock_enabled());
 assert(sof_lock_set_enabled(false));assert(!sof_lock_enabled());
 s_sof_good=8;s_sof_cnt=timer.CNT;assert(!fresh(timer.CNT));
 crs.ISR=CRS_ISR_SYNCOKF;timer.CNT+=4000;RCC_CRS_IRQHandler();assert(s_sof_cnt!=timer.CNT);
 assert(sof_lock_set_enabled(true));assert(sof_lock_enabled() && !s_sof_good);
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

    def test_session_control_and_via(self):
        import json
        sof=(ROOT/'keyboards/hhkb/modules/ec/sof_lock.c').read_text()
        via=(ROOT/'keyboards/hhkb/modules/via_settings/hhkb_via_controls.c').read_text()
        code=r'''
#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#define taskENTER_CRITICAL() ((void)0)
#define taskEXIT_CRITICAL() ((void)0)
#if HHKB_SOF_LOCK
static volatile bool s_enabled=true;
static unsigned resets;
static void sof_lock_reset(void){++resets;}
#endif
''' + function(sof,'bool sof_lock_enabled(') + function(sof,'bool sof_lock_set_enabled(') + r'''
static bool command(uint8_t op,uint8_t value,uint8_t *response){
 uint8_t request[32]={op,0,61,value};uint8_t id=request[2];
''' + function(via,'if (id == 61)') + r'''
 return false;
}
int main(void){
 uint8_t response[32]={0};
 assert(command(8,0,response));
#if HHKB_SOF_LOCK
 assert(response[3]==1);assert(command(7,0,response));assert(!sof_lock_enabled() && resets==1);
 assert(command(7,0,response));assert(resets==1);
 assert(command(8,0,response));assert(response[3]==0);
 assert(command(7,1,response));assert(sof_lock_enabled() && resets==2);
#else
 assert(response[3]==0);assert(!command(7,1,response));assert(command(7,0,response));
#endif
 bool before=sof_lock_enabled();assert(!command(7,2,response));assert(!command(6,1,response));
 assert(sof_lock_enabled()==before);
}
'''
        with tempfile.TemporaryDirectory() as tmp:
            src=Path(tmp)/'session.c';src.write_text(code)
            for enabled in (0,1):
                out=Path(tmp)/f'session-{enabled}'
                subprocess.run(['clang','-std=c11','-Wall','-Wextra','-Werror',f'-DHHKB_SOF_LOCK={enabled}',str(src),'-o',str(out)],check=True)
                subprocess.run([str(out)],check=True)
        menus=json.loads((ROOT/'keyboards/hhkb/via.json').read_text())['menus'][0]['content']
        tab=next(t for t in menus if t['label']=='Scan Rate')
        control=next(c for c in tab['content'] if c.get('content')==['id_hhkb_sof_lock',0,61])
        self.assertEqual(control['type'],'toggle')

    def test_note_work_allows_multi_frame_lead_and_caps_at_7800(self):
        source = (ROOT / 'keyboards/hhkb/modules/ec/sof_lock.c').read_text()
        code = r'''
#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#define HHKB_SOF_LOCK 1
#define HHKB_SOF_MARGIN_US 150U
static volatile bool s_enabled = true;
static struct { uint16_t CNT; } timer;
#define TIM2 (&timer)
static uint16_t s_work_peak, s_lead;
''' + function(source, 'void sof_lock_note_work(') + r'''
int main(void) {
    // 100 ticks (25 us) + 600 ticks margin -> 700 ticks.
    timer.CNT = 100;
    sof_lock_note_work(0);
    assert(s_lead == 700);

    // 3720 ticks (930 us scan) + 600 ticks margin -> 4320 ticks (> 1 USB frame).
    timer.CNT = 3720;
    sof_lock_note_work(0);
    assert(s_lead == 4320);

    // Huge spike is capped at 7800 ticks (just under 2 USB frames).
    timer.CNT = 9000;
    sof_lock_note_work(0);
    assert(s_lead == 7800);
    return 0;
}
'''
        with tempfile.TemporaryDirectory() as tmp:
            src = Path(tmp) / 'lead.c'
            exe = Path(tmp) / 'lead'
            src.write_text(code)
            subprocess.run(['clang', '-std=c11', '-Wall', '-Wextra', '-Werror',
                            '-fsanitize=address,undefined', str(src), '-o', str(exe)], check=True)
            subprocess.run([str(exe)], check=True)
