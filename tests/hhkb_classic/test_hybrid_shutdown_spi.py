"""Clock the production ISR through the MIT shutdown transaction."""
from pathlib import Path
import subprocess
import tempfile
import unittest
from .test_hybrid_button import function

ROOT = Path(__file__).resolve().parents[2]


class ShutdownSpiTest(unittest.TestCase):
    def test_mit_full_poll_and_guarded_followup(self):
        source = (ROOT/'keyboards/hhkb/modules/hybrid/hybrid.c').read_text()
        harness = r'''
#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <assert.h>
#define SPI_SR_OVR 1
#define SPI_SR_RXNE 2
#define SPI_SR_TXE 4
#define SPI_CR2_TXEIE 8
#define pdMS_TO_TICKS(x) (x)
static struct {uint32_t SR,CR2;} spi;
#define SPI1 (&spi)
static volatile uint8_t data;
static volatile uint8_t *const spi_data=&data;
static uint8_t report[8],keyboard_queue[8][7],reply[9];
static uint8_t queue_count,queue_read,control_command,control_slot,reply_length,reply_position;
static uint8_t notification_type,active_slot,battery_pending,battery_reported,display_slot,attention_retries;
static bool notification_value,pairing,pairing_active,battery_polled,hello_seen,control_polled;
static uint32_t profile_display_until;
static uint32_t xTaskGetTickCountFromISR(void){return 0;}
'''
        harness += function(source,'static void start_reply(')
        harness += function(source,'void SPI1_IRQHandler(')
        harness += r'''
static void transact(uint8_t command, const uint8_t *expected, unsigned size) {
 uint8_t wire[10];
 wire[0]=data;data=command;spi.SR=SPI_SR_RXNE;SPI1_IRQHandler();
 for(unsigned i=1;i<size;i++){wire[i]=data;spi.SR=SPI_SR_TXE;SPI1_IRQHandler();}
 assert(memcmp(wire,expected,size)==0);
 assert(reply_length==0 && !(spi.CR2 & SPI_CR2_TXEIE));
}
int main(void){
 const uint8_t poll[]={0,0xaa,0x40,0,0,0,0,0,0,0},idle[]={0,0xaa,0x10,0};
 control_command=4;queue_count=1;battery_pending=16;
 data=3;spi.SR=SPI_SR_RXNE;SPI1_IRQHandler();
 assert(control_command==4 && !control_polled && !reply_length);
 data=0;
 transact(2,poll,sizeof(poll));
 assert(control_command==4 && queue_count==1 && battery_pending==16);
 assert(control_polled);
 transact(3,idle,sizeof(idle));assert(control_command==0 && !control_polled);
}
'''
        with tempfile.TemporaryDirectory() as tmp:
            c=Path(tmp)/'spi.c';binary=Path(tmp)/'spi'
            c.write_text(harness)
            subprocess.run(['cc','-std=c11','-Wall','-Wextra','-Werror',str(c),'-o',str(binary)],check=True)
            subprocess.run([str(binary)],check=True)


if __name__ == '__main__':
    unittest.main()
