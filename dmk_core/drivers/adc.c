#include "hal_adc.h"

// -----------------------------------------------------------------------------
// 1. Raspberry Pi RP2040 / RP2350
// -----------------------------------------------------------------------------
#if defined(MCU_rp2040) || defined(MCU_rp2350)
#include "hardware/adc.h"
#include "hardware/gpio.h"

static bool s_adc_inited = false;

void hal_adc_init(pin_t pin) {
    if (!s_adc_inited) {
        adc_init();
        s_adc_inited = true;
    }
    if (pin >= 26 && pin <= 29) {
        adc_gpio_init(pin);
    }
}

uint16_t hal_adc_read(pin_t pin) {
    if (pin < 26 || pin > 29) {
        return 0;
    }
    if (!s_adc_inited) {
        adc_init();
        s_adc_inited = true;
    }
    adc_select_input(pin - 26);
    return adc_read();
}

// -----------------------------------------------------------------------------
// 2. Milandr K1986BE92QI
// -----------------------------------------------------------------------------
#elif defined(MCU_milandr)
#include "MDR32FxQI_adc.h"
#include "MDR32FxQI_port.h"
#include "MDR32FxQI_rst_clk.h"

static bool s_adc_inited = false;

static inline uint8_t get_milandr_channel(pin_t pin) {
    if (pin >= PD0 && pin <= (PD0 + 7)) {
        return (uint8_t)(pin - PD0);
    }
    if (pin <= 7) {
        return (uint8_t)pin;
    }
    return 0xFF;
}

void hal_adc_init(pin_t pin) {
    uint8_t ch = get_milandr_channel(pin);
    if (ch == 0xFF)
        return;

    if (!s_adc_inited) {
        RST_CLK_PCLKcmd(RST_CLK_PCLK_ADC | RST_CLK_PCLK_PORTD, ENABLE);

        ADC_InitTypeDef sADC;
        ADC_StructInit(&sADC);
        ADC_Init(&sADC);

        ADCx_InitTypeDef sADCx;
        ADCx_StructInit(&sADCx);
        sADCx.ADC_ClockSource = ADC_CLOCK_SOURCE_CPU;
        sADCx.ADC_SamplingMode = ADC_SAMPLING_MODE_SINGLE_CONV;
        sADCx.ADC_ChannelSwitching = ADC_CH_SWITCHING_Disable;
        ADC1_Init(&sADCx);

        ADC1_Cmd(ENABLE);
        s_adc_inited = true;
    }

    PORT_InitTypeDef init;
    PORT_StructInit(&init);
    init.PORT_Pin = (1 << ch);
    init.PORT_OE = PORT_OE_IN;
    init.PORT_MODE = PORT_MODE_ANALOG;
    init.PORT_FUNC = PORT_FUNC_PORT;
    PORT_Init(MDR_PORTD, &init);
}

uint16_t hal_adc_read(pin_t pin) {
    uint8_t ch = get_milandr_channel(pin);
    if (ch == 0xFF)
        return 0;

    if (!s_adc_inited) {
        hal_adc_init(pin);
    }

    ADC1_SetChannel((ADCx_Channel_Number)ch);
    ADC1_Start();

    uint32_t timeout = 10000;
    while (ADC1_GetFlagStatus(ADC1_FLAG_END_OF_CONVERSION) == RESET && --timeout)
        ;

    return (uint16_t)(ADC1_GetResult() & 0x0FFF);
}

// -----------------------------------------------------------------------------
// 3. Nordic nRF52840
// -----------------------------------------------------------------------------
#elif defined(MCU_nrf52840)
#include "nrf_gpio.h"
#include "nrf_saadc.h"

static bool s_adc_inited = false;

static inline nrf_saadc_input_t get_nrf_input(pin_t pin) {
    switch (pin) {
    case P0_02:
        return NRF_SAADC_INPUT_AIN0;
    case P0_03:
        return NRF_SAADC_INPUT_AIN1;
    case P0_04:
        return NRF_SAADC_INPUT_AIN2;
    case P0_05:
        return NRF_SAADC_INPUT_AIN3;
    case P0_28:
        return NRF_SAADC_INPUT_AIN4;
    case P0_29:
        return NRF_SAADC_INPUT_AIN5;
    case P0_30:
        return NRF_SAADC_INPUT_AIN6;
    case P0_31:
        return NRF_SAADC_INPUT_AIN7;
    default:
        if (pin < 8)
            return (nrf_saadc_input_t)(pin + 1);
        return NRF_SAADC_INPUT_DISABLED;
    }
}

