/* Included after the read-only legacy decoder. New writes never use its slots.
 * Physical ranges are candidates from the PFU EEPROM map, not an assertion that
 * every factory revision leaves them free. Unknown contents block enrollment. */
#include "hhkb_compact_storage.h"
#include "hhkb_storage_codec.h"
#include "settings_storage_guard.h"
#ifndef HHKB_STORAGE_LOCK
#include "FreeRTOS.h"
#include "task.h"
#define HHKB_STORAGE_LOCK() vTaskSuspendAll()
#define HHKB_STORAGE_UNLOCK() ((void)xTaskResumeAll())
#endif
#ifndef HHKB_EEPROM_BASE
#define HHKB_EEPROM_BASE 0x08080000UL
#endif
#define C_SLOT_BYTES 880U
#define C_HEADER_BYTES 20U
#define C_CAPACITY (C_SLOT_BYTES - C_HEADER_BYTES)
#define C_MAGIC (0x33434d00U | NUM_KEYS)
#define C_OWNER (0x33484b00U | NUM_KEYS)
#define C_DONE 0x444f4e45U
#define C_RESET 0x52534554U
#define C_CAL_BYTES (sizeof(hhkb_eeprom_cal_t) - 4U * MATRIX_ROWS * MATRIX_COLS + 3U * NUM_KEYS)
#define C_RAW_BYTES (sizeof(dynamic_keymap) + sizeof(vial_storage.data.combos) + sizeof(vial_storage.data.macro_buffer) + C_CAL_BYTES)
_Static_assert(C_SLOT_BYTES == 880, "Update the physical map when changing slot size");
_Static_assert(sizeof(hhkb_eeprom_cal_t) - 4U*MATRIX_ROWS*MATRIX_COLS == 36,
               "Bump the compact format before changing settings layout");
_Static_assert(sizeof(vial_storage.data.combos) == 80 &&
               DYNAMIC_KEYMAP_MACRO_BUFFER_SIZE == (NUM_KEYS == 60 ? 224 : 64),
               "Bump the compact format before changing VIA capacities");
