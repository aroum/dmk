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
static bool button_handled, button_sleep_wake, matrix_sleeping=true, down;
static bool on_battery=true,sleep_requested;
static uint32_t button_down_since;
static unsigned longs, shorts, resets;
static bool hhkb_hybrid_button(void){return down;}
static bool hhkb_hybrid_on_battery(void){return on_battery;}
static void button_long_action(void){++longs;}
static void button_short_action(void){++shorts;}
static void NVIC_SystemReset(void){++resets;}
'''
        harness += function(source, 'bool hhkb_hybrid_take_button_wake(')
        harness += function(source, 'uint32_t hhkb_hybrid_button_wait_ms(')
        harness += function(source, 'void hhkb_hybrid_request_sleep(')
        harness += function(source, 'void hhkb_hybrid_button_poll(')
        harness += r'''
int main(void) {
 down=true; hhkb_hybrid_button_poll(100);
 assert(!hhkb_hybrid_take_button_wake());
 assert(hhkb_hybrid_button_wait_ms(100)==2000);
 hhkb_hybrid_button_poll(2099); assert(longs==0);
 hhkb_hybrid_button_poll(2100); assert(longs==1);
 hhkb_hybrid_button_poll(2200); assert(longs==1);
 down=false; hhkb_hybrid_button_poll(2300); assert(shorts==0 && !hhkb_hybrid_take_button_wake());
 down=true; hhkb_hybrid_button_poll(3000);
 down=false; hhkb_hybrid_button_poll(3100); assert(shorts==0 && hhkb_hybrid_take_button_wake());
 assert(!hhkb_hybrid_take_button_wake());
 down=true; hhkb_hybrid_button_poll(3200);
 down=false; hhkb_hybrid_button_poll(3299); assert(!hhkb_hybrid_take_button_wake());
 matrix_sleeping=false;
 down=true; hhkb_hybrid_button_poll(4000);
 down=false; hhkb_hybrid_button_poll(4100); assert(shorts==1);
 matrix_sleeping=true;
 down=true; hhkb_hybrid_button_poll(UINT32_MAX-100);
 hhkb_hybrid_button_poll(1899);
 assert(longs==2 && resets==0);
 on_battery=false;hhkb_hybrid_request_sleep();assert(!sleep_requested);
 on_battery=true;hhkb_hybrid_request_sleep();assert(sleep_requested);
}
'''
        with tempfile.TemporaryDirectory() as tmp:
            c = Path(tmp) / 'button.c'
            binary = Path(tmp) / 'button'
            c.write_text(harness)
            subprocess.run(['cc','-std=c11','-Wall','-Wextra','-Werror',str(c),'-o',str(binary)],check=True)
            subprocess.run([str(binary)],check=True)

    def test_shutdown_handshake_timeout_ack_and_usb_abort(self):
        source = (ROOT / 'keyboards/hhkb/modules/hybrid/hybrid.c').read_text()
        harness = r'''
#include <stdbool.h>
#include <stdint.h>
#include <assert.h>
static bool powered=true,poweroff_pending,battery=true,off_from_battery,control_polled;
static struct {uint32_t KR;} iwdg;
#define IWDG (&iwdg)
static unsigned mode,calls,delays,releases;
static uint8_t queue_count,control_command,attention_retries,active_slot,reply_length;
static struct {uint32_t SR;} spi;
#define SPI1 (&spi)
#define SPI_SR_BSY 0x80
static uint32_t power_off_flash_until;
static struct {uint32_t BSRR;} gpio;
#define GPIOD (&gpio)
#define SPI1_IRQn 0
#define pdMS_TO_TICKS(x) (x)
static bool hhkb_hybrid_on_battery(void){return battery;}
static bool hhkb_hybrid_output_bluetooth(void){return true;}
static void hhkb_hybrid_refresh_indicators(void){assert(poweroff_pending);}
static void release_current_output(void){++releases;}
static void NVIC_DisableIRQ(int x){(void)x;}
static void NVIC_EnableIRQ(int x){(void)x;}
static uint32_t xTaskGetTickCount(void){return 100;}
static void vTaskDelay(unsigned x){delays+=x;}
void hhkb_hybrid_toggle_power(void);
static void hhkb_hybrid_service(void){
 ++calls;
 if(mode==1 && calls==2){control_command=0;active_slot=255;}
 if(mode==2 && calls==2)battery=false;
 if(mode==3)hhkb_hybrid_toggle_power();
 if(mode==4){if(calls==2){control_command=0;active_slot=255;spi.SR=0x80;}if(calls==1600)spi.SR=0;}
}
'''
        harness += function(source, 'void hhkb_hybrid_toggle_power(')
        harness += r'''
