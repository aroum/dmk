#include <assert.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include "config.h"
#include "vial.h"
#include "ec_settings.h"
#include "stm32l0xx.h"
flash_t flash;
flash_t *FLASH=&flash;
uint32_t dynamic_keymap[4][NUM_KEYS];
vial_storage_t vial_storage;
static uint32_t memory[1536];
static int budget=-1;
static unsigned writes;
static bool inject_write(volatile uint32_t *p,uint32_t v) {
    assert(p>=memory && p<memory+1536);
    if(budget==0) return false;
    if(budget>0) --budget;
    *p=v; ++writes; return true;
}
#define HHKB_STORAGE_LOCK() ((void)0)
#define HHKB_STORAGE_UNLOCK() ((void)0)
#define HHKB_EEPROM_WRITE inject_write
#define HHKB_EEPROM_BASE ((uintptr_t)memory)
#define SLOT_ADDRESS ((uintptr_t)&memory[0x600/4])
#include "../../keyboards/hhkb/modules/via_settings/hhkb_keymap_storage.c"
static hhkb_eeprom_cal_t cal;
static void defaults(void) {
    memset(dynamic_keymap,0,sizeof(dynamic_keymap));
    memset(&vial_storage,0,sizeof(vial_storage));
    memset(&cal,0,sizeof(cal)); cal.magic=USER_EEPROM_MAGIC; cal.version=USER_EEPROM_VERSION;
    for(unsigned i=0;i<NUM_KEYS;++i) {
        unsigned r=c_positions[i][0],c=c_positions[i][1];
        cal.baseline[c][r]=300+i; cal.peak[c][r]=2800+i;
        dynamic_keymap[0][i]=4+i;
        dynamic_keymap[1][i]=0x10000000+(i%10);
    }
    cal.rt.act_pm=750; cal.led_mode[0]=0xa1;cal.reserved[1]=0xf3;
    cal.crc16=hhkb_storage_crc16((uint8_t *)&cal.baseline,sizeof(cal)-8);
}
static void reboot(void) {
    c_initialized=c_keymap_ready=c_cal_ready=c_pending=c_blocked=false;
    c_active=c_target=c_copy=-1; c_stage=c_copy_stage=c_cleanup=0;
    c_generation=c_flags=0; c_status="Not saved";
    active_slot=target_slot=-1; generation=stage=0; pending=false;
    defaults();
    if(!hhkb_compact_load_cal(&cal) && hhkb_compact_legacy_allowed()) {
        hhkb_eeprom_cal_t *old=(void *)&memory[0x1500/4];
        if(old->magic==USER_EEPROM_MAGIC) cal=*old;
    }
    vial_keyboard_keymap_load(); hhkb_compact_save_cal(&cal);
}
static void poll(void) { for(unsigned i=0;i<2500;++i) vial_keyboard_keymap_poll(); }
static void fresh(void) { memset(memory,0,sizeof(memory));budget=-1;reboot(); }
static void legacy(unsigned n,unsigned gen,unsigned key) {
    uint32_t *p=(void *)slot(n);p[0]=SLOT_MAGIC;p[1]=gen;p[3]=SLOT_COMMIT;
    memcpy(p+4,dynamic_keymap,sizeof(dynamic_keymap));p[4]=key;
    memcpy(p+4+KEYMAP_WORDS,vial_storage.data.combos,sizeof(vial_storage.data.combos));
    memcpy(p+4+KEYMAP_WORDS+COMBO_WORDS,vial_storage.data.macro_buffer,sizeof(vial_storage.data.macro_buffer));
    p[2]=slot_hash(p);
}
int main(int argc, char **argv) {
    if(argc==2) {
        FILE *file=fopen(argv[1],"rb");assert(file);
        assert(fread(memory,1,sizeof(memory),file)==sizeof(memory) && fgetc(file)==EOF);
        fclose(file);
        uint32_t original[1536];memcpy(original,memory,sizeof(memory));
        assert(hhkb_storage_erase_early() && writes==0 && !c_initialized);
        assert(!memcmp(original,memory,sizeof(memory)));
        puts("stock EEPROM backup preserved byte-for-byte");return 0;
    }
    assert(hhkb_storage_crc16((const uint8_t *)"123456789",9)==0x29b1);
    uint8_t input[2000],encoded[2100],decoded[2000];
    uint32_t seed=7;
    for(unsigned mode=0;mode<4;++mode) {
        for(unsigned i=0;i<sizeof(input);++i) { seed^=seed<<13;seed^=seed>>17;seed^=seed<<5;input[i]=mode==0?0:mode==1?i%251:mode==2?(seed&3):seed; }
        size_t n=hhkb_pack(input,sizeof(input),encoded,sizeof(encoded));assert(n);
        assert(hhkb_unpack(encoded,n,decoded,sizeof(decoded)) && !memcmp(input,decoded,sizeof(input)));
        assert(hhkb_pack(input,sizeof(input),encoded,1)==0);
    }
    defaults();
    for(unsigned value=0;value<4096;++value) {
        unsigned r=c_positions[0][0],c=c_positions[0][1];
        c_cal=cal;c_cal.baseline[c][r]=value;c_cal.peak[c][r]=4095-value;
        uint8_t packed_cal[C_CAL_BYTES];
        for(unsigned i=0;i<C_CAL_BYTES;++i) packed_cal[i]=c_input(C_RAW_BYTES-C_CAL_BYTES+i,NULL);
        hhkb_eeprom_cal_t restored;c_cal_unpack(packed_cal,&restored);
        assert(restored.baseline[c][r]==value && restored.peak[c][r]==4095-value);
    }
    uint8_t bad[]={0x80,0};assert(!hhkb_unpack(bad,2,decoded,3));
    uint8_t truncated[]={3,1};assert(!hhkb_unpack(truncated,2,decoded,4));
    fresh();
    memory[0x300/4]=0xaaaa; memory[0x360/4]=0xcafebabe; memory[0x1000/4]=0x12345678;
    memory[0x1400/4]=0x76543210; memory[0x1350/4]=0xfeedface;
    poll();
    assert(memory[0x300/4]==0xaaaa && memory[0x360/4]==0xcafebabe && memory[0x1000/4]==0x12345678);
    assert(memory[0x1400/4]==0x76543210 && memory[0x1350/4]==0xfeedface);
    assert(c_active==0);assert(!strcmp(c_status,"Saved (compressed)"));
    unsigned packed=*c_word(0,2);printf("%u keys: %u/%u compressed bytes\n",NUM_KEYS,packed,(unsigned)C_RAW_BYTES);
    assert(packed<=C_CAPACITY);
    unsigned before=writes;reboot();poll();assert(writes==before); /* no boot wear */
    assert(cal.led_mode[0]==0xa1 && cal.reserved[1]==0xf3);
    for(unsigned i=0;i<NUM_KEYS;++i) assert(cal.baseline[c_positions[i][1]][c_positions[i][0]]==300+i);
    /* Every single write boundary of a replacement snapshot. */
    uint32_t saved[1536];memcpy(saved,memory,sizeof(saved));
    for(unsigned cut=0;cut<240;++cut) {
        memcpy(memory,saved,sizeof(saved));budget=-1;reboot();
        dynamic_keymap[0][0]=0xdeadbeef;cal.rt.act_pm=650;
        vial_storage.data.combos[0].output=42;
        for(unsigned i=0;i<DYNAMIC_KEYMAP_MACRO_BUFFER_SIZE;++i) vial_storage.data.macro_buffer[i]=(i%7)+1;
        hhkb_compact_save_cal(&cal);vial_keyboard_keymap_save();budget=cut;poll();budget=-1;reboot();
        bool newer=dynamic_keymap[0][0]==0xdeadbeef;
        assert(newer || dynamic_keymap[0][0]==4);
        assert(cal.rt.act_pm==(newer?650:750));
        assert(vial_storage.data.combos[0].output==(newer?42:0));
    }
    /* Pending edits during a write are eventually persisted as a new snapshot. */
    memcpy(memory,saved,sizeof(saved));budget=-1;reboot();poll();
    dynamic_keymap[0][0]=77;vial_keyboard_keymap_save();vial_keyboard_keymap_poll();
    dynamic_keymap[0][0]=88;vial_keyboard_keymap_save();poll();reboot();assert(dynamic_keymap[0][0]==88);
    /* Incompressible data must fail explicitly, without destroying the old record. */
    unsigned rng=123;
    for(unsigned l=0;l<4;++l)for(unsigned k=0;k<NUM_KEYS;++k) {rng^=rng<<13;rng^=rng>>17;rng^=rng<<5;dynamic_keymap[l][k]=rng;}
    vial_keyboard_keymap_save();poll();assert(!strcmp(c_status,"Full: not saved"));
    reboot();assert(dynamic_keymap[0][0]==88);
    /* CRC corruption falls back to the previous complete snapshot. */
    poll();int latest=c_active;*c_word(latest,9)^=1;reboot();assert(c_active==1-latest);
    /* Unknown factory bytes and a different layout marker are never overwritten. */
    fresh();memory[0x480/4]=0x11223344;reboot();poll();assert(c_blocked && memory[0x480/4]==0x11223344);
    fresh();memory[0x1700/4]=C_OWNER^1;reboot();poll();assert(c_blocked);
    /* Migrate newest DMK2 slot, and interrupt every EEPROM write, including
     * consolidation, enrollment, compact commit and retirement of old bytes. */
    fresh();cal.rt.act_pm=725;memcpy(&memory[0x1500/4],&cal,sizeof(cal));
    legacy(0,4,41);legacy(1,5,42);
    memcpy(saved,memory,sizeof(saved));
    for(unsigned cut=0;cut<1100;++cut) {
        memcpy(memory,saved,sizeof(saved));budget=-1;reboot();assert(dynamic_keymap[0][0]==42);
        budget=cut;poll();budget=-1;reboot();assert(dynamic_keymap[0][0]==42 && cal.rt.act_pm==725);
        poll();assert(c_active>=0 && *c_phys(0x1708)==C_DONE);
        assert(*c_phys(0x700)==0 && *c_phys(0x1500)==0);
    }
#ifndef HHKB_JIS
    /* DMK1's second slot overlaps neither the first compact slot nor its header. */
    fresh();
    uint32_t *v1=(void *)slot_v1(1);
    v1[0]=SLOT_MAGIC_V1; v1[1]=7; v1[3]=SLOT_COMMIT;
    for(unsigned i=0;i<V1_KEYMAP_WORDS;++i) v1[4+i]=i%65;
    v1[2]=slot_v1_hash(v1);
    uint32_t old_v1[1536]; memcpy(old_v1,memory,sizeof(memory));
    for(unsigned cut=0;cut<500;++cut) {
        memcpy(memory,old_v1,sizeof(memory));budget=-1;reboot();
        budget=cut;poll();budget=-1;reboot();
        assert(dynamic_keymap[3][59]==239%65);poll();assert(c_active>=0);
    }
#endif
    /* Reset is tombstoned before either snapshot is invalidated. Prepare two
     * committed generations so an interrupted reset cannot resurrect slot 1. */
    memcpy(memory,saved,sizeof(saved));budget=-1;reboot();poll();
    dynamic_keymap[0][0]=43;vial_keyboard_keymap_save();poll();
    dynamic_keymap[0][0]=42;vial_keyboard_keymap_save();poll();
    uint32_t reset_base[1536];memcpy(reset_base,memory,sizeof(memory));
    for(unsigned cut=0;cut<4;++cut) {
        memcpy(memory,reset_base,sizeof(memory));budget=-1;reboot();poll();
        budget=cut;hhkb_compact_reset();budget=-1;reboot();
        bool reset_marked=c_owner_valid() && *c_phys(0x1708)==C_RESET;
        if(reset_marked) assert(dynamic_keymap[0][0]==4);
        poll();reboot();if(reset_marked) assert(dynamic_keymap[0][0]==4);
    }
    for(unsigned cut=0;cut<240;++cut) {
        memcpy(memory,reset_base,sizeof(memory));budget=-1;reboot();poll();
        budget=1;hhkb_compact_reset();budget=-1;reboot();
        budget=cut;poll();budget=-1;reboot();poll();reboot();
        assert(dynamic_keymap[0][0]==4);
    }
    /* Boot-time SW5+SW6 is independent of current matrix geometry and never
     * initializes FreeRTOS or the runtime storage state. */
    fresh();poll();
    dynamic_keymap[0][0]=123;vial_keyboard_keymap_save();poll();
    for(unsigned i=0x700/4;i<0xd00/4;++i) memory[i]=0x51515151;
    for(unsigned i=0x1000/4;i<0x1370/4;++i) memory[i]=0x81818181;
    for(unsigned i=0x1400/4;i<0x1700/4;++i) memory[i]=0x41414141;
    for(unsigned i=0x300/4;i<0x480/4;++i) memory[i]=0x31313131;
    uint32_t compact_base[1536];memcpy(compact_base,memory,sizeof(memory));
    bool ours[1536]={0};
    for(unsigned n=0;n<2;++n)for(unsigned w=0;w<C_SLOT_BYTES/4;++w) ours[c_word(n,w)-memory]=true;
    for(unsigned i=0x1700/4;i<0x170c/4;++i) ours[i]=true;
    for(unsigned cut=0;cut<450;++cut) {
        memcpy(memory,compact_base,sizeof(memory));c_initialized=false;
        budget=cut;hhkb_storage_erase_early();budget=-1;
        assert(hhkb_storage_erase_early() && !c_initialized);
        for(unsigned i=0;i<1536;++i) assert(memory[i]==(ours[i]?0:compact_base[i]));
    }
    /* Both old ANSI formats and the JIS format, even when booting the other
     * layout. Factory bytes around each exact record are preserved. */
    static const struct {unsigned off,words;uint32_t magic;} legacy_cases[]={
      {0x600,244,0x314b4d44},{0x9d0,244,0x314b4d44},
      {0x600,320,0x324b4d44},{0xb00,320,0x324b4d44},
      {0x600,316,0x324a4d44},{0xaf0,316,0x324a4d44}};
    for(unsigned n=0;n<6;++n) {
        for(unsigned i=0;i<1536;++i) memory[i]=0x71717171;
        unsigned start=legacy_cases[n].off/4,words=legacy_cases[n].words;
        uint32_t *p=memory+start;
        p[0]=legacy_cases[n].magic;p[1]=13;p[3]=SLOT_COMMIT;p[2]=legacy_hash(p,words-4);
        memcpy(saved,memory,sizeof(memory));
        assert(hhkb_storage_erase_early());
        for(unsigned i=0;i<1536;++i) assert(memory[i]==(i>=start && i<start+words?0:saved[i]));
        // A stale header does not claim PFU bytes whose checksum no longer matches.
        memcpy(memory,saved,sizeof(memory));memory[start+4]^=1;
        memcpy(saved,memory,sizeof(memory));assert(hhkb_storage_erase_early());
        assert(!memcmp(memory,saved,sizeof(memory)));
    }
    for(unsigned kind=0;kind<6;++kind)for(unsigned version=1;version<=2;++version) {
        for(unsigned i=0;i<1536;++i) memory[i]=0x61616161;
        uint32_t *p=memory+0x1500/4;
        static const unsigned old_words[]={62,65,66,66};
        unsigned words=kind<4?old_words[kind]:(kind==4?66:81)+(version==2?3:0);
        p[0]=kind<5?0x484b4341+kind:0x484b4a45;
        if(kind>=4) p[1]=version|((uint32_t)hhkb_storage_crc16((uint8_t *)(p+2),words*4-8)<<16);
        memcpy(saved,memory,sizeof(memory));assert(hhkb_storage_erase_early());
        for(unsigned i=0;i<1536;++i) assert(memory[i]==(i>=0x1500/4 && i<0x1500/4+words?0:saved[i]));
        if(kind>=4) {
            memcpy(memory,saved,sizeof(memory));p[2]^=1;
            memcpy(saved,memory,sizeof(memory));assert(hhkb_storage_erase_early());
            assert(!memcmp(memory,saved,sizeof(memory)));
        }
    }
    puts("compact EEPROM transactions and early recovery reset passed");
}