static const uint8_t c_positions[NUM_KEYS][2] = LAYOUT;
static uint32_t c_image[C_SLOT_BYTES / 4];
static hhkb_eeprom_cal_t c_cal;
static bool c_initialized, c_keymap_ready, c_cal_ready, c_pending, c_blocked;
static int c_active = -1, c_target = -1, c_copy = -1;
static unsigned c_stage, c_copy_stage, c_cleanup;
static uint32_t c_generation, c_flags;
static hhkb_pack_state c_pack;
static bool c_packing;
_Static_assert(C_RAW_BYTES <= UINT16_MAX, "Compact codec indices must fit");
static const char *c_status = "Not saved";
#ifndef HHKB_EEPROM_LOG
#if defined(DEBUG_CONSOLE_ENABLE) && !defined(HHKB_EEPROM_WRITE)
extern bool hhkb_get_debug_flag(uint8_t flag);
extern void debug_console_printf(const char *fmt, ...);
#define HHKB_EEPROM_LOG(fmt, ...) do { \
    if (hhkb_get_debug_flag(DEBUG_FLAG_EEPROM)) debug_console_printf("[EEPROM] " fmt "\r\n", ##__VA_ARGS__); \
} while (0)
#else
#define HHKB_EEPROM_LOG(...) ((void)0)
#endif
#endif
static void c_write_failed(const char *where) {
    (void)where;
    if (strcmp(c_status, "Write failed: retrying") != 0) {
        HHKB_EEPROM_LOG("write failed (%s), retrying", where);
    }
    c_status = "Write failed: retrying";
}
static volatile uint32_t *c_phys(unsigned offset) {
    return (volatile uint32_t *)(HHKB_EEPROM_BASE + offset);
}
static volatile uint32_t *c_word(unsigned slot_index, unsigned word) {
    unsigned offset = slot_index * C_SLOT_BYTES + word * 4;
    static const struct { uint16_t offset, size; } ranges[] = {
        {0xda0,608}, {0x480,128}, {0x524,144}, /* first slot, safe over DMK1 */
        {0x5b4,332}, {0xd00,160}, {0x1370,144}, {0x170c,244}
    };
    for (unsigned i = 0; i < sizeof(ranges)/sizeof(ranges[0]); ++i) {
        if (offset < ranges[i].size) return c_phys(ranges[i].offset + offset);
        offset -= ranges[i].size;
    }
    return NULL; /* callers stay within the fixed two-slot logical range */
}
static uint32_t c_hash(const uint32_t *p) {
    uint32_t h = hash_word(2166136261U, p[0]);
    h = hash_word(h, p[1]); h = hash_word(h, p[2]);
    for (unsigned i = 5; i < C_SLOT_BYTES/4; ++i) h = hash_word(h, p[i]);
    return h;
}
static bool c_read(unsigned n) {
    for (unsigned i = 0; i < C_SLOT_BYTES/4; ++i) c_image[i] = *c_word(n,i);
    return c_image[0] == C_MAGIC && c_image[4] == SLOT_COMMIT &&
        c_image[2] > 0 && c_image[2] <= C_CAPACITY && c_image[3] == c_hash(c_image) &&
        hhkb_unpack_stream((uint8_t *)&c_image[5], c_image[2], C_RAW_BYTES, NULL, NULL);
}
static bool c_owner_valid(void) {
    uint32_t f = *c_phys(0x1704);
    uint16_t owned = (uint16_t)f;
    return *c_phys(0x1700) == C_OWNER && (uint16_t)(f >> 16) == (uint16_t)~f &&
        (owned >> 4) <= sizeof(hhkb_eeprom_cal_t)/4 &&
        !((owned & 3U) && (owned & 12U));
}
static size_t c_legacy_cal_size(void) {
    const volatile hhkb_eeprom_cal_t *p = (const volatile void *)c_phys(0x1500);
    return hhkb_settings_owned_size(p->magic, USER_EEPROM_MAGIC,
        p->version == USER_EEPROM_VERSION ? sizeof(*p) : offsetof(hhkb_eeprom_cal_t, curve),
        offsetof(hhkb_eeprom_cal_t, rt), (offsetof(hhkb_eeprom_cal_t, scan_rate)+3U)&~3U,
        offsetof(hhkb_eeprom_cal_t, curve));
}
static uint32_t c_legacy_flags(void) {
    uint32_t f = 0;
    bool a = valid(0), b = valid(1);
    if (a || b) { f |= 1; if (b) f |= 2; }
    else { if (valid_v1(0)) f |= 4; if (valid_v1(1)) f |= 8; }
    size_t bytes = c_legacy_cal_size();
    if (bytes != SIZE_MAX) f |= (uint32_t)(bytes / 4) << 4;
    return f;
}
static bool c_legacy_contains(unsigned offset, uint32_t f) {
    for (unsigned n = 0; n < 2; ++n) {
        if ((f & (1U << n)) && offset >= 0x600 + n*SLOT_WORDS*4 && offset < 0x600+(n+1)*SLOT_WORDS*4) return true;
        if ((f & (4U << n)) && offset >= 0x600 + n*V1_SLOT_WORDS*4 && offset < 0x600+(n+1)*V1_SLOT_WORDS*4) return true;
    }
    return false;
}
static void c_init_once(void) {
    if (c_initialized) return;
    c_initialized = true;
    if (c_owner_valid()) {
        c_flags = *c_phys(0x1704) & 0xffff;
        if (*c_phys(0x1708) == C_RESET) {
            /* A reset tombstone wins over every older committed snapshot. */
            c_status = "Reset pending"; return;
        }
        bool a = c_read(0); uint32_t ga = c_image[1];
        bool b = c_read(1); uint32_t gb = c_image[1];
        if (a || b) {
            c_active = a && b ? ((int32_t)(gb-ga)>0 ? 1:0) : (a?0:1);
            c_read((unsigned)c_active); c_generation = c_image[1]; c_status = "Saved (compressed)";
        }
        return;
    }
    c_flags = c_legacy_flags();
    uint32_t expected_flags = c_flags | ((uint32_t)(uint16_t)~c_flags << 16);
    /* Permit an interrupted enrollment header, never arbitrary nonzero words. */
    if ((*c_phys(0x1700) != 0 && *c_phys(0x1700) != C_OWNER) ||
        (*c_phys(0x1704) != 0 && *c_phys(0x1704) != expected_flags) || *c_phys(0x1708) != 0) c_blocked = true;
    for (unsigned s = 0; s < 2; ++s) for (unsigned w = 0; w < C_SLOT_BYTES/4; ++w) {
        volatile uint32_t *p = c_word(s,w);
        unsigned off = (uintptr_t)p - HHKB_EEPROM_BASE;
        if (*p && !c_legacy_contains(off,c_flags)) c_blocked = true;
    }
    if (c_blocked) c_status = "Occupied: not saved";
}
/* Matrix initialization and VIA initialization can run in different tasks.
 * Keep the one-time ownership/read pass indivisible without masking IRQs. */
