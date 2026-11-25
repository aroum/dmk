"""Exercise the HHKB hook with real One-Shot and Hold-Tap implementations."""
import json
import subprocess
import tempfile
import unittest
from pathlib import Path
from .test_hybrid_button import ROOT, function


class GraveEscapeTest(unittest.TestCase):
    def test_modifiers_overrides_and_release_order(self):
        module=(ROOT/'keyboards/hhkb/modules/grave_escape/grave_escape.c').read_text()
        usb=(ROOT/'dmk_core/drivers/usb.c').read_text()
        code=r'''
#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include "FreeRTOS.h"
#include "keys.h"
#include "oneshot.h"
#include "hold_tap.h"
#define KEY_CONSUMER_FLAG 0x8000u
#define HHKB_GESC 0x7c16u
TickType_t g_mock_ticks;
static uint8_t s_real_mods,s_weak_mods,overrides;
static uint16_t held,selected,last_down,last_up;
static unsigned downs,ups;
static uint8_t at_press;
static uint8_t hhkb_gesc_overrides(void){return overrides;}
void layers_on(uint8_t l){(void)l;}
void layers_off(uint8_t l){(void)l;}
void layers_toggle(uint8_t l){(void)l;}
void keyboard_send_modifier_mask(uint8_t m,bool p){if(p)s_real_mods|=m;else s_real_mods&=~m;}
void keyboard_send_modifiers(uint8_t m,bool p){keyboard_send_modifier_mask(m,p);}
void keyboard_send_key(uint16_t key,bool pressed);
''' + 'static uint8_t s_real_refs[8],s_weak_refs[8];\n' + function(usb,'static void update_mods(') + function(usb,'uint8_t usb_get_effective_modifiers(') + r'''
static int current_report;
static bool sync_report_modifiers(void){return true;}
static bool add_key_to_report(uint8_t k){(void)k;return false;}
static bool del_key_from_report(uint8_t k){(void)k;return false;}
static bool USB_HID_SendReport(const int *r){(void)r;return true;}
static void USB_HID_SendConsumerReport(uint16_t usage){(void)usage;}
''' + function(usb,'void usb_process_key(') + function(usb,'void usb_process_modifiers(') + function(usb,'void usb_process_key_with_modifiers(') + r'''
void keyboard_send_key(uint16_t key,bool pressed){
 assert(key==K_ESC || key==K_GRAV);
 if(pressed){last_down=key;++downs;at_press=usb_get_effective_modifiers();}
 else{last_up=key;++ups;}
}
''' + function(module,'static bool grave_escape_process(') + r'''
static void press(void){oneshot_on_key_press(HHKB_GESC);assert(grave_escape_process(HHKB_GESC,true));}
static void release(void){assert(grave_escape_process(HHKB_GESC,false));oneshot_on_key_release();if(!held)assert(last_up==last_down);}
static uint16_t expected(uint8_t mods,unsigned flags){
 unsigned groups[4]={0x44,0x11,0x88,0x22};
 for(unsigned i=0;i<4;i++)if((flags&(1u<<i)) && (mods&groups[i]))return K_ESC;
 return mods ? K_GRAV : K_ESC;
}
int main(void){
 // Every left/right modifier combination and override mask, real and weak sources.
 for(unsigned flags=0;flags<16;flags++)for(unsigned mods=0;mods<256;mods++){
  overrides=flags;s_real_mods=mods&0x55;s_weak_mods=mods&0xaa;
  press();assert(last_down==expected(mods,flags) && at_press==mods);
  s_real_mods=s_weak_mods=0;release();
 }
 overrides=0;
 usb_process_modifiers(MOD_RSHIFT,true);usb_process_modifiers(MOD_RSHIFT,true);
 usb_process_modifiers(MOD_RSHIFT,false);assert(usb_get_effective_modifiers()==MOD_RSHIFT);
 usb_process_modifiers(MOD_RSHIFT,false);assert(!usb_get_effective_modifiers());
 usb_process_key_with_modifiers(MOD_RALT,0,true);usb_process_key_with_modifiers(MOD_RALT,0,true);
 usb_process_key_with_modifiers(MOD_RALT,0,false);assert(usb_get_effective_modifiers()==MOD_RALT);
 usb_process_key_with_modifiers(MOD_RALT,0,false);assert(!usb_get_effective_modifiers());
 assert(OS(MOD_RCTRL)==OS_MOD(MOD_RCTRL));
 // Exercise the actual USB physical and MK weak-modifier update paths.
 for(unsigned bit=0;bit<8;bit++){
  s_real_mods=s_weak_mods=0;
  // Physical KC_LCTL..KC_RGUI use HID usages, not modifier masks.
  usb_process_key(0xe0u+bit,true);
  assert(usb_get_effective_modifiers()==(1u<<bit));
  press();assert(last_down==expected(1u<<bit,0) && at_press==(1u<<bit));
  release();usb_process_key(0xe0u+bit,false);
  assert(!usb_get_effective_modifiers());
  usb_process_modifiers(1u<<bit,true);press();assert(last_down==expected(1u<<bit,0));
  usb_process_modifiers(1u<<bit,false);release();
  usb_process_key_with_modifiers(1u<<bit,0,true);press();assert(last_down==expected(1u<<bit,0));
  usb_process_key_with_modifiers(1u<<bit,0,false);release();
 }
 // All One-Shot masks, including masks 0xe0..0xe7 which are not HID keycodes.
 for(unsigned mods=1;mods<256;mods++){
  oneshot_init();s_real_mods=s_weak_mods=0;
  uint32_t os=OS_MOD(mods);
  assert((os&0x10000) && ((os>>8)&255)==mods);
  assert(oneshot_process_event(os,true));assert(oneshot_process_event(os,false));
  press();assert(last_down==expected(mods,0) && at_press==mods);
  release();assert(!usb_get_effective_modifiers() && !oneshot_has_active());
 }
 // Modifier masks supplied by HT must be visible too, on both sides.
 for(unsigned bit=0;bit<8;bit++){
  hold_tap_init();s_real_mods=s_weak_mods=0;
  uint32_t ht=HT(MK(1u<<bit,K_NULL),K_A);
  assert(hold_tap_process_event(0,0,ht,true));
  g_mock_ticks+=1000;hold_tap_check_timeouts(g_mock_ticks);
  assert(s_real_mods==(1u<<bit));press();assert(last_down==expected(1u<<bit,0));
  assert(hold_tap_process_event(0,0,ht,false));release();
 }
 // Changing settings while held must not change the key released.
 overrides=0;s_real_mods=MOD_RSHIFT;s_weak_mods=0;press();assert(last_down==K_GRAV);
 overrides=8;s_real_mods=0;release();assert(last_up==K_GRAV);
 s_real_mods=MOD_RSHIFT;press();assert(last_down==K_ESC);
 overrides=0;release();assert(last_up==K_ESC);s_real_mods=0;
 // A second binding must not prematurely release the shared held action.
 unsigned before=ups;s_real_mods=0;press();s_real_mods=0x22;press();
 assert(selected==K_ESC);release();assert(ups==before);release();assert(ups==before+1);
 assert(!grave_escape_process(K_A,true));
 assert(grave_escape_process(HHKB_GESC,false));assert(ups==before+1);
 assert(downs==ups);
}
'''
        with tempfile.TemporaryDirectory() as tmp:
            src=Path(tmp)/'gesc.c';src.write_text(code);out=Path(tmp)/'gesc'
            subprocess.run(['clang','-std=c11','-Wall','-Wextra','-Werror','-DHHKB_GRAVE_ESCAPE=1',
                '-I',str(ROOT/'tests/mocks'),'-I',str(ROOT/'dmk_core/include'),
                '-I',str(ROOT/'dmk_core/features'),'-I',str(ROOT/'dmk_core/config'),
                str(src),str(ROOT/'dmk_core/features/oneshot.c'),str(ROOT/'dmk_core/features/hold_tap.c'),
                '-o',str(out)],check=True)
            subprocess.run([str(out)],check=True)

    def test_via_menu_and_eeprom_nibble(self):
        definition=json.loads((ROOT/'keyboards/hhkb/via.json').read_text())
        tab=next(x for x in definition['menus'][0]['content'] if x['label']=='Grave Escape')
        self.assertEqual([x['content'][2] for x in tab['content']],list(range(57,61)))
        self.assertTrue(all(x['type']=='toggle' for x in tab['content']))

    def test_via_override_commands(self):
        source=(ROOT/'keyboards/hhkb/modules/via_settings/hhkb_via_controls.c').read_text()
        settings=(ROOT/'keyboards/hhkb/modules/ec/ec_settings.c').read_text()
        code=r'''
#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
static uint8_t s_gesc_overrides;
static unsigned dirty;
static void hhkb_settings_mark_dirty(void){++dirty;}
''' + function(settings,'uint8_t hhkb_gesc_overrides(') + function(settings,'void hhkb_gesc_set_override(') + r'''
static bool command(uint8_t op,uint8_t id,uint8_t value,uint8_t *response){
 uint8_t request[32]={op,0,id,value};
''' + function(source,'if (id >= 57 && id <= 60)') + r'''
 return false;
}
int main(void){
 uint8_t response[32]={0};
 for(unsigned id=57;id<=60;id++){
  assert(command(7,id,1,response));assert(command(8,id,0,response));assert(response[3]==1);
  unsigned before=dirty;assert(command(7,id,1,response));assert(dirty==before);
  assert(!command(7,id,2,response));assert(command(8,id,0,response));assert(response[3]==1);
  assert(command(7,id,0,response));assert(command(8,id,0,response));assert(response[3]==0);
 }
 assert(!s_gesc_overrides && dirty==8);
 hhkb_gesc_set_override(4,true);assert(dirty==8 && !s_gesc_overrides);
}
'''
        with tempfile.TemporaryDirectory() as tmp:
            src=Path(tmp)/'commands.c';src.write_text(code);out=Path(tmp)/'commands'
            subprocess.run(['clang','-std=c11','-Wall','-Wextra','-Werror',str(src),'-o',str(out)],check=True)
            subprocess.run([str(out)],check=True)
