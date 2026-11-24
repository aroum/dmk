#include "MDR32FxQI_dma.h"

// The vendor SPL defines its shared control table only for IAR/ARMCC/Phyton.
// Supply the same primary/alternate table for GCC without modifying the SDK.
#if defined(__GNUC__) && !defined(__ARMCC_VERSION)
DMA_CtrlDataTypeDef DMA_ControlTable[32 * DMA_AlternateData + DMA_Channels_Number]
    __attribute__((aligned(1024)));
#endif