static void c_init(void) {
    if(c_initialized) return;
    HHKB_STORAGE_LOCK();
    c_init_once();
    HHKB_STORAGE_UNLOCK();
}
bool hhkb_compact_legacy_allowed(void) {
    c_init();
    return c_active < 0 && (!c_owner_valid() || *c_phys(0x1708) == 0);
}
bool hhkb_compact_writable(void) { c_init(); return !c_blocked; }
const char *hhkb_compact_status(void) { c_init(); return c_status; }
/* Header/tail retain the original settings representation; only actual keys'
 * ADC ranges are packed. JIS holes are reconstructed as zero, then CRC rebuilt. */
uint16_t hhkb_storage_crc16(const uint8_t *p, size_t n) {
    uint16_t crc = 0xffff;
    while (n--) { crc ^= (uint16_t)*p++ << 8; for(unsigned b=0;b<8;++b) crc = (crc&0x8000)?(crc<<1)^0x1021:crc<<1; }
    return crc;
}
static void c_cal_unpack(const uint8_t *p, hhkb_eeprom_cal_t *cal) {
    memset(cal,0,sizeof(*cal)); memcpy(cal,p,8); p+=8;
    for(unsigned i=0;i<NUM_KEYS;++i) {
        unsigned r=c_positions[i][0], c=c_positions[i][1];
        cal->baseline[c][r]=p[0]|((p[1]&15U)<<8);
        cal->peak[c][r]=(p[1]>>4)|(p[2]<<4); p+=3;
    }
    memcpy(&cal->rt,p,sizeof(*cal)-offsetof(hhkb_eeprom_cal_t,rt));
    cal->crc16=hhkb_storage_crc16((uint8_t *)&cal->baseline,sizeof(*cal)-offsetof(hhkb_eeprom_cal_t,baseline));
}
typedef struct { size_t start, size; uint8_t *dest; } c_range;
static void c_extract(size_t i, uint8_t value, void *ctx) {
    c_range *range=ctx;
    if(i>=range->start && i-range->start<range->size) {
        size_t offset=i-range->start;
        if(range->start==0 && range->size==sizeof(dynamic_keymap)) {
            unsigned plane=0;
            while(offset>=KEYMAP_WORDS) { offset-=KEYMAP_WORDS; ++plane; }
            offset=offset*4+plane;
        }
        range->dest[offset]=value;
    }
}
static void c_decode_range(size_t start, size_t size, void *dest) {
    c_range range={start,size,dest};
    hhkb_unpack_stream((uint8_t *)&c_image[5],c_image[2],C_RAW_BYTES,c_extract,&range);
}
bool hhkb_compact_load_cal(hhkb_eeprom_cal_t *cal) {
    c_init(); if(c_active<0) return false;
    /* Startup only: reuse the settings cache as packed scratch, avoiding a
     * second calibration-sized stack allocation in the matrix task. */
    uint8_t *packed=(uint8_t *)&c_cal;
    c_decode_range(C_RAW_BYTES-C_CAL_BYTES,C_CAL_BYTES,packed);
    c_cal_unpack(packed,cal); c_cal=*cal; c_cal_ready=true; return true;
}
static bool c_cal_equal(const hhkb_eeprom_cal_t *cal) {
    /* CRC and empty cells are reconstructed, not calibration content. */
    if(memcmp(&c_cal,cal,6) || memcmp(&c_cal.rt,&cal->rt,
        sizeof(*cal)-offsetof(hhkb_eeprom_cal_t,rt))) return false;
    for(unsigned i=0;i<NUM_KEYS;++i) {
        unsigned r=c_positions[i][0], c=c_positions[i][1];
        if(c_cal.baseline[c][r]!=cal->baseline[c][r] ||
           c_cal.peak[c][r]!=cal->peak[c][r]) return false;
    }
    return true;
}
void hhkb_compact_save_cal(const hhkb_eeprom_cal_t *cal) {
    HHKB_STORAGE_LOCK();
    if(!c_cal_ready || !c_cal_equal(cal)) {
        c_cal=*cal; c_cal_ready=true; c_packing=false; c_pending=true;
        if(!c_blocked) c_status="Pending";
    }
    HHKB_STORAGE_UNLOCK();
}
/* Called at the mutation, not only after VIA's deferred save debounce. */
void vial_keyboard_storage_changed(void) {
    if(c_packing) { c_packing=false; c_pending=true; }
}
void vial_keyboard_keymap_load(void) {
    c_init();
    if(c_active>=0) {
        c_decode_range(0,sizeof(dynamic_keymap),dynamic_keymap);
        c_decode_range(sizeof(dynamic_keymap),sizeof(vial_storage.data.combos),vial_storage.data.combos);
        c_decode_range(sizeof(dynamic_keymap)+sizeof(vial_storage.data.combos),sizeof(vial_storage.data.macro_buffer),vial_storage.data.macro_buffer);
    } else if(hhkb_compact_legacy_allowed()) legacy_keymap_load();
    c_keymap_ready=true; if(c_active<0) c_pending=true;
}
void vial_keyboard_keymap_save(void) { c_packing=false; c_pending=true; if(!c_blocked) c_status="Pending"; }
static uint8_t c_input(size_t i, void *ctx) {
    (void)ctx;
    if(i<sizeof(dynamic_keymap)) {
        unsigned plane=0;
        while(i>=KEYMAP_WORDS) { i-=KEYMAP_WORDS; ++plane; }
        return ((uint8_t *)dynamic_keymap)[i*4+plane];
    }
    i-=sizeof(dynamic_keymap);
    if(i<sizeof(vial_storage.data.combos)) return ((uint8_t *)vial_storage.data.combos)[i];
    i-=sizeof(vial_storage.data.combos);
    if(i<sizeof(vial_storage.data.macro_buffer)) return vial_storage.data.macro_buffer[i];
    i-=sizeof(vial_storage.data.macro_buffer);
    if(i<8) return ((uint8_t *)&c_cal)[i];
    i-=8;
    if(i<3*NUM_KEYS) {
        unsigned key=(i*0xaaabU)>>17, byte=i-3*key;
        unsigned r=c_positions[key][0], c=c_positions[key][1];
        unsigned b=c_cal.baseline[c][r], k=c_cal.peak[c][r];
        return byte==0 ? b : byte==1 ? (b>>8)|(k<<4) : k>>4;
    }
    return ((uint8_t *)&c_cal.rt)[i-3*NUM_KEYS];
}
static bool c_build(void) {
    if(!c_packing) {
        for(unsigned i=0;i<NUM_KEYS;++i) {
            unsigned r=c_positions[i][0], c=c_positions[i][1];
            if(c_cal.baseline[c][r]>4095 || c_cal.peak[c][r]>4095) {
                c_status="Invalid ADC: not saved";
                HHKB_EEPROM_LOG("save failed: invalid ADC");
                return false;
            }
        }
        memset(c_image,0,sizeof(c_image)); memset(&c_pack,0,sizeof(c_pack));
        c_packing=true;
    }
    /* No task can change calibration while this bounded slice reads it.
     * IRQs stay enabled; USB gets control back after at most 512 reads. */
    HHKB_STORAGE_LOCK();
    int result=hhkb_pack_step(&c_pack,c_input,NULL,C_RAW_BYTES,
                            (uint8_t *)&c_image[5],C_CAPACITY,512);
    HHKB_STORAGE_UNLOCK();
    if(!result) return false;
    c_packing=false;
    size_t size=result>0 ? c_pack.o : 0;
    if(!size) {
        c_status="Full: not saved";
        HHKB_EEPROM_LOG("save failed: full (%u B raw)", (unsigned)C_RAW_BYTES);
        return false;
    }
    if(c_active>=0 && *c_word((unsigned)c_active,2)==size) {
        bool equal=true;
        for(unsigned i=5;i<C_SLOT_BYTES/4;++i) if(c_image[i]!=*c_word((unsigned)c_active,i)) { equal=false; break; }
        if(equal) {
            c_status="Saved (compressed)";
            HHKB_EEPROM_LOG("unchanged (skipped)");
            return false;
        }
    }
    c_image[0]=C_MAGIC; c_image[1]=c_generation+1; c_image[2]=size;
    c_image[3]=c_hash(c_image); c_image[4]=SLOT_COMMIT;
    return true;
}
/* Retire only the old DMK bytes overlapping stock tables, after the first
 * compact snapshot is committed. Keep our new fragmented storage intact. */
