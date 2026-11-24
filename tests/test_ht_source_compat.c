#include <stdint.h>
#include "hold_tap_keycode.h"
#include <assert.h>
#include <stdio.h>
enum { FN1=1, LWR=2 };
#define WRAPPED_HT(h,t) HT(h,t)
#define WRAPPED_HT_T(h,t,ms) HT_T(h,t,ms)
#define OLD_LAYER 3
#define MY_A K_A
_Static_assert(HT(1,K_A)==HT(MO(1),K_A),"numeric layer");
_Static_assert(HT(0x4,K_A)==HT(MO(4),K_A),"hex layer");
_Static_assert(HT(FN1,K_A)==HT(MO(FN1),K_A),"enum layer");
_Static_assert(HT(OLD_LAYER,K_A)==HT(MO(3),K_A),"macro layer");
_Static_assert(HT(K_A,K_B)==DMK_HT_PACK(K_A,K_B),"HID A");
_Static_assert(HT((K_A),K_B)==HT(K_A,K_B),"parenthesized A");
_Static_assert(HT(HID_KEY_A,K_B)==HT(K_A,K_B),"long HID alias");
_Static_assert(HT(KEY(MY_A),K_B)==HT(K_A,K_B),"custom HID alias");
_Static_assert(HT(MOD_LSHIFT,K_A)==HT(MK(MOD_LSHIFT,K_NULL),K_A),"left mask");
_Static_assert(HT(16,K_APP)==HT(MK(MOD_RCTRL,K_NULL),K_APP),"legacy numeric mask");
_Static_assert(HT_T(K_A,K_B,80)==(HT(K_A,K_B)|(80u<<16)),"timeout preserves key spelling");
_Static_assert(HT_T(FN1,K_B,80)==HT_T(MO(FN1),K_B,80),"timeout preserves layer spelling");
_Static_assert(WRAPPED_HT(KEY(MY_A),K_B)==HT(K_A,K_B),"key marker survives wrapper expansion");
_Static_assert(WRAPPED_HT_T(KEY(MY_A),K_B,80)==HT_T(K_A,K_B,80),"key marker survives timed wrapper");
_Static_assert(WRAPPED_HT(FN1,K_A)==HT(MO(FN1),K_A),"old wrapper preserves layers");
_Static_assert(HT_T(MOD_LSHIFT,K_A,80)==HT_T(MK(MOD_LSHIFT,K_NULL),K_A,80),"timed mask spelling");
const uint32_t map[]={HT(1,K_A),HT(FN1,K_A),HT(K_A,K_B),HT(KEY(MY_A),K_B),HT(MOD_LSHIFT,K_A)};

int main(void) {
    const uint32_t expected[]={MO(1),MO(FN1),K_A,K_A,MK(MOD_LSHIFT,K_NULL)};
    for(unsigned i=0;i<5;++i){
        uint32_t hold,tap;uint8_t ms;
        assert(ht_decode(map[i],&hold,&tap,&ms));
        assert(hold==expected[i] && tap==(i==2 || i==3?K_B:K_A) && ms==0);
    }
    puts("Legacy numeric/enum/mask HT source and explicit HID markers passed");
}
