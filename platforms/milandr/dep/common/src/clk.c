#include "clk.h"

void CLK_Init_80_mhz(void)
{
  RST_CLK_DeInit();
  SystemCoreClockUpdate();

  RST_CLK_HSEconfig(RST_CLK_HSE_ON);
  while (RST_CLK_HSEstatus() == ERROR) {
  }

  RST_CLK_CPU_PLLconfig(RST_CLK_CPU_PLLsrcHSEdiv1, RST_CLK_CPU_PLLmul10);
  RST_CLK_CPU_PLLcmd(ENABLE);
  while (RST_CLK_CPU_PLLstatus() == ERROR) {
  }

  RST_CLK_CPU_PLLuse(ENABLE);
  RST_CLK_CPUclkPrescaler(RST_CLK_CPUclkDIV1);

  RST_CLK_PCLKcmd(RST_CLK_PCLK_EEPROM, ENABLE);
  EEPROM_SetLatency(EEPROM_Latency_3);
  RST_CLK_PCLKcmd(RST_CLK_PCLK_EEPROM, DISABLE);

  RST_CLK_PCLKcmd(RST_CLK_PCLK_BKP, ENABLE);
  BKP_DUccMode(BKP_DUcc_upto_80MHz);

  RST_CLK_CPUclkSelection(RST_CLK_CPUclkCPU_C3);
  SystemCoreClockUpdate();
}