static bool c_cleanup_poll(void) {
    if(*c_phys(0x1708)!=0) return true;
    while(c_cleanup<0x1200/4) {
        unsigned off=0x600+4*c_cleanup++;
        bool owned=(off>=0x700 && off<0xd00 && c_legacy_contains(off,c_flags)) ||
            (off>=0x1500 && off<0x1500+4*((c_flags>>4)&255));
        if(owned) {
            if(!write_word(c_phys(off),0)) { --c_cleanup; c_write_failed("cleanup"); }
            return false;
        }
    }
    if(!write_word(c_phys(0x1708),C_DONE)) { c_write_failed("cleanup done"); return false; }
    if(!c_pending && c_target<0) c_status="Saved (compressed)";
    return true;
}
bool hhkb_compact_reset(void) {
    c_init(); if(c_blocked) { HHKB_EEPROM_LOG("reset blocked"); return false; }
    if(!c_owner_valid()) {
        /* Preserve unknown factory data and let normal enrollment handle blank EEPROM. */
        if(c_flags) { HHKB_EEPROM_LOG("reset blocked: foreign data"); return false; }
        c_pending=c_packing=false;
        return true;
    }
    if(!write_word(c_phys(0x1708),C_RESET)) { HHKB_EEPROM_LOG("reset failed: marker"); return false; }
    if(!write_word(c_word(0,4),0) || !write_word(c_word(1,4),0)) { HHKB_EEPROM_LOG("reset failed: slot clear"); return false; }
    c_active=c_target=-1; c_pending=c_packing=false; c_status="Defaults (compressed)";
    HHKB_EEPROM_LOG("reset to defaults");
    return true; /* tombstone remains until the first default snapshot commits */
}
bool vial_keyboard_keymap_reset(void) {
    // Commit the reset marker before the SW5+SW6 physical EEPROM cleanup.
    // A power cut during cleanup must not resurrect an older snapshot.
    return hhkb_compact_reset() && hhkb_storage_erase_early();
}
bool hhkb_compact_busy(void) {
    if (c_blocked || !c_keymap_ready || !c_cal_ready) return false;
    return c_pending || c_packing || c_target >= 0 ||
        (c_active >= 0 && (*c_phys(0x1708) == 0 || *c_phys(0x1708) == C_RESET));
}

