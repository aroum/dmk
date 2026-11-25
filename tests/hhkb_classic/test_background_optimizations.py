"""Exercise production wait policies and SSP DMA sequencing with host peripherals."""
from pathlib import Path
import subprocess
import tempfile
import unittest
from .test_hybrid_button import ROOT, function


def run_c(code):
    with tempfile.TemporaryDirectory() as tmp:
        src = Path(tmp) / 'test.c'
        out = Path(tmp) / 'test'
        src.write_text(code)
        subprocess.run(['clang', '-std=c11', '-Wall', '-Wextra', '-Werror',
                        str(src), '-o', str(out)], check=True)
        subprocess.run([str(out)], check=True)


class BackgroundOptimizationTest(unittest.TestCase):
    def test_eeprom_wait_tracks_settings_and_incremental_writes(self):
        source = (ROOT / 'dmk_core/features/vial.c').read_text()
        settings = (ROOT / 'keyboards/hhkb/modules/ec/ec_settings.c').read_text()
        compact = (ROOT / 'keyboards/hhkb/modules/via_settings/hhkb_compact_storage.inc').read_text()
        code = r'''
#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
typedef unsigned TickType_t;
#define pdMS_TO_TICKS(x) (x)
#define VIA_FULL_SETTINGS_RESET
#define C_RESET 42
static bool s_reset_pending,s_eeprom_dirty,s_settings_dirty,c_pending,c_packing;
static int c_target=-1,c_active=-1;
static bool c_blocked,c_keymap_ready=true,c_cal_ready=true;
static uint32_t owner=1;
static uint32_t *c_phys(unsigned off){(void)off;return &owner;}
'''
        code += function(compact, 'bool hhkb_compact_busy(')
        code += function(settings, 'bool vial_keyboard_eeprom_busy(')
        code += function(source, 'TickType_t vial_eeprom_poll_interval(')
        code += r'''
int main(void){
 assert(vial_eeprom_poll_interval()==1000);
 s_settings_dirty=true;assert(vial_eeprom_poll_interval()==2);s_settings_dirty=false;
 s_eeprom_dirty=true;assert(vial_eeprom_poll_interval()==2);s_eeprom_dirty=false;
 s_reset_pending=true;assert(vial_eeprom_poll_interval()==2);s_reset_pending=false;
 c_pending=true;assert(vial_eeprom_poll_interval()==2);c_pending=false;
 c_packing=true;assert(vial_eeprom_poll_interval()==2);c_packing=false;
 c_target=1;assert(vial_eeprom_poll_interval()==2);c_target=-1;
 c_active=0;owner=0;assert(vial_eeprom_poll_interval()==2);
 owner=C_RESET;assert(vial_eeprom_poll_interval()==2);
 owner=1;assert(vial_eeprom_poll_interval()==1000);
 c_pending=true;c_blocked=true;assert(vial_eeprom_poll_interval()==1000);
}
'''
        run_c(code)

    def test_ssp_dma_preserves_frame_and_waits_for_shifter_and_reset(self):
        source = (ROOT / 'platforms/milandr/ws2812_ssp.c').read_text()
        code = r'''
#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
typedef uint32_t TickType_t;
#define pdMS_TO_TICKS(x) (x)
#define portTICK_PERIOD_MS 1
#define taskSCHEDULER_RUNNING 1
#define DMA_Channel_SSP1_TX 4
#define DMA_Channel_SSP2_TX 6
#define RST_CLK_PCLK_DMA 1
#define SSP_DMA_TXE 2
#define DMA_SourceIncHalfword 1
#define DMA_DestIncNo 3
#define DMA_MemoryDataSize_HalfWord 17
#define DMA_Mode_Basic 1
#define DMA_Transfers_1 0
#define ENABLE 1
#define DISABLE 0
#define SSP_FLAG_BSY 1
#define SET 1
#define __DMB() ((void)0)
static unsigned ws2812_len,active_ssp_idx=1,ws2812_dma_timeouts;
static uint16_t _rgb[384];
static struct {uint32_t DR;} ssp;
static struct {uint32_t CHNL_ENABLE_SET;} dma;
#define MDR_DMA (&dma)
#define active_ssp (&ssp)
typedef struct {
 uint32_t DMA_SourceBaseAddr,DMA_DestBaseAddr,DMA_SourceIncSize,DMA_DestIncSize;
 uint32_t DMA_MemoryDataSize,DMA_Mode,DMA_CycleSize,DMA_NumContinuous;
} DMA_CtrlDataInitTypeDef;
typedef struct {DMA_CtrlDataInitTypeDef *DMA_PriCtrlData;} DMA_ChannelInitTypeDef;
static unsigned ticks,clock_calls,busy,reset_calls;
static bool stalled,requests,scheduled=true;
static void RST_CLK_PCLKcmd(unsigned p,unsigned on){assert(p==1 && on);++clock_calls;}
static void SSP_DMACmd(void *p,unsigned req,unsigned on){assert(p==active_ssp && req==2);requests=on;}
static void DMA_Cmd(unsigned ch,unsigned on){assert(!on);dma.CHNL_ENABLE_SET&=~(1u<<ch);}
static void DMA_StructInit(DMA_ChannelInitTypeDef *p){p->DMA_PriCtrlData=0;}
static void DMA_Init(unsigned ch,DMA_ChannelInitTypeDef *p){
 assert(ch==(active_ssp_idx==1?4u:6u));
 assert(p->DMA_PriCtrlData->DMA_CycleSize==ws2812_len*6);
 assert(p->DMA_PriCtrlData->DMA_SourceIncSize==1 && p->DMA_PriCtrlData->DMA_DestIncSize==3);
 assert(p->DMA_PriCtrlData->DMA_Mode==1);
 assert(!requests);dma.CHNL_ENABLE_SET=1u<<ch;
}
static unsigned xTaskGetSchedulerState(void){return scheduled;}
static void vTaskDelay(unsigned n){assert(n==1 && requests);++ticks;if(!stalled && ticks==3){dma.CHNL_ENABLE_SET=0;busy=2;}}
static unsigned SSP_GetFlagStatus(void *p,unsigned f){assert(p==active_ssp && f==1);if(busy){--busy;return SET;}return 0;}
static void hal_sleep_us(unsigned us){if(us==10){assert(!scheduled);++ticks;if(!stalled && ticks==3){dma.CHNL_ENABLE_SET=0;busy=2;}return;}assert(us==350 && !requests && !busy && !dma.CHNL_ENABLE_SET);++reset_calls;}
'''
        code += function(source, 'void ws2812_show(')
        code += r'''
int main(void){
 ws2812_show();assert(!clock_calls && !reset_calls);
 ws2812_len=64;ws2812_show();assert(ticks==3 && reset_calls==1 && !ws2812_dma_timeouts);
 active_ssp_idx=2;ticks=0;ws2812_len=1;ws2812_show();assert(ticks==3 && reset_calls==2);
 scheduled=false;ticks=0;ws2812_show();assert(ticks==3 && reset_calls==3);
 scheduled=true;stalled=true;ticks=0;ws2812_show();assert(ticks==20 && reset_calls==4 && ws2812_dma_timeouts==1);
}
'''
        # Host pointers are wider than MCU addresses; only hardware-address casts differ.
        code = code.replace('(uint32_t)_rgb', '(uint32_t)(uintptr_t)_rgb')
        code = code.replace('(uint32_t)&active_ssp->DR', '(uint32_t)(uintptr_t)&active_ssp->DR')
        run_c(code)
