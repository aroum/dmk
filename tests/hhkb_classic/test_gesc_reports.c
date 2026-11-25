/* Link the actual USB report engine and HHKB module, replacing only host I/O. */
#include <assert.h>
#include <string.h>
#include "usb.h"
#include "app_usb_hid.h"
#include "FreeRTOS.h"
#include "hooks.h"
#undef DMK_HOOK_SUBSCRIBER
#define DMK_HOOK_SUBSCRIBER(name, ...) static const dmk_hook_subscriber_t name __attribute__((used))={__VA_ARGS__}
#include "../../keyboards/hhkb/modules/grave_escape/grave_escape.c"
TickType_t g_mock_ticks;
static uint8_t ignore_mode, overrides;
static USB_HID_KeyboardReport_TypeDef sent, reports[16];
static unsigned report_count;
uint8_t hhkb_gesc_ignore(void){return ignore_mode;}
uint8_t hhkb_gesc_overrides(void){return overrides;}
bool USB_HID_Init(void){memset(&sent,0,sizeof(sent));report_count=0;return true;}
bool USB_HID_SendReport(const USB_HID_KeyboardReport_TypeDef *r){
 assert(report_count<16);
 reports[report_count++]=*r;
 sent=*r;return true;
}
bool USB_HID_SendConsumerReport(uint16_t usage){(void)usage;return true;}
void keyboard_send_key(uint16_t key,bool down){usb_process_key(key,down);}
void keyboard_send_modifier_mask(uint8_t mask,bool down){usb_process_modifiers(mask,down);}
void layers_on(uint8_t l){(void)l;}
void layers_off(uint8_t l){(void)l;}
void layers_toggle(uint8_t l){(void)l;}
static void press(void){oneshot_on_key_press(HHKB_GESC);assert(grave_escape_process(HHKB_GESC,true));}
static void release(void){assert(grave_escape_process(HHKB_GESC,false));oneshot_on_tap_key();}
static bool report_contains(const USB_HID_KeyboardReport_TypeDef *r,uint8_t key){
 for(unsigned i=0;i<6;i++)if(r->Keycodes[i]==key)return true;
 return false;
}
static bool contains(uint8_t key){return report_contains(&sent,key);}
int main(void){
 const uint8_t masks[]={0,0x44,0x11,0x88};
 for(unsigned mode=0;mode<4;mode++)for(unsigned mods=0;mods<256;mods++)for(unsigned weak=0;weak<2;weak++){
  usb_init();oneshot_init();ignore_mode=mode;
  if(weak)usb_process_key_with_modifiers(mods,0,true);else usb_process_modifiers(mods,true);
  // Suppressing modifiers and pressing the key must happen in a single HID report.
  report_count=0;press();
  uint8_t out_key=mods?K_GRAV:K_ESC;
  assert(report_count==1);
  assert(report_contains(&reports[0],out_key));
  assert(reports[0].Modifier==(mods&~masks[mode]));
  assert(usb_get_effective_modifiers()==mods);
  release();assert(sent.Modifier==mods && !contains(K_GRAV) && !contains(K_ESC));
  if(weak)usb_process_key_with_modifiers(mods,0,false);else usb_process_modifiers(mods,false);
  assert(sent.Modifier==0 && usb_get_effective_modifiers()==0);
 }
#ifdef MATRIX_REPORT_BATCH
 // A pending batched key flushes with GUI intact before GUI is suppressed with Grave.
 usb_init();oneshot_init();ignore_mode=3;
 usb_process_modifiers(MOD_LGUI,true);
 usb_process_batched_key(K_A,true);
 report_count=0;press();
 assert(report_count==2);
 assert(reports[0].Modifier==MOD_LGUI && report_contains(&reports[0],K_A) && !report_contains(&reports[0],K_GRAV));
 assert(reports[1].Modifier==0 && report_contains(&reports[1],K_A) && report_contains(&reports[1],K_GRAV));
 release();
 usb_process_key(K_A,false);usb_process_modifiers(MOD_LGUI,false);
 assert(sent.Modifier==0 && !contains(K_A) && !contains(K_GRAV));
#endif
 // Release or press the ignored mod while GESC is down; reference counts stay correct.
 usb_init();ignore_mode=3;usb_process_modifiers(MOD_RGUI,true);press();
 usb_process_modifiers(MOD_RGUI,false);usb_process_modifiers(MOD_LGUI,true);
 usb_process_modifiers(MOD_RSHIFT,true);assert(sent.Modifier==MOD_RSHIFT && contains(K_GRAV));
 ignore_mode=0; // Settings changes take effect on the next GESC press.
 assert(sent.Modifier==MOD_RSHIFT);release();assert(sent.Modifier==(MOD_LGUI|MOD_RSHIFT));
 usb_process_modifiers(MOD_LGUI,false);assert(sent.Modifier==MOD_RSHIFT);
 usb_process_modifiers(MOD_RSHIFT,false);assert(sent.Modifier==0);
 // Multiple GESC bindings share suppression until the final release.
 ignore_mode=3;usb_process_modifiers(MOD_RGUI,true);press();press();release();
 assert(sent.Modifier==0 && contains(K_GRAV));release();assert(sent.Modifier==MOD_RGUI);
 usb_process_modifiers(MOD_RGUI,false);
 // One-Shot GUI must be consumed before suppression is removed: no stuck GUI,
 // and no intermediate report drops GUI without K_GRAV.
 oneshot_init();assert(oneshot_process_event(OS_MOD(MOD_RGUI),true));
 assert(oneshot_process_event(OS_MOD(MOD_RGUI),false));
 report_count=0;press();
 assert(report_count>=1);
 for(unsigned i=0;i<report_count;i++){
  if(!(reports[i].Modifier&MOD_RGUI))assert(report_contains(&reports[i],K_GRAV));
 }
 assert(sent.Modifier==0 && contains(K_GRAV));
 release();assert(!sent.Modifier && !usb_get_effective_modifiers() && !oneshot_has_active());
 // Overrides still choose Escape using the unsuppressed modifier state, in one report.
 overrides=4;usb_process_modifiers(MOD_LGUI,true);
 report_count=0;press();
 assert(report_count==1 && report_contains(&reports[0],K_ESC) && !reports[0].Modifier);
 release();assert(sent.Modifier==MOD_LGUI);
}