void vial_keyboard_keymap_poll(void) {
    c_init();
    if(c_blocked || !c_keymap_ready || !c_cal_ready) return;
    if(c_active>=0 && *c_phys(0x1708)==C_RESET) {
        if(!write_word(c_phys(0x1708),0)) c_write_failed("reset clear");
        c_cleanup=0; return;
    }
    if(c_active>=0 && !c_cleanup_poll()) return;
    if(c_target<0) {
        if(!c_pending && !c_packing) return;
        if(c_active<0 && c_owner_valid() && *c_phys(0x1708)==C_RESET) {
            /* Complete a reset interrupted before either invalidation. Never
             * let a high-generation pre-reset slot become visible again. */
            if(!write_word(c_word(0,4),0) || !write_word(c_word(1,4),0)) {
                c_write_failed("reset invalidate"); return;
            }
        }
        c_pending=false;
        if(!c_build()) return;
        /* Before overwriting the old second slot, commit its newest contents in
         * legacy slot zero. The source and destination do not overlap (DMK2). */
        if(!c_owner_valid()) {
            if(valid(1) && (!valid(0) || (int32_t)(slot(1)[1]-slot(0)[1])>0)) c_copy=1;
        }
        c_target=c_active<0?0:1-c_active; c_stage=0; c_status="Saving";
        HHKB_EEPROM_LOG("saving slot=%d gen=%lu (%lu B)", c_target, (unsigned long)c_image[1], (unsigned long)c_image[2]);
    }
    if(c_copy>=0) {
        unsigned off; uint32_t value;
        if(c_copy_stage==0) { off=3; value=0; }
        else if(c_copy_stage<=SLOT_WORDS) { off=c_copy_stage-1; value=off==3?0:slot(1)[off]; }
        else { off=3; value=SLOT_COMMIT; }
        if(!write_word((volatile uint32_t *)slot(0)+off,value)) { c_write_failed("legacy copy"); return; }
        if(++c_copy_stage>SLOT_WORDS+1) { c_copy=-1; c_flags|=1; }
        return;
    }
    if(!c_owner_valid()) {
        uint32_t flags=c_flags|((uint32_t)(uint16_t)~c_flags<<16);
        if(!write_word(c_phys(0x1704),flags) || !write_word(c_phys(0x1700),C_OWNER)) { c_write_failed("owner"); return; }
    }
    unsigned off;
    uint32_t value;
    /* Commit word is written last, never with the header. */
    if(c_stage==0) { off=4; value=0; }
    else if(c_stage==C_SLOT_BYTES/4+1) { off=4; value=SLOT_COMMIT; }
    else { off=c_stage-1; value=off==4?0:c_image[off]; }
    if(!write_word(c_word((unsigned)c_target,off),value)) { c_write_failed("slot"); return; }
    if(++c_stage==C_SLOT_BYTES/4+2) {
        c_active=c_target; c_target=-1; c_generation=c_image[1]; c_status=c_pending?"Pending":"Saved (compressed)";
        HHKB_EEPROM_LOG("saved slot=%d gen=%lu", c_active, (unsigned long)c_generation);
    }
}

