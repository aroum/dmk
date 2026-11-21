"""Keep the physical tester live while muting HHKB calibration key events."""
from pathlib import Path
import subprocess
import tempfile
import unittest
from .test_hybrid_button import ROOT, function


class CalibrationInputTest(unittest.TestCase):
    def test_calibration_releases_keys_and_finish_restores_publication(self):
        source = (ROOT / 'keyboards/hhkb/modules/ec/matrix.c').read_text()
        start = source.index('        bool calibrating = hhkb_is_calibrating();')
        end = source.index('\n#ifdef HHKB_SCAN_DIAGNOSTICS\n        if (!first_event)', start)
        publish = source[start:end]
        code = r'''
#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#define MATRIX_ROWS ROWS
#define MATRIX_COLS 15
#define MATRIX_EVENT_ROW_SNAPSHOT 0x80
#define MATRIX_EVENT_SCAN_START 0x40
#define pdPASS 1
#define HHKB_HYBRID 1
#define xTaskGetTickCount() 123u
typedef uint16_t matrix_row_t;
typedef struct {uint8_t row,col,row_state_hi;} matrix_event_t;
static matrix_row_t s_matrix_state[MATRIX_ROWS],published_state[MATRIX_ROWS];
static bool calibration;
static unsigned events,last_activity;
static int matrix_queue;
static matrix_event_t received[20];
static bool hhkb_is_calibrating(void){return calibration;}
static int xQueueSend(int q,const matrix_event_t *e,unsigned wait){
 (void)q;(void)wait;received[events++]=*e;return pdPASS;
}
''' + function(source, 'bool matrix_is_pressed(') + '\nstatic void publish(void){\n' + publish + r'''
}
int main(void){
 s_matrix_state[0]=1;s_matrix_state[MATRIX_ROWS-1]=1u<<14;
 publish();assert(events==2 && published_state[0]==1);
 calibration=true;events=0;publish();
 assert(events==2 && received[0].col==0 && received[1].row_state_hi==0);
 assert(published_state[0]==0 && published_state[MATRIX_ROWS-1]==0);
 assert(matrix_is_pressed(0,0) && matrix_is_pressed(MATRIX_ROWS-1,14));
 s_matrix_state[1]=1u<<3;events=0;publish();
 assert(events==0 && matrix_is_pressed(1,3));
 calibration=false;publish();assert(events==3);
 assert(published_state[0]==1 && published_state[1]==8);
 memset(s_matrix_state,0,sizeof(s_matrix_state));events=0;publish();
 assert(events==3 && !matrix_is_pressed(0,0));
 calibration=true;events=0;publish();assert(events==0);
 calibration=false;publish();assert(events==0);
}
'''
        with tempfile.TemporaryDirectory() as tmp:
            c = Path(tmp) / 'calibration.c'; c.write_text(code)
            for rows in (4,5):
                exe = Path(tmp) / f'calibration-{rows}'
                subprocess.run(['cc','-std=c11','-Wall','-Wextra','-Werror',f'-DROWS={rows}',
                                str(c),'-o',str(exe)],check=True)
                subprocess.run([str(exe)],check=True)
