"""Exercise bounded USB enqueue and full-state recovery using production functions."""
from pathlib import Path
import subprocess
import tempfile
import unittest
from .test_hybrid_button import ROOT, function


class UsbRetryTest(unittest.TestCase):
    def test_stalled_host_and_resync(self):
        source = (ROOT/'dmk_core/drivers/app_usb_tinyusb.c').read_text()
        production = '\n'.join(function(source, x) for x in (
            'void USB_HID_ResyncKeyboard(', 'static void usb_keyboard_flush(',
            'static bool usb_enqueue_key_report('))
        harness = r'''
#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#define KEYBOARD_REPORT_ENQUEUE_TRIES 8
#define pdTRUE 1
#define pdMS_TO_TICKS(x) (x)
typedef struct {unsigned Modifier,Keycodes;} Keyboard;
typedef struct {unsigned report_id;Keyboard keyboard;uint16_t consumer;} queued_key_report_t;
static void *s_keyboard_queue=(void*)1,*s_usb_task_handle=(void*)2;
static Keyboard s_latest_keyboard_report={2,4};
static uint16_t s_latest_consumer=42;
static volatile bool s_resync_pending;
static unsigned depth,attempts,notifications,count,sent,blocked;
static bool mounted=true,suspended,ready=true;
static queued_key_report_t queue[2];
#define taskENTER_CRITICAL() (++depth)
#define taskEXIT_CRITICAL() (--depth)
static bool tud_mounted(void){return mounted;}
static bool tud_suspended(void){return suspended;}
static bool tud_hid_ready(void){return ready;}
static bool keyboard_power_saving_enabled(void){return true;}
static void tud_remote_wakeup(void){}
static void usb_keyboard_report_armed(void){}
static void xTaskNotifyGive(void *p){(void)p;++notifications;}
static unsigned uxQueueMessagesWaiting(void *p){(void)p;return count;}
static void xQueueReset(void *p){(void)p;assert(depth);count=0;}
static int xQueueSend(void *p,const queued_key_report_t *r,unsigned timeout){
 (void)p;++attempts;
 if(timeout){assert(!depth);if(blocked)return 0;}
 else assert(depth); // Reset and recovery must not race queue producers.
 if(count==2)return 0;queue[count++]=*r;return pdTRUE;
}
static int xQueuePeek(void *p,queued_key_report_t *r,unsigned timeout){
 (void)p;(void)timeout;if(!count)return 0;*r=queue[0];return 1;
}
static int xQueueReceive(void *p,queued_key_report_t *r,unsigned timeout){
 (void)p;(void)timeout;*r=queue[0];queue[0]=queue[1];--count;return 1;
}
static bool tud_hid_keyboard_report(unsigned id,unsigned mod,unsigned keys){
 assert(!depth && id==1 && mod==2 && keys==4);++sent;return true;
}
static bool tud_hid_report(unsigned id,const void *data,unsigned size){
 assert(!depth && id==2 && size==2 && *(const uint16_t*)data==42);++sent;return true;
}
'''
        harness += production + r'''
int main(void){
 queued_key_report_t event={.report_id=1};
 blocked=1;
 assert(!usb_enqueue_key_report(&event));
 assert(attempts==8 && s_resync_pending && notifications==1 && !depth);
 ready=false;usb_keyboard_flush();
 assert(!s_resync_pending && count==2 && !sent && !depth);
 ready=true;usb_keyboard_flush();usb_keyboard_flush();
 assert(sent==2 && !count && !depth);
 blocked=0;attempts=0;assert(usb_enqueue_key_report(&event));assert(attempts==1);
 mounted=false;attempts=0;assert(usb_enqueue_key_report(&event));assert(!attempts);
 return 0;
}
'''
        with tempfile.TemporaryDirectory() as tmp:
            src=Path(tmp)/'retry.c';src.write_text(harness)
            out=Path(tmp)/'retry'
            subprocess.run(['clang','-std=c11','-Wall','-Wextra','-Werror',str(src),'-o',str(out)],check=True)
            subprocess.run([str(out)],check=True)
