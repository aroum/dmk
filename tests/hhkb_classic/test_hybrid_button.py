"""Exercise production PC5 handling without MCU hardware."""
from pathlib import Path
import subprocess
import tempfile
import unittest

ROOT = Path(__file__).resolve().parents[2]


def function(source, signature):
    start = source.index(signature)
    pos = source.index('{', start) + 1
    depth = 1
    while depth:
        depth += (source[pos] == '{') - (source[pos] == '}')
        pos += 1
    return source[start:pos]


class HybridButtonTest(unittest.TestCase):
    def test_wake_press_keeps_long_action_and_suppresses_only_short_action(self):
        source = (ROOT / 'keyboards/hhkb/modules/hybrid/hybrid.c').read_text()
        harness = r'''
#include <stdbool.h>
#include <stdint.h>
#include <assert.h>
static bool powered=true, off_from_battery, button_was_down;
static bool button_handled, button_wake_press, down;
static uint32_t button_down_since;
static unsigned longs, shorts, resets;
static bool hhkb_hybrid_button(void){return down;}
static bool hhkb_hybrid_on_battery(void){return true;}
static void button_long_action(void){++longs;}
static void button_short_action(void){++shorts;}
static void NVIC_SystemReset(void){++resets;}
'''
        harness += function(source, 'void hhkb_hybrid_button_woke(')
        harness += function(source, 'void hhkb_hybrid_button_poll(')
        harness += r'''
int main(void) {
 down=true; hhkb_hybrid_button_poll(100); hhkb_hybrid_button_woke();
 hhkb_hybrid_button_poll(2099); assert(longs==0);
 hhkb_hybrid_button_poll(2100); assert(longs==1);
 hhkb_hybrid_button_poll(2200); assert(longs==1);
 down=false; hhkb_hybrid_button_poll(2300); assert(shorts==0);
 down=true; hhkb_hybrid_button_poll(3000); hhkb_hybrid_button_woke();
 down=false; hhkb_hybrid_button_poll(3100); assert(shorts==0);
 down=true; hhkb_hybrid_button_poll(4000);
 down=false; hhkb_hybrid_button_poll(4100); assert(shorts==1);
 down=true; hhkb_hybrid_button_poll(UINT32_MAX-100);
 hhkb_hybrid_button_woke(); hhkb_hybrid_button_poll(1899);
 assert(longs==2 && resets==0);
}
'''
        with tempfile.TemporaryDirectory() as tmp:
            c = Path(tmp) / 'button.c'
            binary = Path(tmp) / 'button'
            c.write_text(harness)
            subprocess.run(['cc','-std=c11','-Wall','-Wextra','-Werror',str(c),'-o',str(binary)],check=True)
            subprocess.run([str(binary)],check=True)

    def test_stop_sleep_and_off_hold_boundaries(self):
        source = (ROOT / 'keyboards/hhkb/modules/hybrid/hybrid_stop.c').read_text()
        harness = '#include <stdbool.h>\n#include <stdint.h>\n#include <assert.h>\n'
        harness += function(source, 'static unsigned stop_button_decision(')
        harness += r'''
int main(void) {
 assert(stop_button_decision(true, 288, true)==0);
 assert(stop_button_decision(true, 289, false)==0);
 assert(stop_button_decision(true, 289, true)==1);
 assert(stop_button_decision(true, 577, true)==1);
 assert(stop_button_decision(true, 578, false)==2);
 assert(stop_button_decision(false, 288, false)==0);
 assert(stop_button_decision(false, 289, false)==1);
}
'''
        with tempfile.TemporaryDirectory() as tmp:
            c = Path(tmp) / 'stop.c'
            binary = Path(tmp) / 'stop'
            c.write_text(harness)
            subprocess.run(['cc','-std=c11','-Wall','-Wextra','-Werror',str(c),'-o',str(binary)],check=True)
            subprocess.run([str(binary)],check=True)


if __name__ == '__main__':
    unittest.main()
