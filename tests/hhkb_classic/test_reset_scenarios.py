"""Exercise the production VIA reset dispatcher and recovery entry points."""
import subprocess
import tempfile
import unittest
from pathlib import Path

from .test_hybrid_button import ROOT, function


class ResetScenariosTest(unittest.TestCase):
    def test_via_actions_delays_and_failures(self):
        source = (ROOT / 'dmk_core/features/vial.c').read_text()
        start = source.index('#ifdef VIA_FULL_SETTINGS_RESET\nenum {')
        end = source.index('static void vial_storage_init_default', start)
        dispatcher = source[start:end].replace(
            '__attribute__((weak)) bool vial_keyboard_reset_allowed(void) { return true; }',
            'static bool vial_keyboard_reset_allowed(void) { return allowed; }')
        code = r'''
#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <setjmp.h>
#define VIA_FULL_SETTINGS_RESET 1
#define HHKB_COMPACT_STORAGE 1
#define VIA_CUSTOM_GET_VALUE 8
#define VIA_CUSTOM_SET_VALUE 7
#define pdMS_TO_TICKS(ms) (ms)
typedef unsigned TickType_t;
static unsigned ticks, settings_erases, keymap_erases, resets, recoveries;
static bool allowed=true, erase_ok=true, s_eeprom_dirty;
static jmp_buf rebooted;
static unsigned xTaskGetTickCount(void){return ticks;}
static bool vial_keyboard_settings_reset(void){++settings_erases;return erase_ok;}
static bool vial_platform_settings_reset(void){++keymap_erases;return erase_ok;}
static void platform_system_reset(void){++resets;longjmp(rebooted,1);}
void hhkb_enter_bank1(void){++recoveries;longjmp(rebooted,1);}
''' + dispatcher + r'''
static void poll(void){
''' + function(source, 'if (s_reset_pending)') + r'''
}
static bool command(unsigned op,unsigned id,unsigned value){
 uint8_t request[32]={op,0,id,value},response[32]={op};
 assert(vial_settings_reset_command(request,response));
 return response[0]!=255;
}
static void fresh(void){
 s_reset_armed=s_reset_pending=s_reset_erased=s_reset_recovery=s_recovery_erase=false;
 settings_erases=keymap_erases=resets=recoveries=ticks=0;
 s_reset_status="Ready";allowed=erase_ok=s_eeprom_dirty=true;
}
int main(void){
 for(unsigned scenario=0;scenario<3;scenario++){
  fresh();unsigned action=scenario?64:38;
  assert(!command(7,action,1)); // Both actions require arming.
  assert(command(7,63,scenario==2));assert(command(7,37,1));
  assert(command(7,action,1));assert(!s_reset_armed && s_reset_pending);
  assert(!command(7,63,scenario!=2)); // Pending action cannot be changed.
  poll();ticks=149;poll();assert(!settings_erases && !keymap_erases && !resets && !recoveries);
  ticks=150;if(!setjmp(rebooted))poll();
  assert(!s_eeprom_dirty);
  assert(settings_erases==(scenario!=1) && keymap_erases==(scenario!=1));
  assert(resets==(scenario==0) && recoveries==(scenario!=0));
 }
 fresh();allowed=false;assert(command(7,37,1));assert(!command(7,64,1));
 assert(!s_reset_pending && !strcmp(s_reset_status,"Set SW6 to OFF"));
 fresh();assert(command(7,37,1));assert(command(7,64,1));allowed=false;ticks=150;poll();
 assert(!recoveries && !settings_erases);
 fresh();erase_ok=false;assert(command(7,63,1));assert(command(7,37,1));
 assert(command(7,64,1));ticks=150;poll();
 assert(!recoveries && !resets && !s_reset_erased && !strcmp(s_reset_status,"Reset failed; retry"));
 assert(command(7,37,1)); // Failure can be retried.
}
'''
        self.run_c(code)

    def test_bank1_entry_matches_classic_and_hybrid_recovery(self):
        source = (ROOT / 'keyboards/hhkb/modules/dip_switch/dip_switch.c').read_text()
        code = r'''
#include <assert.h>
#include <stdint.h>
#define HHKB_EEPROM_BANK2_STATUS_ADDR 0x08080348u
#define HHKB_EEPROM_BANK2_CRC_ADDR 0x08080370u
static unsigned count,erases;
static uint32_t addresses[3];static uint8_t values[3];
static void write_eeprom_byte(uint32_t address,uint8_t value){addresses[count]=address;values[count++]=value;}
static void erase_bank2_vector_page_ram(void){++erases;}
''' + function(source, 'void hhkb_enter_bank1(') + r'''
int main(void){
 hhkb_enter_bank1();assert(erases==1);
#ifdef HHKB_HYBRID
 assert(count==1 && addresses[0]==0x08080360u && values[0]==0);
#else
 assert(count==3 && addresses[0]==HHKB_EEPROM_BANK2_STATUS_ADDR && values[0]==1);
 assert(addresses[1]==HHKB_EEPROM_BANK2_CRC_ADDR && values[1]==0);
 assert(addresses[2]==HHKB_EEPROM_BANK2_CRC_ADDR+1 && values[2]==0);
#endif
}
'''
        self.run_c(code)
        self.run_c(code, '-DHHKB_HYBRID')

    @staticmethod
    def run_c(code, *flags):
        with tempfile.TemporaryDirectory() as directory:
            source = Path(directory) / 'reset.c'
            binary = Path(directory) / 'reset'
            source.write_text(code)
            subprocess.run(['clang', '-std=c11', '-Wall', '-Wextra', '-Werror',
                            *flags, str(source), '-o', str(binary)], check=True)
            subprocess.run([str(binary)], check=True)
