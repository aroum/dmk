/* Integration tests: production layer and One-Shot state machines, no MCU. */
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include "FreeRTOS.h"
#include "keys.h"
#include "layers.h"
#include "oneshot.h"

TickType_t g_mock_ticks;
const uint32_t keymap[3][6] = {{K_A}, {K_B}, {K_C}};
const size_t keymap_layers = 3;
static unsigned mod_refs[8];
void macros_run_layer_triggers(uint8_t l) {(void)l;}
void hooks_dispatch_layer_change(uint8_t l) {(void)l;}
void keyboard_send_modifier_mask(uint8_t mask, bool down) {
    for(unsigned i=0;i<8;i++) if(mask&(1u<<i)) {
        if(down) ++mod_refs[i]; else {assert(mod_refs[i]);--mod_refs[i];}
    }
}
#include "../dmk_core/features/layers.c"

static void clean(void) {
    for(unsigned i=0;i<8;i++) assert(!mod_refs[i]);
    layers_init();oneshot_init();g_mock_ticks=0;
}
static void layer_sequences(void) {
    clean();
    uint32_t rng=0x12345678;
    unsigned owners[16]={0};uint16_t persistent=1;
    for(unsigned n=0;n<20000;n++) {
        rng=rng*1664525u+1013904223u;
        unsigned l=(rng>>16)&15,op=(rng>>24)%4;
        if(op==0 && owners[l]<100){layers_on(l);++owners[l];}
        else if(op==1){layers_off(l);if(owners[l])--owners[l];}
        else if(op==2){layers_toggle(l);persistent^=1u<<l;}
        else {persistent=(uint16_t)rng;layers_set_state(persistent);}
        unsigned expected=persistent;
        for(unsigned j=0;j<16;j++) if(owners[j])expected|=1u<<j;
        assert(layers_get_latched_state()==persistent);
        assert(layers_get_state()==(expected?expected:1));
        unsigned highest=0;
        for(unsigned j=0;j<16;j++)if(expected&(1u<<j))highest=j;
        assert(layers_get_active()==highest);
    }
    uint16_t before=layers_get_state();
    layers_on(255);layers_off(255);layers_toggle(255);
    assert(layers_get_state()==before);
    assert(keyboard_get_flat_key_index(255,255)==-1);
    assert(layers_lookup_key_on_layer(255,0,0)==0);
    assert(layers_lookup_key(255,255)==0);
}
static void oneshot_lifetimes(void) {
    clean();
    // A physical MO owner and an OSL owner must remain independent on retap.
    layers_on(2);
    assert(oneshot_process_event(OS_LAYER(2),true));
    assert(oneshot_process_event(OS_LAYER(2),false));
    assert(oneshot_process_event(OS_LAYER(2),true));
    g_mock_ticks=1000;
    assert(oneshot_process_event(OS_LAYER(2),false));
    assert(layers_get_state()&4);layers_off(2);assert(layers_get_state()==1);
    // Lazy mods must be acquired once, even for repeated lazy-send requests.
    for(unsigned bit=0;bit<8;bit++) {
        uint32_t os=OS_MOD(1u<<bit);
        assert(oneshot_process_event(os,true));
        oneshot_send_lazy_mods();oneshot_send_lazy_mods();
        assert(mod_refs[bit]==1);
        assert(oneshot_process_event(os,true)); // re-arm balances the old owner
        assert(!mod_refs[bit]);oneshot_send_lazy_mods();assert(mod_refs[bit]==1);
        g_mock_ticks+=1000;assert(oneshot_process_event(os,false));assert(!mod_refs[bit]);
    }
    // Timeout arithmetic across the uint32 tick rollover.
    g_mock_ticks=UINT32_MAX-100;
    assert(oneshot_process_event(OS_MOD(MOD_RSHIFT),true));
    assert(oneshot_process_event(OS_MOD(MOD_RSHIFT),false));
    oneshot_send_lazy_mods();assert(mod_refs[5]==1);
    g_mock_ticks+=1000;assert(oneshot_check_timeouts(g_mock_ticks)==500);
    g_mock_ticks+=500;oneshot_check_timeouts(g_mock_ticks);
    assert(!mod_refs[5] && !oneshot_has_active());
    // Exhaustion: no unowned layer or modifier must be activated.
    for(unsigned i=0;i<MAX_OS_TRACKERS;i++)assert(oneshot_process_event(OS_LAYER(i+1),true));
    assert(oneshot_process_event(OS_LAYER(10),true));assert(!(layers_get_state()&(1u<<10)));
    g_mock_ticks+=1000;
    for(unsigned i=0;i<MAX_OS_TRACKERS;i++)assert(oneshot_process_event(OS_LAYER(i+1),false));
    assert(layers_get_state()==1);
    clean();
}
int main(void) {
    layer_sequences();oneshot_lifetimes();
    puts("runtime ownership: model sequences, OSL retap, lazy refs, rollover and exhaustion passed");
}
