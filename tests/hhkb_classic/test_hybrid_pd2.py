"""Check the production startup PD2 sequence against register mocks."""
from pathlib import Path
import subprocess
import tempfile
import unittest

from .test_hybrid_button import ROOT, function


class HybridPd2Test(unittest.TestCase):
    def test_startup_high_then_low_after_spi_ready(self):
        platform = (ROOT / 'platforms/stm32l072/platform.c').read_text()
        hybrid = (ROOT / 'keyboards/hhkb/modules/hybrid/hybrid.c').read_text()
        code = r'''
#include <stdint.h>
#include <assert.h>
static struct {uint32_t IOPENR,APB2ENR;} rcc;
static struct gpio {uint32_t MODER,PUPDR,OTYPER,OSPEEDR,BSRR,BRR,AFR[2];} a,b,c,d;
static struct {uint32_t CR1,CR2,DR;} spi;
static volatile uint8_t *const spi_data=(volatile uint8_t *)&spi.DR;
#define RCC (&rcc)
#define GPIOA (&a)
#define GPIOB (&b)
#define GPIOC (&c)
#define GPIOD (&d)
#define SPI1 (&spi)
#define RCC_IOPENR_GPIOAEN 1U
#define RCC_IOPENR_GPIOBEN 2U
#define RCC_IOPENR_GPIOCEN 4U
#define RCC_IOPENR_GPIODEN 8U
#define RCC_APB2ENR_SPI1EN 1U
#define SPI_CR1_CPOL 1U
#define SPI_CR1_CPHA 2U
#define SPI_CR1_SPE 4U
#define SPI_CR2_RXNEIE 1U
#define SPI1_IRQn 1U
static unsigned irq_ready;
static void NVIC_EnableIRQ(unsigned irq){
 assert(irq==SPI1_IRQn);
 assert((spi.CR1 & SPI_CR1_SPE) && (spi.CR2 & SPI_CR2_RXNEIE));
 assert(d.BSRR==4 && d.BRR==0); // PD2 not released before SPI/IRQ readiness.
 ++irq_ready;
}
'''
        code += function(platform, 'static void hhkb_hybrid_startup_pd2(')
        code += function(hybrid, 'void hhkb_hybrid_init(')
        code += r'''
int main(void){
 d.MODER=d.OTYPER=d.PUPDR=UINT32_MAX;
 hhkb_hybrid_startup_pd2();
 assert(rcc.IOPENR & RCC_IOPENR_GPIODEN);
 assert(d.BSRR==4 && !d.BRR);
 assert((d.MODER & (3U<<4))==(1U<<4));
 assert(!(d.OTYPER & 4U) && !(d.PUPDR & (3U<<4)));
 assert((d.OTYPER | 4U)==UINT32_MAX); // Other pin configuration preserved.
 hhkb_hybrid_init();
 assert(irq_ready==1 && d.BRR==4);
 assert((d.MODER & (3U<<4))==(1U<<4) && !(d.OTYPER & 4U));
}
'''
        with tempfile.TemporaryDirectory() as tmp:
            c = Path(tmp) / 'pd2.c'
            binary = Path(tmp) / 'pd2'
            c.write_text(code)
            subprocess.run(['cc', '-std=c11', '-Wall', '-Wextra', '-Werror',
                            '-Wsign-compare', str(c), '-o', str(binary)], check=True)
            subprocess.run([str(binary)], check=True)