static void setup(unsigned m){mode=m;powered=true;poweroff_pending=false;battery=true;calls=delays=releases=0;gpio.BSRR=0;spi.SR=0;active_slot=0;control_command=0;}
int main(void){
 setup(1);hhkb_hybrid_toggle_power();assert(!powered && gpio.BSRR==4 && calls==1501 && releases==1);
 hhkb_hybrid_toggle_power();assert(releases==1);
 setup(0);hhkb_hybrid_toggle_power();assert(!powered && delays==3000 && gpio.BSRR==4);
 setup(2);hhkb_hybrid_toggle_power();assert(powered && !poweroff_pending && gpio.BSRR==0);
 setup(3);hhkb_hybrid_toggle_power();assert(!powered && releases==1 && calls==3000);
 setup(4);hhkb_hybrid_toggle_power();assert(!powered && calls==1600);
}
'''
        with tempfile.TemporaryDirectory() as tmp:
            c = Path(tmp) / 'shutdown.c'
            binary = Path(tmp) / 'shutdown'
            c.write_text(harness)
            subprocess.run(['cc','-std=c11','-Wall','-Wextra','-Werror',str(c),'-o',str(binary)],check=True)
            subprocess.run([str(binary)],check=True)

    def test_off_ten_second_guard(self):
        source=(ROOT/'keyboards/hhkb/modules/hybrid/hybrid_stop.c').read_text()
        code='#include <stdint.h>\n#include <stdbool.h>\n#include <assert.h>\n'
        code+=function(source,'static unsigned off_press_action(')
        code+=r'''
int main(void){
 assert(off_press_action(288,true)==0);
 assert(off_press_action(289,false)==0);
 assert(off_press_action(289,true)==1);
 assert(off_press_action(2889,true)==1);
 assert(off_press_action(2890,false)==2);
 assert(off_press_action(3000,true)==2);
}
'''
        with tempfile.TemporaryDirectory() as tmp:
            c=Path(tmp)/'guard.c';binary=Path(tmp)/'guard'
            c.write_text(code)
            subprocess.run(['cc','-std=c11','-Wall','-Wextra','-Werror',str(c),'-o',str(binary)],check=True)
            subprocess.run([str(binary)],check=True)

    def test_power_settings_round_trip_and_legacy(self):
        source=(ROOT/'keyboards/hhkb/modules/ec/ec_settings.c').read_text()
        code='#include <stdint.h>\n#include <stdbool.h>\n#include <assert.h>\n'
        code+=function(source,'static uint8_t bt_power_encode(')
        code+=function(source,'static bool bt_power_decode(')
        code+=r'''
int main(void){
 for(uint8_t slot=0;slot<4;slot++)for(uint8_t i=0;i<10;i++)for(uint8_t a=0;a<3;a++){
  uint8_t ri=255,ra=255,byte=0x80|(slot<<5)|bt_power_encode(i,a);
  assert(bt_power_decode(byte,true,&ri,&ra));assert(ri==i && ra==a);
  assert(((byte>>5)&3)==slot);
 }
 for(uint8_t a=0;a<3;a++){
  uint8_t i=255,r=255;
  assert(bt_power_decode(0x80|7|(a<<3),false,&i,&r));assert(i==2 && r==a);
 }
 uint8_t i=2,a=0;
 assert(!bt_power_decode(31,true,&i,&a));assert(i==2 && a==0);
}
'''
        with tempfile.TemporaryDirectory() as tmp:
            c=Path(tmp)/'settings.c';binary=Path(tmp)/'settings'
            c.write_text(code)
            subprocess.run(['cc','-std=c11','-Wall','-Wextra','-Werror',str(c),'-o',str(binary)],check=True)
            subprocess.run([str(binary)],check=True)



if __name__ == '__main__':
    unittest.main()
