"""Keep shipped configs explicit while preserving legacy HT source tests."""
from pathlib import Path
import re
import unittest
from .test_hybrid_button import ROOT

class ExplicitLayerConfigTest(unittest.TestCase):
    def test_no_bare_numeric_hold_layers(self):
        for config in (ROOT/'keyboards').rglob('config.h'):
            self.assertIsNone(re.search(r'\bHT(?:_T)?\(\s*(?:0x[0-9a-fA-F]+|[0-9]+)\s*,',config.read_text()),str(config))

    def test_flat_press_cache_keeps_original_key(self):
        import subprocess
        import tempfile
        from .test_hybrid_button import function
        source=(ROOT/'dmk_core/core/keyboard.c').read_text()
        code=r'''
#include <assert.h>
#include <stdint.h>
#define NUM_KEYS 2
static uint32_t s_active_keys[NUM_KEYS];
static int16_t keyboard_get_flat_key_index(uint8_t r,uint8_t c){
 if(r==0 && c==0)return 0;
 if(r==2 && c==3)return 1;
 return -1;
}
static uint32_t layers_lookup_key(uint8_t r,uint8_t c){(void)r;(void)c;return 99;}
''' + function(source,'void keyboard_record_pressed_key(') + function(source,'static uint32_t pop_pressed_key(') + r'''
int main(void){
 keyboard_record_pressed_key(0,0,4);keyboard_record_pressed_key(2,3,5);
 keyboard_record_pressed_key(255,255,6);
 assert(pop_pressed_key(2,3)==5 && pop_pressed_key(0,0)==4);
 assert(pop_pressed_key(2,3)==99 && pop_pressed_key(255,255)==99);
}
'''
        with tempfile.TemporaryDirectory() as tmp:
            c=Path(tmp)/'cache.c';c.write_text(code);out=Path(tmp)/'cache'
            subprocess.run(['clang','-std=c11','-Wall','-Wextra','-Werror',str(c),'-o',str(out)],check=True)
            subprocess.run([str(out)],check=True)
