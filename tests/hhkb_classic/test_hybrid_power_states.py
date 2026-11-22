"""Exercise production Nordic-idle sleep and USB soft-OFF transitions."""
from pathlib import Path
import subprocess
import tempfile
import unittest

from .test_hybrid_button import ROOT, function


class HybridPowerStatesTest(unittest.TestCase):
    def test_sleep_blanks_leds_on_usb_as_well_as_battery(self):
        source = (ROOT / 'keyboards/hhkb/modules/indicators_system/indicators_system.c').read_text()
        code = r'''
#include <stdint.h>
#include <stdbool.h>
#include <assert.h>
typedef uint32_t TickType_t;
#define HHKB_HYBRID 1
#define pdMS_TO_TICKS(x) (x)
static const unsigned led_pins[2]={8,9};
static bool caps_lock=true,output[10],sleeping,battery,powered=true;
static bool keyboard_power_saving_enabled(void){return true;}
static bool tud_mounted(void){return true;}
static bool tud_suspended(void){return false;}
static bool hhkb_hybrid_on_battery(void){return battery;}
static bool hhkb_hybrid_output_bluetooth(void){return battery;}
static bool hhkb_hybrid_powered(void){return powered;}
static uint8_t hhkb_get_led_mode(unsigned i){(void)i;return 2;}
static uint16_t layers_get_state(void){return 1;}
static uint8_t hhkb_get_system(void){return 0;}
static void hal_gpio_put(unsigned pin,bool on){output[pin]=on;}
static bool hhkb_hybrid_led_override(unsigned i,uint32_t now,bool *on){
 (void)i;(void)now;if(!sleeping && powered)return false;*on=false;return true;
}
'''
        code += function(source, 'void keyboard_platform_tick(')
        code += r'''
int main(void){
 keyboard_platform_tick(0);assert(output[8] && output[9]);
 sleeping=true;keyboard_platform_tick(0);assert(!output[8] && !output[9]);
 battery=true;keyboard_platform_tick(0);assert(!output[8] && !output[9]);
 sleeping=false;powered=false;battery=false;
 keyboard_platform_tick(0);assert(!output[8] && !output[9]);
}
'''
        with tempfile.TemporaryDirectory() as tmp:
            c = Path(tmp) / 'sleep_led.c'
            binary = Path(tmp) / 'sleep_led'
            c.write_text(code)
            subprocess.run(['cc', '-std=c11', '-Wall', '-Wextra', '-Werror',
                            '-Wsign-compare', str(c), '-o', str(binary)], check=True)
            subprocess.run([str(binary)], check=True)

    def test_sleep_idle_reconnect_usb_off_and_cancelled_wake(self):
        source = (ROOT / 'keyboards/hhkb/modules/hybrid/hybrid.c').read_text()
        code = r'''
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <assert.h>
typedef uint32_t TickType_t;
#define pdMS_TO_TICKS(x) (x)
#define SPI1_IRQn 1
#define SPI_SR_BSY 0x80
#define SPI_CR2_TXEIE 0x40
static struct {uint32_t SR,CR2;} spi;
#define SPI1 (&spi)
static struct {uint32_t KR;} iwdg;
#define IWDG (&iwdg)
static struct {uint32_t BRR,BSRR;} gpio;
#define GPIOB (&gpio)
static bool powered=true,poweroff_pending,matrix_sleeping,sleep_requested;
static bool button_sleep_wake,button_was_down,button_handled,control_polled;
static bool notification_value,auto_select_needed,off_from_battery;
static bool off_retry_pending,off_wait_release,off_wake_ready,idle_polling;
static bool radio_link_wake,radio_disconnected,pairing,pairing_active;
static bool battery=true,manual_bt_output,source_seen,previous_battery,hello_seen=true;
static bool down,ack=true,disconnect=true;
static uint32_t now,button_down_since,power_off_flash_until;
static uint32_t idle_poll_started,off_idle_since,radio_disconnected_since;
static uint8_t control_command,queue_count,queue_read,queue_write,attention_retries;
static uint8_t active_slot=255,reply_length,reply_position,battery_reported=100,battery_pending;
static uint8_t report[8],consumer_bits[4],spi_data_register;
static volatile uint8_t *const spi_data=&spi_data_register;
static unsigned calls,idles,releases,selections,resets;
static bool hhkb_hybrid_on_battery(void){return battery;}
static bool hhkb_hybrid_output_bluetooth(void){return battery || manual_bt_output;}
static bool hhkb_hybrid_button(void){return down;}
static uint32_t xTaskGetTickCount(void){return now;}
static uint8_t hhkb_hybrid_battery_level(void){return 100;}
static uint8_t hhkb_bt_last_slot(void){return 2;}
static void hhkb_bt_set_last_slot(uint8_t slot){assert(slot<4);}
static void hhkb_hybrid_refresh_indicators(void){}
static void release_current_output(void){++releases;queue_count=0;}
static void NVIC_DisableIRQ(unsigned irq){(void)irq;}
static void NVIC_EnableIRQ(unsigned irq){(void)irq;}
static void NVIC_SystemReset(void){++resets;}
static void vTaskDelay(uint32_t ticks){now+=ticks;}
static void hal_sleep_us(unsigned us){(void)us;}
static void hhkb_hybrid_report(uint8_t m,const uint8_t k[6]){(void)m;(void)k;}
static bool hhkb_hybrid_consumer_report(uint16_t u){(void)u;return true;}
static bool hhkb_hybrid_request_slot(uint8_t slot,bool pair){
 assert(slot==2 && !pair);++selections;auto_select_needed=false;return true;
}
static void actual_service(void);
static void hhkb_hybrid_service(void){
 ++calls;actual_service();
 if(control_command==4){++idles;if(ack){control_command=0;control_polled=false;if(disconnect)active_slot=255;}}
}
'''
        for signature in ('static bool nordic_idle_wait(',
                          'void hhkb_hybrid_sleep_radio_idle(',
                          'bool hhkb_hybrid_take_link_wake(',
                          'bool hhkb_hybrid_disconnected_sleep_due(',
                          'void hhkb_hybrid_wake_radio(',
                          'void hhkb_hybrid_resume_after_stop(',
                          'void hhkb_hybrid_off_service('):
            code += function(source, signature)
        code += function(source, 'void hhkb_hybrid_service(').replace(
            'void hhkb_hybrid_service(', 'static void actual_service(', 1)
        code += r'''
static void setup(bool on_battery){
 powered=true;poweroff_pending=matrix_sleeping=sleep_requested=false;
 button_sleep_wake=button_was_down=button_handled=control_polled=false;
 notification_value=auto_select_needed=off_from_battery=false;
 off_retry_pending=off_wait_release=off_wake_ready=idle_polling=false;
 radio_link_wake=radio_disconnected=pairing=pairing_active=false;
 manual_bt_output=source_seen=previous_battery=down=false;
 battery=on_battery;hello_seen=ack=disconnect=true;
 now=button_down_since=power_off_flash_until=0;
 idle_poll_started=off_idle_since=radio_disconnected_since=0;
 control_command=queue_count=reply_length=attention_retries=0;
 active_slot=255;calls=idles=releases=selections=resets=0;spi.SR=0;
}
int main(void){
 // Sleep asks Nordic to idle and cannot reselect a profile until wake.
 setup(true);matrix_sleeping=true;auto_select_needed=true;
 actual_service();assert(!selections);
 radio_link_wake=true;hhkb_hybrid_sleep_radio_idle();
 assert(idles && releases==1 && !auto_select_needed && !radio_link_wake);
 assert(!control_command && !selections);
 hhkb_hybrid_wake_radio();matrix_sleeping=false;actual_service();assert(selections==1);
 // Wake must not overwrite a configured pairing/profile action or live link.
 setup(true);pairing=true;control_command=3;
 hhkb_hybrid_wake_radio();assert(!auto_select_needed && control_command==3);
 setup(true);control_command=6;
 hhkb_hybrid_wake_radio();assert(!auto_select_needed && control_command==6);
 setup(true);active_slot=1;hhkb_hybrid_wake_radio();assert(!auto_select_needed);
 // Standby repeats a consumed idle exchange while disconnect is unconfirmed.
 setup(true);matrix_sleeping=true;active_slot=0;disconnect=false;
 hhkb_hybrid_sleep_radio_idle();assert(idles==7 && now==3000 && !selections);
 // Explicit link-connect notifications can wake sleep once.
 radio_link_wake=true;assert(hhkb_hybrid_take_link_wake());
 assert(!hhkb_hybrid_take_link_wake());
 // A missing in-flight idle follow-up cannot block wake indefinitely.
 setup(true);ack=false;control_command=4;control_polled=true;
 hhkb_hybrid_wake_radio();assert(resets==1 && now==100);
 setup(true);
 // Sixty-second disconnected standby, without imposing it on USB output.
 radio_disconnected=true;radio_disconnected_since=100;
 assert(!hhkb_hybrid_disconnected_sleep_due(60099));
 assert(hhkb_hybrid_disconnected_sleep_due(60100));
 battery=false;assert(!hhkb_hybrid_disconnected_sleep_due(60100));
 manual_bt_output=true;assert(hhkb_hybrid_disconnected_sleep_due(60100));
 pairing=true;assert(!hhkb_hybrid_disconnected_sleep_due(60100));
 // USB soft-OFF waits for the old OFF press to be released.
 setup(false);powered=false;off_wait_release=true;down=true;
 hhkb_hybrid_off_service(0);assert(!powered && !off_wake_ready && iwdg.KR==0xAAAA);
 down=false;hhkb_hybrid_off_service(10);
 down=true;hhkb_hybrid_off_service(100);
 hhkb_hybrid_off_service(1099);assert(!off_wake_ready && !powered);
 hhkb_hybrid_off_service(1100);assert(off_wake_ready && !powered);
 down=false;hhkb_hybrid_off_service(1200);assert(powered && !resets);
 // Short press does not wake USB OFF.
 setup(false);powered=false;down=true;hhkb_hybrid_off_service(100);
 down=false;hhkb_hybrid_off_service(999);assert(!powered);
 // Ten seconds cancels wake, warns for three seconds, then repeats idle.
 setup(false);powered=false;down=true;hhkb_hybrid_off_service(100);
 hhkb_hybrid_off_service(10100);assert(button_handled && !off_wake_ready && !powered);
 down=false;hhkb_hybrid_off_service(13099);assert(!powered);
 unsigned before=idles;hhkb_hybrid_off_service(13100);
 assert(!powered && !auto_select_needed && idles>before && !resets);
 // Periodic USB idle retries are nonblocking; PC5 still works if nRF stalls.
 setup(false);powered=false;ack=false;
 hhkb_hybrid_off_service(2999);assert(!idle_polling);
 hhkb_hybrid_off_service(3000);assert(idle_polling && control_command==4);
 control_polled=true;hhkb_hybrid_off_service(6000);
 assert(!idle_polling && !control_polled && !control_command);
 down=true;hhkb_hybrid_off_service(6100);
 down=false;hhkb_hybrid_off_service(7100);assert(powered && !resets);
 // Wake must restore the Nordic keyboard discriminator, not just clear keys.
 for(unsigned attempt=0;attempt<3;attempt++){
  setup(true);memset(report,0xff,sizeof(report));
  hhkb_hybrid_resume_after_stop(true);
  assert(powered && report[0]==1 && auto_select_needed);
  for(unsigned i=1;i<sizeof(report);i++)assert(report[i]==0);
 }
 // A cancelled battery wake restores SPI, repeats idle and remains OFF.
 setup(true);hhkb_hybrid_resume_after_stop(false);
 assert(!powered && off_retry_pending && !auto_select_needed && report[0]==1);
 hhkb_hybrid_off_service(0);assert(!powered && idles && now>=1500 && !off_retry_pending);
 // Inserting USB into battery OFF retains the reset-based transition.
 off_from_battery=true;battery=false;hhkb_hybrid_off_service(0);assert(resets==1);
 // Removing USB while soft-OFF selects the battery STOP path, not power-on.
 setup(false);powered=false;battery=true;hhkb_hybrid_off_service(0);
 assert(!powered && off_from_battery && !resets);
}
'''
        with tempfile.TemporaryDirectory() as tmp:
            c = Path(tmp) / 'power_states.c'
            binary = Path(tmp) / 'power_states'
            c.write_text(code)
            subprocess.run(['cc', '-std=c11', '-Wall', '-Wextra', '-Werror',
                            '-Wsign-compare', str(c), '-o', str(binary)], check=True)
            subprocess.run([str(binary)], check=True)