void hal_adc_init(pin_t pin) {
    nrf_saadc_input_t ain = get_nrf_input(pin);
    if (ain == NRF_SAADC_INPUT_DISABLED)
        return;

    if (!s_adc_inited) {
        nrf_saadc_enable();
        nrf_saadc_resolution_set(NRF_SAADC_RESOLUTION_12BIT);
        s_adc_inited = true;
    }
}

uint16_t hal_adc_read(pin_t pin) {
    nrf_saadc_input_t ain = get_nrf_input(pin);
    if (ain == NRF_SAADC_INPUT_DISABLED)
        return 0;

    if (!s_adc_inited) {
        hal_adc_init(pin);
    }

    nrf_saadc_channel_config_t config = {.resistor_p = NRF_SAADC_RESISTOR_DISABLED,
                                         .resistor_n = NRF_SAADC_RESISTOR_DISABLED,
                                         .gain = NRF_SAADC_GAIN1_6,
                                         .reference = NRF_SAADC_REFERENCE_INTERNAL,
                                         .acq_time = NRF_SAADC_ACQTIME_10US,
                                         .mode = NRF_SAADC_MODE_SINGLE_ENDED,
                                         .burst = NRF_SAADC_BURST_DISABLED,
                                         .pin_p = ain,
                                         .pin_n = NRF_SAADC_INPUT_DISABLED};
    nrf_saadc_channel_init(0, &config);

    nrf_saadc_value_t sample_val = 0;
    nrf_saadc_buffer_init(&sample_val, 1);

    nrf_saadc_task_trigger(NRF_SAADC_TASK_START);
    uint32_t timeout = 10000;
    while (!nrf_saadc_event_check(NRF_SAADC_EVENT_STARTED) && --timeout)
        ;
    nrf_saadc_event_clear(NRF_SAADC_EVENT_STARTED);

    nrf_saadc_task_trigger(NRF_SAADC_TASK_SAMPLE);
    timeout = 10000;
    while (!nrf_saadc_event_check(NRF_SAADC_EVENT_END) && --timeout)
        ;
    nrf_saadc_event_clear(NRF_SAADC_EVENT_END);

    nrf_saadc_task_trigger(NRF_SAADC_TASK_STOP);
    timeout = 10000;
    while (!nrf_saadc_event_check(NRF_SAADC_EVENT_STOPPED) && --timeout)
        ;
    nrf_saadc_event_clear(NRF_SAADC_EVENT_STOPPED);

    if (sample_val < 0)
        sample_val = 0;
    if (sample_val > 4095)
        sample_val = 4095;
    return (uint16_t)sample_val;
}

// -----------------------------------------------------------------------------
// 4. Baikal BE-T1000 / BMCU
// -----------------------------------------------------------------------------
#elif defined(MCU_baikal)
#include "bmcu_adc.h"
#include "bmcu_cru.h"
#include "bmcu_gpio.h"

static bool s_adc_inited = false;

void hal_adc_init(pin_t pin) {
    if (!s_adc_inited) {
        CRU_APB2_EnablePeriph(CRU_APB2_PERIPH_ADC0);

        ADC_InitStruct_TypeDef init;
        ADC_StructInit(&init);
        init.ConversionMode = ADC_CONV_SINGLE;
        init.SequencerScanMode = ADC_SEQ_SCAN_DISABLE;
        init.SequencerLength = ADC_SEQ_SCAN_LENGTH_1RANK;
        init.TriggerSource = ADC_TRIG_SOFTWARE;
        ADC_Init(ADC0, &init);
        ADC_Enable(ADC0);

        s_adc_inited = true;
    }

    uint8_t port = pin / 16;
    uint16_t mask = (uint16_t)(1U << (pin % 16));
    CRU_SetPinPull(port, mask, CRU_PIN_PULL_NONE);
}

uint16_t hal_adc_read(pin_t pin) {
    uint8_t ch = (uint8_t)(pin % 8);
    if (!s_adc_inited)
        hal_adc_init(pin);

    ADC_SetSequencerRanks(ADC0, ADC_RANK0, (ADC_Channels_TypeDef)ch);
    ADC_StartConversionSWStart(ADC0);

    uint32_t timeout = 10000;
    while (!ADC_IsActiveFlag_EOC(ADC0) && --timeout)
        ;

    return ADC_ReadConversionData12(ADC0);
}

#else
// Mock / Host test build
void hal_adc_init(pin_t pin) {
    (void)pin;
}
uint16_t hal_adc_read(pin_t pin) {
    (void)pin;
    return 0;
}
#endif
