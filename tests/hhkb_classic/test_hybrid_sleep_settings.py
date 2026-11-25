"""Round-trip the production EEPROM save/load paths for ANSI and JIS."""
from pathlib import Path
import re
import subprocess
import tempfile
import unittest
from .test_hybrid_button import ROOT, function


class HybridSleepSettingsTest(unittest.TestCase):
    def test_eeprom_round_trip_and_legacy_default(self):
        source = (ROOT / 'keyboards/hhkb/modules/ec/ec_settings.c').read_text()
        header = (ROOT / 'keyboards/hhkb/modules/ec/ec_settings.h').read_text()
        record = header[header.index('typedef struct'):header.index('_Static_assert')]
        constants = header[header.index('#ifdef HHKB_JIS'):header.index('typedef struct')]
        constants = re.sub(r'#define\s+USER_EEPROM_ADDR\s+\S+', '#define USER_EEPROM_ADDR ((uintptr_t)storage)', constants)
        decl = source[source.index('static rt_cfg_t s_rt_cfg'):source.index('static volatile bool s_settings_dirty')]
        code = r'''
#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include "rapid_trigger.h"
#include "ec_curve.h"
#include "settings_storage_guard.h"
#define HHKB_CAL_BASE_MIN 20u
#define WORD_ALIGNED_SIZE(x) (((x)+3U)&~3U)
#define MATRIX_COLS 15
#define DEBUG_FLAG_TOPRE 1
#define pdMS_TO_TICKS(x) (x)
#define FLASH_PECR_PELOCK 1
#define FLASH_SR_BSY 1
#define FLASH_SR_WRPERR 0
#define FLASH_SR_PGAERR 0
#define FLASH_SR_SIZERR 0
#define FLASH_SR_NOTZEROERR 0
#define FLASH_SR_FWWERR 0
#define FLASH_SR_EOP 0
static struct {uint32_t PECR,PEKEYR,SR;} flash;
#define FLASH (&flash)
static uint32_t storage[256];
static uint16_t s_cal_baseline[MATRIX_COLS][MATRIX_ROWS],s_cal_peak[MATRIX_COLS][MATRIX_ROWS];
static rt_key_t s_rt_keys[MATRIX_COLS][MATRIX_ROWS];
static bool s_custom_calibration=true;
static uint8_t s_debug_flags;
static uint8_t s_gesc_overrides,s_gesc_ignore;
uint8_t ec_curve_settings[EC_CURVE_FIELDS];
static unsigned dirty;
static bool s_settings_dirty;
static unsigned snapshot_depth;
#define taskENTER_CRITICAL() (++snapshot_depth)
#define taskEXIT_CRITICAL() (--snapshot_depth)
static void *checked_memcpy(void *dst,const void *src,size_t n){
 if(src==s_cal_baseline || src==s_cal_peak) assert(snapshot_depth==1);
 return memcpy(dst,src,n);
}
#define memcpy checked_memcpy

static bool hhkb_eeprom_clear_words(volatile uint32_t *dst,size_t n){while(n--)dst[n]=0;return true;}
static void hhkb_settings_mark_dirty(void){++dirty;}
bool hhkb_settings_storage_writable(void);
static void vTaskDelay(unsigned t){(void)t;}
static uint16_t get_default_baseline(unsigned c,unsigned r){(void)c;(void)r;return 300;}
static uint16_t get_default_peak(unsigned c,unsigned r){(void)c;(void)r;return 2800;}
static void rt_cfg_defaults(rt_cfg_t *c){memset(c,0,sizeof(*c));c->bottom_dz=5;}
static void rt_key_set_range(rt_key_t *k,uint16_t b,uint16_t p){(void)k;(void)b;(void)p;}
static void rt_key_reset(rt_key_t *k){(void)k;}
bool ec_curve_set(uint8_t id,uint8_t value){(void)id;(void)value;return true;}
void ec_curve_rebuild(void){}
'''
        # Header prototypes for the mocked RT functions are external.
        code = code.replace('static void rt_', 'void rt_')
        code += constants + record + decl
        for signature in ('bool vial_keyboard_settings_reset(', 'bool hhkb_settings_storage_writable(', 'static uint16_t compute_crc16(', 'uint8_t hhkb_bt_sleep_hz(',
                          'bool hhkb_bt_set_sleep_hz(', 'uint8_t hhkb_rt_get(', 'void hhkb_rt_set(', 'void hhkb_calibration_save_to_eeprom(',
                          'void hhkb_calibration_load('):
            code += function(source, signature) + '\n'
        code += r'''
int main(void){
 for(unsigned zone=0;zone<=10;zone++){hhkb_rt_set(65,zone);assert(hhkb_rt_get(65)==zone && !snapshot_depth);}
 hhkb_rt_set(65,11);assert(hhkb_rt_get(65)==10);
 for(unsigned hz=1;hz<=10;hz++){
  assert(hhkb_bt_set_sleep_hz(hz));
  for(unsigned led=0;led<=4;led++)for(unsigned enabled=0;enabled<=1;enabled++){
   s_led_mode[0]=led;s_led_mode[1]=4;
   s_bt_matrix_wake=enabled;s_bt_sleep_minutes=23;s_bt_deep_index=7;
   s_bt_short_action=5;s_bt_long_action=2;s_bt_last_slot=3;
   for(unsigned c=0;c<MATRIX_COLS;c++)for(unsigned r=0;r<MATRIX_ROWS;r++){
    s_cal_baseline[c][r]=c==0 ? 99 : 300+c;s_cal_peak[c][r]=2800+r;
   }
   s_debug_flags=16;s_rt_cfg.bottom_dz=hz;
   s_gesc_overrides=(hz+led+enabled)&15;
   unsigned saved_overrides=s_gesc_overrides;
   unsigned saved_ignore=(hz+led+enabled)&3;s_gesc_ignore=saved_ignore;
   assert(hhkb_settings_storage_writable());
   flash.PECR=0;hhkb_calibration_save_to_eeprom();assert(snapshot_depth==0);
   assert(hhkb_settings_storage_writable());
   s_bt_sleep_hz=1;s_bt_matrix_wake=false;s_bt_sleep_minutes=0;
   s_bt_deep_index=0;s_bt_short_action=0;s_bt_long_action=0;s_bt_last_slot=0;
   s_led_mode[0]=0;s_led_mode[1]=0;
   s_gesc_overrides=0;s_gesc_ignore=0;
   hhkb_calibration_load();
   assert(s_gesc_ignore==saved_ignore);
   assert(s_gesc_overrides==saved_overrides && s_debug_flags==16 && s_rt_cfg.bottom_dz==hz);
   assert(hhkb_bt_sleep_hz()==hz && s_bt_matrix_wake==enabled && s_bt_sleep_minutes==23);
   assert(s_bt_deep_index==7 && s_bt_short_action==5 && s_bt_long_action==2 && s_bt_last_slot==3);
   assert(s_led_mode[0]==led && s_led_mode[1]==4);
   for(unsigned c=0;c<MATRIX_COLS;c++)for(unsigned r=0;r<MATRIX_ROWS;r++){
    assert(s_cal_baseline[c][r]==(c==0 ? 99 : 300+c) && s_cal_peak[c][r]==2800+r);
   }
  }
 }
 for(unsigned flags=0;flags<16;flags++){
  s_gesc_overrides=flags;flash.PECR=0;hhkb_calibration_save_to_eeprom();
  s_gesc_overrides=255;hhkb_calibration_load();assert(s_gesc_overrides==flags);
  assert(s_led_mode[1]==4);
 }
 assert(!hhkb_bt_set_sleep_hz(0) && !hhkb_bt_set_sleep_hz(11) && !hhkb_bt_set_sleep_hz(255));
 hhkb_eeprom_cal_t *cal=(hhkb_eeprom_cal_t *)storage;
 // Explicit zero zone round-trips; unmarked old padding must be ignored.
 s_rt_cfg.bottom_dz=0;flash.PECR=0;hhkb_calibration_save_to_eeprom();
 hhkb_calibration_load();assert(s_rt_cfg.bottom_dz==0);
 cal->debug_flags&=~0x40;cal->rt.bottom_dz=255;
 cal->crc16=compute_crc16((const uint8_t *)&cal->baseline,sizeof(*cal)-offsetof(hhkb_eeprom_cal_t,baseline));
 hhkb_calibration_load();assert(s_rt_cfg.bottom_dz==5);
 // Old record: no rate stored. Recalculate its real CRC before loading.
 cal->led_mode[0]&=15;cal->led_mode[1]&=7;cal->debug_flags&=~0x20;
 cal->crc16=compute_crc16((const uint8_t *)&cal->baseline,sizeof(*cal)-offsetof(hhkb_eeprom_cal_t,baseline));
 s_bt_sleep_hz=1;hhkb_calibration_load();assert(hhkb_bt_sleep_hz()==1 && s_led_mode[0]==4 && s_gesc_overrides==0 && !s_gesc_ignore);
 // Reject ANSI legacy data instead of reading it as a five-row record.
#ifdef HHKB_JIS
 cal->magic=USER_EEPROM_LEGACY_MAGIC;
 hhkb_calibration_load();
 for(unsigned c=0;c<MATRIX_COLS;c++)for(unsigned r=0;r<MATRIX_ROWS;r++)
  assert(s_cal_baseline[c][r]==300 && s_cal_peak[c][r]==2800);
 cal->magic=USER_EEPROM_MAGIC;
#endif
 // Corrupted data must not enable matrix wake or load the new rate.
 cal->led_mode[0]^=0x80;s_bt_sleep_hz=1;s_bt_matrix_wake=false;
 hhkb_calibration_load();assert(hhkb_bt_sleep_hz()==1 && !s_bt_matrix_wake);
 assert(vial_keyboard_settings_reset());
 for(unsigned i=0;i<sizeof(*cal);i++)assert(((uint8_t *)cal)[i]==0);
 assert(hhkb_settings_storage_writable());
}
'''
        with tempfile.TemporaryDirectory() as tmp:
            c = Path(tmp) / 'settings.c'; c.write_text(code)
            for rows in (4, 5):
                binary = Path(tmp) / f'settings-{rows}'
                subprocess.run(['cc', '-std=c11', '-DHHKB_HYBRID=1', f'-DMATRIX_ROWS={rows}',
                                *(['-DHHKB_JIS=1'] if rows == 5 else []),
                                '-I', str(ROOT / 'keyboards/hhkb/modules/ec'),
                                '-I', str(ROOT / 'dmk_core/include'),
                                '-I', str(ROOT / 'keyboards/hhkb/modules/via_settings'), str(c), '-o', str(binary)], check=True)
                subprocess.run([str(binary)], check=True)

    def test_sleep_routing_probe_period_and_via_visibility(self):
        import json
        source = (ROOT / 'keyboards/hhkb/modules/ec/matrix.c').read_text()
        first = function(source, 'if (!critical && !hhkb_bt_matrix_wake())')
        asleep = function(source, 'if (!critical && !wake && !hhkb_bt_matrix_wake())')
        period = next(line.strip() for line in source.splitlines() if 'TickType_t probe_period =' in line)
        code = r'''
#include <stdbool.h>
#include <stdint.h>
#include <assert.h>
typedef uint32_t TickType_t;
#define pdMS_TO_TICKS(x) (x)
static bool enabled;
static uint8_t hz;
static unsigned off;
static bool hhkb_bt_matrix_wake(void){return enabled;}
static uint8_t hhkb_bt_sleep_hz(void){return hz;}
static void hhkb_hybrid_toggle_power(void){++off;}
static bool enter(bool critical){for(unsigned i=0;i<1;i++){
''' + first + r'''
return true;}return false;}
static bool stay(bool critical,bool wake){for(unsigned i=0;i<1;i++){
''' + asleep + r'''
return true;}return false;}
static TickType_t probe(void){
''' + period + r'''
return probe_period;}
int main(void){
 enabled=false;assert(!enter(false) && off==1);
 assert(enter(true) && off==1); // Critical warning is not bypassed.
 assert(!stay(false,false) && off==2); // Turning wake off during sleep.
 assert(stay(true,false) && stay(false,true) && off==2);
 enabled=true;assert(enter(false) && stay(false,false) && off==2);
 for(hz=1;hz<=10;hz++)assert(probe()==1000u/hz);
}
'''
        with tempfile.TemporaryDirectory() as tmp:
            c = Path(tmp) / 'routing.c'; c.write_text(code)
            binary = Path(tmp) / 'routing'
            subprocess.run(['cc', '-std=c11', '-Wall', '-Wextra', '-Werror',
                            str(c), '-o', str(binary)], check=True)
            subprocess.run([str(binary)], check=True)
        definition = json.loads((ROOT / 'keyboards/hhkb/via.json').read_text())
        def walk(node):
            if isinstance(node, dict):
                yield node
                for value in node.values(): yield from walk(value)
            elif isinstance(node, list):
                for value in node: yield from walk(value)
        bluetooth = next(n for n in walk(definition) if n.get('label') == 'Bluetooth')
        self.assertEqual(bluetooth['showIf'], '{id_hhkb_hybrid} == 1')
        controls = {n['label']: n for n in walk(bluetooth) if 'label' in n}
        for label in ('Sleep matrix scan rate (Hz)', 'Power off after sleep (minutes)'):
            self.assertEqual(controls[label]['showIf'], '{id_hhkb_bt_matrix_wake} == 1')
        self.assertEqual([o[1] for o in controls['Sleep matrix scan rate (Hz)']['options']], list(range(1,11)))
