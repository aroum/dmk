"""Exercise consumer mapping, transport routing and packets through the real ISR."""
from pathlib import Path
import subprocess
import tempfile
import unittest
from .test_hybrid_button import ROOT, function


class HybridConsumerTest(unittest.TestCase):
    def test_all_consumer_usages_fifo_overflow_release_and_routing(self):
        source=(ROOT/'keyboards/hhkb/modules/hybrid/hybrid.c').read_text()
        usb=(ROOT/'dmk_core/drivers/app_usb_tinyusb.c').read_text()
        mapping=source[source.index('static const uint16_t consumer_usages'):source.index('static uint8_t queue_read;')]
        code=r'''
#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <assert.h>
#define HHKB_HYBRID 1
#define SPI_SR_OVR 1
#define SPI_SR_RXNE 2
#define SPI_SR_TXE 4
#define SPI_CR2_TXEIE 8
#define pdMS_TO_TICKS(x) (x)
static unsigned depth;
#define taskENTER_CRITICAL() (++depth)
#define taskEXIT_CRITICAL() (--depth)
static struct {uint32_t SR,CR2;} spi;
#define SPI1 (&spi)
static volatile uint8_t data;
static volatile uint8_t *const spi_data=&data;
static uint8_t report[8]={1},input_queue[8][8],consumer_bits[4],reply[9];
static uint8_t queue_count,queue_read,queue_write,control_command,control_slot,reply_length,reply_position;
static uint8_t notification_type,active_slot,battery_pending,battery_reported,display_slot,attention_retries;
static bool notification_value,pairing,pairing_active,battery_polled,hello_seen,control_polled;
static uint32_t profile_display_until,radio_disconnected_since;
static bool radio_link_wake,radio_disconnected;
static void hhkb_hybrid_notify_wake_from_isr(void){}
static uint32_t xTaskGetTickCountFromISR(void){return 0;}
typedef struct {uint8_t ReportID,Modifier,Reserved,Keycodes[6];} USB_HID_KeyboardReport_TypeDef;
typedef struct {uint8_t report_id;USB_HID_KeyboardReport_TypeDef keyboard;uint16_t consumer;} queued_key_report_t;
static unsigned usb_packets;
static bool manual_bt_output,auto_select_needed,matrix_sleeping,button_sleep_wake;
static uint8_t last_slot;
static uint32_t xTaskGetTickCount(void){return 0;}
static bool hhkb_hybrid_on_battery(void){return true;}
static void hhkb_bt_set_last_slot(uint8_t slot){last_slot=slot;}
static int s_keyboard_queue=1;
static bool bluetooth=true;
static USB_HID_KeyboardReport_TypeDef s_latest_keyboard_report;
static uint16_t s_latest_consumer;
static bool hhkb_hybrid_output_bluetooth(void){return bluetooth;}
static bool usb_enqueue_key_report(const queued_key_report_t *q){
 assert(!bluetooth);++usb_packets;
 assert(q->report_id==1 || q->report_id==2);return true;
}
'''+mapping
        for sig in ('static void start_reply(', 'void SPI1_IRQHandler(',
                    'static void resync_input_queue(', 'static void enqueue_input(',
                    'void hhkb_hybrid_report(', 'bool hhkb_hybrid_consumer_report('):
            code+=function(source,sig)+'\n'
        for sig in ('bool USB_HID_SendReport(', 'bool USB_HID_SendConsumerReport('):
            code+=function(usb,sig)+'\n'
        code+=function(source,'static void release_current_output(')
        code+=function(source,'bool hhkb_hybrid_request_slot(')
        code+=r'''
static void poll(uint8_t packet[8]){
 data=2;spi.SR=SPI_SR_RXNE;SPI1_IRQHandler();
 assert(data==0xaa && reply_length==8);
 for(unsigned i=0;i<8;i++){
  spi.SR=SPI_SR_TXE;SPI1_IRQHandler();packet[i]=data;
 }
 SPI1_IRQHandler();assert(!reply_length);
}
static void reset(void){
 memset(report,0,8);report[0]=1;memset(consumer_bits,0,4);
 queue_count=queue_read=queue_write=control_command=0;
 battery_pending=0;control_polled=false;reply_length=reply_position=0;
}
int main(void){
 const uint16_t expected[32]={
  0x06f,0x070,0x030,0x0b7,0x0e2,0x0e9,0x0ea,0x0b8,
  0x087,0x18a,0x192,0x088,0x196,0x221,0x223,0x224,
  0x225,0x226,0x227,0x22a,0x0cd,0x0b5,0x0b6,0x153,
  0x154,0x155,0x206,0x0b2,0x0b3,0x0b4,0x003,0x152};
 uint8_t p[8];
 for(unsigned i=0;i<32;i++){
  reset();assert(USB_HID_SendConsumerReport(expected[i]));
  assert(USB_HID_SendConsumerReport(0)); // Release before the first radio poll.
  assert(queue_count==2 && depth==0 && usb_packets==0);
  poll(p);assert(p[0]==2);
  for(unsigned j=0;j<4;j++)assert(p[j+1]==(j==i/8?(1u<<(i%8)):0));
  assert(!p[5] && !p[6] && !p[7]);
  poll(p);assert(p[0]==2);for(unsigned j=1;j<8;j++)assert(!p[j]);
  poll(p);assert(p[0]==1); // Consumer packets never corrupt the keyboard header.
 }
 reset();
 const uint8_t keys[6]={4};
 hhkb_hybrid_report(2,keys);hhkb_hybrid_consumer_report(0xe9);
 hhkb_hybrid_report(0,(const uint8_t[6]){0});hhkb_hybrid_consumer_report(0);
 poll(p);assert(p[0]==1 && p[1]==2 && p[2]==4);
 poll(p);assert(p[0]==2 && p[1]==0x20);
 poll(p);assert(p[0]==1 && !p[1] && !p[2]);
 poll(p);assert(p[0]==2 && !p[1]);
 // Overflow must resynchronize both latest full states, including releases.
 reset();hhkb_hybrid_consumer_report(0xe9);poll(p);assert(p[1]==0x20);
 for(unsigned i=0;i<8;i++)hhkb_hybrid_report(0,keys);
 assert(queue_count==8);hhkb_hybrid_consumer_report(0);assert(queue_count==2);
 poll(p);assert(p[0]==1 && p[2]==4);
 poll(p);assert(p[0]==2);for(unsigned i=1;i<8;i++)assert(!p[i]);
 reset();for(unsigned i=0;i<8;i++)hhkb_hybrid_consumer_report(0xea);
 hhkb_hybrid_report(0,(const uint8_t[6]){0});assert(queue_count==2);
 poll(p);assert(p[0]==1 && !p[2]);poll(p);assert(p[0]==2 && p[1]==0x40);
 // Unsupported codes cannot leak to USB or replace the pending BT state.
 reset();assert(!USB_HID_SendConsumerReport(0xffff) && queue_count==0 && usb_packets==0);
 control_command=4;assert(!USB_HID_SendConsumerReport(0xe9) && queue_count==0);
 control_command=0;hhkb_hybrid_consumer_report(0xe9);control_command=4;
 poll(p);assert(p[0]==0x40 && queue_count==1 && control_polled);
 control_command=0;control_polled=false;battery_pending=16;
 poll(p);assert(p[0]==0x80 && queue_count==1);poll(p);assert(p[0]==2 && p[1]==0x20);
 // Release both report families when leaving a transport, sleeping or powering off.
 reset();hhkb_hybrid_report(0,keys);hhkb_hybrid_consumer_report(0xe9);
 release_current_output();assert(queue_count==4);
 poll(p);poll(p);poll(p);assert(p[0]==1 && !p[2]);
 poll(p);assert(p[0]==2 && !p[1]);
 reset();hello_seen=true;matrix_sleeping=true;
 hhkb_hybrid_report(2,keys);hhkb_hybrid_consumer_report(0xe9);
 assert(hhkb_hybrid_request_slot(2,false));
 assert(control_command==6 && last_slot==2 && button_sleep_wake && !auto_select_needed);
 assert(queue_count==2 && !depth);
 for(unsigned i=1;i<8;i++)assert(input_queue[0][i]==0 && input_queue[1][i]==0);
 assert(input_queue[0][0]==1 && input_queue[1][0]==2);
 reset();
 bluetooth=false;assert(USB_HID_SendConsumerReport(0xe9));
 assert(usb_packets==1 && s_latest_consumer==0xe9 && queue_count==0);
 release_current_output();assert(usb_packets==3 && !s_latest_consumer && !queue_count && !depth);
}
'''
        with tempfile.TemporaryDirectory() as tmp:
            c=Path(tmp)/'consumer.c';binary=Path(tmp)/'consumer';c.write_text(code)
            subprocess.run(['cc','-std=c11','-Wall','-Wextra','-Werror',str(c),'-o',str(binary)],check=True)
            subprocess.run([str(binary)],check=True)
