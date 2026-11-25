#include <assert.h>
#include <stdio.h>
#include <string.h>
#define MATRIX_COLS 2
#define MATRIX_ROWS 3
#include "scan_cache.h"

void reference_rt_key_set_range(rt_key_t *,uint16_t,uint16_t);
bool reference_rt_update_curve(rt_key_t *,const rt_cfg_t *,uint16_t,const uint16_t *);
uint16_t reference_rt_normalize_curve(const rt_key_t *,uint16_t,const uint16_t *);
static uint32_t random_state=19;
static uint32_t random32(void){random_state=random_state*1664525u+1013904223u;return random_state;}
static void equal_state(const rt_key_t *a,const rt_key_t *b,bool extreme){
 assert(a->base==b->base && a->scale==b->scale);
 assert(a->down==b->down && a->latched==b->latched);
 if(extreme)assert(a->ext==b->ext);
}
int main(void){
 uint16_t lut[126];
 for(unsigned trial=0;trial<300;++trial){
  rt_key_t key={0},ref={0},generic={0};
  uint16_t base=random32()%4096,peak=random32()%65536;
  rt_key_set_range(&key,base,peak);reference_rt_key_set_range(&ref,base,peak);generic=key;
  unsigned depth=trial%17==0 ? 100 : 775+random32()%351;
  for(unsigned i=0;i<126;++i)lut[i]=depth*i*i/(125*125);
  const uint16_t *curve=trial%2?lut:NULL;
  rt_cfg_t cfg={.act_pm=random32()%1126,.release_pm=random32()%1126,.reset_pm=random32()%201,
                .press_pm=1+random32()%300};
  rt_scan_cfg_t scan;rt_prepare_scan(&scan,&cfg,curve);
  rt_raw_cache_t cache={0};rt_prepare_raw_cache(&cache,&key,&scan);
  for(unsigned i=0;i<8192;++i){
   uint16_t raw=i<4096?i:8191-i;
   assert(rt_scan_normalize(&key,raw,&scan)==reference_rt_normalize_curve(&ref,raw,curve));
   bool expected=reference_rt_update_curve(&ref,&cfg,raw,curve);
   assert(rt_update_raw_cached(&key,&cache,raw)==expected);equal_state(&key,&ref,false);
   assert(rt_update_curve(&generic,&cfg,raw,curve)==expected);equal_state(&generic,&ref,true);
  }
  // Restoring the lazy classic extreme makes RT transitions bit-for-bit equivalent.
  key.ext=rt_scan_normalize(&key,cache.last_raw,&scan);equal_state(&key,&ref,true);
  cfg.rt_enabled=true;rt_prepare_scan(&scan,&cfg,curve);
  rt_prepare_raw_cache(&cache,&key,&scan);
  for(unsigned i=0;i<1000;++i){
   uint16_t raw=random32()%4096,travel;
   bool expected=reference_rt_update_curve(&ref,&cfg,raw,curve);
   assert(rt_update_scan_cached(&key,&cache,&scan,raw,i%2?&travel:NULL)==expected);
   equal_state(&key,&ref,key.latched);
   if(i%2)assert(travel==reference_rt_normalize_curve(&ref,raw,curve));
  }
  // Keep the public 16-bit API correct beyond the scanner's 12-bit fast path.
  generic=ref;
  for(unsigned i=0;i<100;++i){
   uint16_t raw=random32();
   assert(rt_update_curve(&generic,&cfg,raw,curve)==reference_rt_update_curve(&ref,&cfg,raw,curve));
   equal_state(&generic,&ref,true);
  }
 }
 rt_key_t keys[MATRIX_COLS][MATRIX_ROWS]={0},ref[MATRIX_COLS][MATRIX_ROWS]={0};
 hhkb_scan_cache_t cache={0};uint32_t curve_revision=0;
 for(unsigned pass=0;pass<4000;++pass){
  if(pass%137==0){
   for(unsigned c=0;c<MATRIX_COLS;++c)for(unsigned r=0;r<MATRIX_ROWS;++r){
    uint16_t base=random32()%1000,peak=base+150+random32()%2800;
    rt_key_set_range(&keys[c][r],base,peak);reference_rt_key_set_range(&ref[c][r],base,peak);
    rt_key_reset(&keys[c][r]);rt_key_reset(&ref[c][r]);
   }
  }
  if(pass%211==0){
   ++curve_revision;
   for(unsigned c=0;c<MATRIX_COLS;++c)for(unsigned r=0;r<MATRIX_ROWS;++r){rt_key_reset(&keys[c][r]);rt_key_reset(&ref[c][r]);}
  }
  rt_cfg_t cfg={.rt_enabled=(pass/41)%2,.act_pm=100+((pass/97)*53)%901,
                .release_pm=(pass/73)%400,.reset_pm=50,.press_pm=25};
  rt_scan_cfg_t scan;rt_prepare_scan(&scan,&cfg,(pass/211)%2?lut:NULL);
  // Enabling RT while held beyond peak canonicalizes the former classic anchor.
  if(cache.fast_active && cfg.rt_enabled && cache.range_revision==rt_range_revision && cache.curve_revision==curve_revision)
   for(unsigned c=0;c<MATRIX_COLS;++c)for(unsigned r=0;r<MATRIX_ROWS;++r){
    unsigned bottom=scan.curve?scan.curve[125]:RT_FULL_SCALE;
    if(ref[c][r].ext>(int)bottom)ref[c][r].ext=bottom;
   }
  hhkb_scan_cache_prepare(&cache,keys,&scan,rt_range_revision,curve_revision);
  for(unsigned c=0;c<MATRIX_COLS;++c)for(unsigned r=0;r<MATRIX_ROWS;++r){
   if(cfg.rt_enabled)equal_state(&keys[c][r],&ref[c][r],keys[c][r].latched);
   uint16_t raw=random32()%4096,travel;
   bool expected=reference_rt_update_curve(&ref[c][r],&cfg,raw,scan.curve);
   // Mirror scan_process_pending: cells the incremental sweep has not reached
   // take the uncached reference path; classic fallback maintains last_raw.
   bool actual;
   if(!hhkb_scan_cache_entry_valid(&cache,(uint8_t)c,(uint8_t)r)){
    actual=rt_update_scan_sample(&keys[c][r],&scan,raw,&travel);
    if(!cfg.rt_enabled)cache.key[c][r].last_raw=raw;
   }else if(cfg.rt_enabled)actual=rt_update_scan_cached(&keys[c][r],&cache.key[c][r],&scan,raw,&travel);
   else actual=rt_update_raw_cached(&keys[c][r],&cache.key[c][r],raw);
   assert(actual==expected);equal_state(&keys[c][r],&ref[c][r],cfg.rt_enabled && keys[c][r].latched);
  }
 }
 // A completed incremental sweep must be byte-identical to a direct rebuild,
 // and every partially swept cache may only expose cells matching the current cfg.
 {
  rt_cfg_t cfg={.rt_enabled=true,.act_pm=300,.release_pm=40,.reset_pm=50,.press_pm=25};
  for(unsigned c=0;c<MATRIX_COLS;++c)for(unsigned r=0;r<MATRIX_ROWS;++r){
   rt_key_set_range(&keys[c][r],(uint16_t)(400+37*(c*MATRIX_ROWS+r)),2900);rt_key_reset(&keys[c][r]);
  }
  for(unsigned phase=0;phase<=MATRIX_COLS*MATRIX_ROWS+1;++phase){
   cfg.act_pm=(uint16_t)(300+(phase%2)*50);
   rt_scan_cfg_t scan;rt_prepare_scan(&scan,&cfg,lut);
   for(unsigned k=0;k<=phase;++k)
    hhkb_scan_cache_prepare(&cache,keys,&scan,rt_range_revision,curve_revision);
   for(unsigned c=0;c<MATRIX_COLS;++c)for(unsigned r=0;r<MATRIX_ROWS;++r){
    if(!hhkb_scan_cache_entry_valid(&cache,(uint8_t)c,(uint8_t)r))continue;
    rt_raw_cache_t fresh;fresh.last_raw=cache.key[c][r].last_raw;
    rt_prepare_raw_cache(&fresh,&keys[c][r],&scan);
    assert(fresh.press_raw==cache.key[c][r].press_raw);
    assert(fresh.release_raw==cache.key[c][r].release_raw);
   }
  }
  rt_scan_cfg_t scan;rt_prepare_scan(&scan,&cfg,lut);
  while(cache.rebuild_cursor<MATRIX_COLS*MATRIX_ROWS)
   hhkb_scan_cache_prepare(&cache,keys,&scan,rt_range_revision,curve_revision);
  for(unsigned c=0;c<MATRIX_COLS;++c)for(unsigned r=0;r<MATRIX_ROWS;++r){
   rt_raw_cache_t fresh;fresh.last_raw=cache.key[c][r].last_raw;
   rt_prepare_raw_cache(&fresh,&keys[c][r],&scan);
   assert(fresh.press_raw==cache.key[c][r].press_raw);
   assert(fresh.release_raw==cache.key[c][r].release_raw);
  }
 }
 // Classic->RT ext restore must hold even when the sweep never completes:
 // the classic fallback's last_raw writes feed the restore.
 {
  for(unsigned c=0;c<MATRIX_COLS;++c)for(unsigned r=0;r<MATRIX_ROWS;++r){
   uint16_t base=(uint16_t)(500+29*(c*MATRIX_ROWS+r));
   rt_key_set_range(&keys[c][r],base,(uint16_t)(base+2400));
   reference_rt_key_set_range(&ref[c][r],base,(uint16_t)(base+2400));
   rt_key_reset(&keys[c][r]);rt_key_reset(&ref[c][r]);
  }
  rt_cfg_t cfg={.rt_enabled=false,.act_pm=400,.release_pm=100,.reset_pm=50,.press_pm=25};
  for(unsigned pass=0;pass<40;++pass){
   cfg.act_pm=(uint16_t)(400+pass%2); // invalidate every pass: sweep never completes
   rt_scan_cfg_t scan;rt_prepare_scan(&scan,&cfg,NULL);
   hhkb_scan_cache_prepare(&cache,keys,&scan,rt_range_revision,curve_revision);
   for(unsigned c=0;c<MATRIX_COLS;++c)for(unsigned r=0;r<MATRIX_ROWS;++r){
    uint16_t raw=random32()%4096,travel;
    bool expected=reference_rt_update_curve(&ref[c][r],&cfg,raw,NULL);
    bool actual;
    if(!hhkb_scan_cache_entry_valid(&cache,(uint8_t)c,(uint8_t)r)){
     actual=rt_update_scan_sample(&keys[c][r],&scan,raw,&travel);
     cache.key[c][r].last_raw=raw;
    }else actual=rt_update_raw_cached(&keys[c][r],&cache.key[c][r],raw);
    assert(actual==expected);
   }
  }
  cfg.rt_enabled=true; // same ranges: prepare runs the ext restore from last_raw
  rt_scan_cfg_t scan;rt_prepare_scan(&scan,&cfg,NULL);
  hhkb_scan_cache_prepare(&cache,keys,&scan,rt_range_revision,curve_revision);
  for(unsigned c=0;c<MATRIX_COLS;++c)for(unsigned r=0;r<MATRIX_ROWS;++r){
   uint16_t raw=random32()%4096,travel;
   bool expected=reference_rt_update_curve(&ref[c][r],&cfg,raw,NULL);
   bool actual;
   if(!hhkb_scan_cache_entry_valid(&cache,(uint8_t)c,(uint8_t)r))
    actual=rt_update_scan_sample(&keys[c][r],&scan,raw,&travel);
   else actual=rt_update_scan_cached(&keys[c][r],&cache.key[c][r],&scan,raw,&travel);
   assert(actual==expected);equal_state(&keys[c][r],&ref[c][r],keys[c][r].latched);
  }
 }
 puts("Scan thresholds, prepared RT, LUT rounding and 16-bit compatibility tests passed");
}