/* Read ownership before erasing overlapping legacy/compact windows. Never use
 * the current build's matrix geometry to decode a different model's record. */
bool hhkb_storage_erase_early(void) {
    static const struct { uint16_t offset, words; uint32_t magic; } old[] = {
        {0x600,244,0x314b4d44U}, {0x9d0,244,0x314b4d44U}, /* ANSI DMK1 */
        {0x600,320,0x324b4d44U}, {0xb00,320,0x324b4d44U}, /* ANSI DMK2 */
        {0x600,316,0x324a4d44U}, {0xaf0,316,0x324a4d44U}, /* JIS DMK2 */
    };
    unsigned mask = 0;
    for (unsigned n = 0; n < sizeof(old)/sizeof(old[0]); ++n) {
        volatile const uint32_t *p = c_phys(old[n].offset);
        if (legacy_valid(p, old[n].magic, old[n].words-4)) mask |= 1U << n;
    }
    uint32_t owner = *c_phys(0x1700), flags = *c_phys(0x1704);
    bool compact = (owner == 0x33484b3cU || owner == 0x33484b45U) &&
        (uint16_t)(flags >> 16) == (uint16_t)~flags &&
        ((flags & 0xffffU) >> 4) <= (owner == 0x33484b3cU ? 69U : 84U) &&
        !((flags & 3U) && (flags & 12U));
    // Retry the final ownership-word clear after a power cut. This proof
    // accepts no nonzero payload without the normal valid ownership header.
    if (!compact && (owner == 0x33484b3cU || owner == 0x33484b45U) &&
        flags == 0 && *c_phys(0x1708) == 0) {
        compact = true;
        for (unsigned w = 0; w < 2*C_SLOT_BYTES/4; ++w)
            if (*c_word(0,w)) compact = false;
    }
    volatile uint32_t *cal = c_phys(0x1500);
    uint32_t magic = cal[0];
    unsigned cal_words = 0;
    if (magic >= 0x484b4341U && magic <= 0x484b4344U) {
        static const uint8_t sizes[] = {62,65,66,66};
        cal_words = sizes[magic - 0x484b4341U];
    } else if (magic == 0x484b4345U || magic == 0x484b4a45U) {
        unsigned version = cal[1] & 255U;
        unsigned words = magic == 0x484b4345U ? 66U : 81U;
        if (version == 2) words += 3;
        if ((version == 1 || version == 2) && (cal[1] >> 16) ==
            hhkb_storage_crc16((const uint8_t *)&cal[2], words*4-8)) cal_words = words;
    }
    /* Valid CRCs are required for old snapshots: a stale DMK header beside
     * subsequently restored PFU tables is not ownership of those PFU bytes. */
    for (unsigned n = 0; n < sizeof(old)/sizeof(old[0]); ++n)
        if ((mask & (1U << n)) && !hhkb_eeprom_clear_words(c_phys(old[n].offset), old[n].words)) return false;
    if (cal_words && !hhkb_eeprom_clear_words(cal, cal_words)) return false;
    if (compact) {
        for (unsigned w = 0; w < 2*C_SLOT_BYTES/4; ++w)
            if (!write_word(c_word(0,w),0)) return false;
        // Ownership magic is removed last, after all fragmented payload bytes.
        if (!hhkb_eeprom_clear_words(c_phys(0x1700),3)) return false;
    }
    return true;
}
