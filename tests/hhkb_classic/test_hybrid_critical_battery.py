"""Exercise the production critical-battery timeout, PC5 and LED priority."""
from pathlib import Path
import subprocess
import tempfile
import unittest

from .test_hybrid_button import ROOT, function


class HybridCriticalBatteryTest(unittest.TestCase):
    def test_critical_battery_timeout_button_and_led(self):
        source = (ROOT / 'keyboards/hhkb/modules/hybrid/hybrid.c').read_text()
        code = r'''
#include <stdint.h>
#include <stdbool.h>
#include <assert.h>
static bool powered=true,poweroff_pending,critical_battery,battery=true;
static bool off_from_battery,button_was_down,button_handled,button_sleep_wake;
static bool matrix_sleeping,down,pairing,pairing_active;
static uint32_t critical_battery_since,button_down_since,power_off_flash_until;
static uint32_t battery_display_until,profile_display_until;
static uint8_t display_slot,level=100;
static unsigned deep,shorts,longs,refreshes,resets;
static bool hhkb_hybrid_on_battery(void){return battery;}
static uint8_t hhkb_hybrid_battery_level(void){return level;}
static bool hhkb_hybrid_button(void){return down;}
static uint8_t hhkb_bt_indicator_flags(void){return 0;}
static void hhkb_hybrid_refresh_indicators(void){++refreshes;}
static void hhkb_hybrid_enter_deep_sleep(void){++deep;powered=false;}
static void button_short_action(void){++shorts;}
static void button_long_action(void){++longs;}
static void NVIC_SystemReset(void){++resets;}
'''
        for signature in ('bool hhkb_hybrid_critical_battery(',
                          'void hhkb_hybrid_critical_battery_poll(',
                          'void hhkb_hybrid_button_poll(',
                          'bool hhkb_hybrid_led_override('):
            code += function(source, signature)
        code += r'''
static void setup(void){
 powered=true;poweroff_pending=false;critical_battery=false;battery=true;
 off_from_battery=false;button_was_down=button_handled=button_sleep_wake=false;
 matrix_sleeping=down=false;level=0;deep=shorts=longs=refreshes=resets=0;
}
int main(void){
 setup();battery=false;
 hhkb_hybrid_critical_battery_poll(0);hhkb_hybrid_critical_battery_poll(180000);
 assert(!hhkb_hybrid_critical_battery() && !deep);
 battery=true;level=5;hhkb_hybrid_critical_battery_poll(180001);assert(!critical_battery);
 level=15;hhkb_hybrid_critical_battery_poll(180002);assert(!critical_battery);
 level=0;hhkb_hybrid_critical_battery_poll(200000);
 assert(critical_battery && refreshes==1);
 hhkb_hybrid_critical_battery_poll(379999);assert(!deep);
 hhkb_hybrid_critical_battery_poll(380000);assert(deep==1 && !powered);
 // Unsigned timeout remains correct across the millisecond counter wrap.
 setup();hhkb_hybrid_critical_battery_poll(UINT32_MAX-1000);
 hhkb_hybrid_critical_battery_poll(178998);assert(!deep);
 hhkb_hybrid_critical_battery_poll(178999);assert(deep==1);
 // USB or recovered PB9 cancels the warning and resets the next deadline.
 setup();hhkb_hybrid_critical_battery_poll(0);battery=false;
 hhkb_hybrid_critical_battery_poll(179999);assert(!critical_battery && refreshes==2);
 hhkb_hybrid_critical_battery_poll(180000);assert(!deep);
 battery=true;hhkb_hybrid_critical_battery_poll(180001);
 level=100;hhkb_hybrid_critical_battery_poll(180002);assert(!critical_battery);
 level=0;hhkb_hybrid_critical_battery_poll(180003);
 hhkb_hybrid_critical_battery_poll(360002);assert(!deep);
 hhkb_hybrid_critical_battery_poll(360003);assert(deep==1);
 // Safety orange overrides sleep blanking and disabled VIA status LEDs.
 setup();hhkb_hybrid_critical_battery_poll(0);matrix_sleeping=true;
 bool on=true;assert(hhkb_hybrid_led_override(0,0,&on) && !on);
 on=false;assert(hhkb_hybrid_led_override(1,0,&on) && on);
 battery=false;on=true;assert(hhkb_hybrid_led_override(1,0,&on) && !on);
 // Short presses do nothing; a held long press bypasses configured actions.
 setup();hhkb_hybrid_critical_battery_poll(0);
 down=true;hhkb_hybrid_button_poll(100);
 down=false;hhkb_hybrid_button_poll(300);assert(!shorts && !button_sleep_wake);
 down=true;hhkb_hybrid_button_poll(1000);
 hhkb_hybrid_button_poll(2999);assert(!deep);
 hhkb_hybrid_button_poll(3000);assert(deep==1 && !longs && !resets);
 // Release after the threshold also works if the held sample was delayed.
 setup();hhkb_hybrid_critical_battery_poll(0);down=true;hhkb_hybrid_button_poll(100);
 down=false;hhkb_hybrid_button_poll(2100);assert(deep==1 && !longs);
 // Normal configurable PC5 actions still run after recovery.
 setup();hhkb_hybrid_critical_battery_poll(0);level=100;
 hhkb_hybrid_critical_battery_poll(1);down=true;hhkb_hybrid_button_poll(100);
 hhkb_hybrid_button_poll(2100);assert(longs==1 && !deep);
}
'''
        with tempfile.TemporaryDirectory() as tmp:
            c = Path(tmp) / 'critical.c'
            binary = Path(tmp) / 'critical'
            c.write_text(code)
            subprocess.run(['cc', '-std=c11', '-Wall', '-Wextra', '-Werror',
                            str(c), '-o', str(binary)], check=True)
            subprocess.run([str(binary)], check=True)
