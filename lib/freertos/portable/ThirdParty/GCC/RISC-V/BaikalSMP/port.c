/*
 * FreeRTOS Kernel V11.1.0
 * Copyright (C) 2021 Amazon.com, Inc. or its affiliates. All Rights Reserved.
 * Copyright (c) 2024 Raspberry Pi (Trading) Ltd.
 * Copyright (c) 2025 Baikal Electronics JSC JSC
 *
 * SPDX-License-Identifier: MIT AND BSD-3-Clause
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of
 * this software and associated documentation files (the "Software"), to deal in
 * the Software without restriction, including without limitation the rights to
 * use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
 * the Software, and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 * https://www.FreeRTOS.org
 * https://github.com/FreeRTOS
 *
 */

/*-----------------------------------------------------------
* Implementation of functions defined in portable.h for the RISC-V port.
*----------------------------------------------------------*/

/* Scheduler includes. */
#include "FreeRTOS.h"
#include "task.h"
#include "portmacro.h"

/* Standard includes. */
#include "string.h"

/* Let the user override the pre-loading of the initial RA. */
#ifdef configTASK_RETURN_ADDRESS
    #define portTASK_RETURN_ADDRESS    configTASK_RETURN_ADDRESS
#else
    #define portTASK_RETURN_ADDRESS    0
#endif

#ifndef configUSE_ISR_STACK
#define configUSE_ISR_STACK 1
#endif

/* The stack used by interrupt service routines.  Set configISR_STACK_SIZE_WORDS
 * to use a statically allocated array as the interrupt stack.  Alternative leave
 * configISR_STACK_SIZE_WORDS undefined and update the linker script so that a
 * linker variable names __freertos_irq_stack_top has the same value as the top
 * of the stack used by main.  Using the linker script method will repurpose the
 * stack that was used by main before the scheduler was started for use as the
 * interrupt stack after the scheduler has started. */
#ifdef configISR_STACK_SIZE_WORDS
#error configISR_STACK_SIZE_WORDS not yet supported; we need per core ones
static __attribute__( ( aligned( 16 ) ) ) StackType_t xISRStack[ configISR_STACK_SIZE_WORDS ] = { 0 };
const StackType_t xISRStackTop = ( StackType_t ) &( xISRStack[ configISR_STACK_SIZE_WORDS & ~portBYTE_ALIGNMENT_MASK ] );

/* Don't use 0xa5 as the stack fill bytes as that is used by the kernel for
 * the task stacks, and so will legitimately appear in many positions within
 * the ISR stack. */
    #define portISR_STACK_FILL_BYTE    0xee
#else
    /*
     * If these are 0, then no stack switch is performed.
     *
     * Note: if configUSE_ISR_STACK == 0, then this is never initialized,
     * but we keep this array around, because portASM.S / portContext.h does not
     * have access to config variables
     */
    StackType_t xISRStackTops[configNUMBER_OF_CORES] = {0};
#endif

/*
 * Setup the timer to generate the tick interrupts.  The implementation in this
 * file is weak to allow application writers to change the timer used to
 * generate the tick interrupt.
 */
void vPortSetupTimerInterrupt( void ) __attribute__( ( weak ) );

/*-----------------------------------------------------------*/

/* Used to program the machine timer compare register. */
uint64_t ullNextTime = 0ULL;
const uint64_t * pullNextTime = &ullNextTime;
const size_t uxTimerIncrementsForOneTick = ( size_t ) ( ( configCPU_CLOCK_HZ ) / ( configTICK_RATE_HZ ) ); /* Assumes increment won't go over 32-bits. */
uint32_t const ullMachineTimerCompareRegisterBase = configMTIMECMP_BASE_ADDRESS;
volatile uint64_t * pullMachineTimerCompareRegister = NULL;

/* Holds the critical nesting value - deliberately non-zero at start up to
 * ensure interrupts are not accidentally enabled before the scheduler starts. */
typedef struct tskTaskControlBlock TCB_t;
#if configNUMBER_OF_CORES == 1
size_t xCriticalNesting = ( size_t ) 0xaaaaaaaa;
size_t *xCriticalNestingArray = &xCriticalNesting;
extern portDONT_DISCARD PRIVILEGED_DATA TCB_t * volatile pxCurrentTCB;
TCB_t * volatile * pxCurrentTCBArray = &pxCurrentTCB;
#else
size_t xCriticalNestings[ configNUMBER_OF_CORES ] = { 0 };
size_t *xCriticalNestingArray = &xCriticalNestings[0];
extern portDONT_DISCARD PRIVILEGED_DATA TCB_t * volatile pxCurrentTCBs[ configNUMBER_OF_CORES ];
TCB_t * volatile * pxCurrentTCBArray = pxCurrentTCBs;
#endif

/* Used to catch tasks that attempt to return from their implementing function. */
size_t xTaskReturnAddress = ( size_t ) portTASK_RETURN_ADDRESS;

/* Set configCHECK_FOR_STACK_OVERFLOW to 3 to add ISR stack checking to task
 * stack checking.  A problem in the ISR stack will trigger an assert, not call
 * the stack overflow hook function (because the stack overflow hook is specific
 * to a task stack, not the ISR stack). */
#if defined( configISR_STACK_SIZE_WORDS ) && ( configCHECK_FOR_STACK_OVERFLOW > 2 )
    #warning "This path not tested, or even compiled yet."

    static const uint8_t ucExpectedStackBytes[] =
    {
        portISR_STACK_FILL_BYTE, portISR_STACK_FILL_BYTE, portISR_STACK_FILL_BYTE, portISR_STACK_FILL_BYTE, \
        portISR_STACK_FILL_BYTE, portISR_STACK_FILL_BYTE, portISR_STACK_FILL_BYTE, portISR_STACK_FILL_BYTE, \
        portISR_STACK_FILL_BYTE, portISR_STACK_FILL_BYTE, portISR_STACK_FILL_BYTE, portISR_STACK_FILL_BYTE, \
        portISR_STACK_FILL_BYTE, portISR_STACK_FILL_BYTE, portISR_STACK_FILL_BYTE, portISR_STACK_FILL_BYTE, \
        portISR_STACK_FILL_BYTE, portISR_STACK_FILL_BYTE, portISR_STACK_FILL_BYTE, portISR_STACK_FILL_BYTE
    }; \

    #define portCHECK_ISR_STACK()    configASSERT( ( memcmp( ( void * ) xISRStack, ( void * ) ucExpectedStackBytes, sizeof( ucExpectedStackBytes ) ) == 0 ) )
#else /* if defined( configISR_STACK_SIZE_WORDS ) && ( configCHECK_FOR_STACK_OVERFLOW > 2 ) */
    /* Define the function away. */
    #define portCHECK_ISR_STACK()
#endif /* configCHECK_FOR_STACK_OVERFLOW > 2 */

#define INVALID_PRIMARY_CORE_NUM    0xffu
/* The primary core number (the own which has the SysTick handler) */
static uint8_t ucPrimaryCoreNum = INVALID_PRIMARY_CORE_NUM;

/* Note: portIS_FREE_RTOS_CORE() also returns false until the scheduler is started */
#if ( configNUMBER_OF_CORES != 1 )
#define portIS_FREE_RTOS_CORE()    ( ucPrimaryCoreNum != INVALID_PRIMARY_CORE_NUM )
#else
#define portIS_FREE_RTOS_CORE()    ( ucPrimaryCoreNum == get_core_num() )
#endif

/*-----------------------------------------------------------*/

#if ( configMTIME_BASE_ADDRESS != 0 ) && ( configMTIMECMP_BASE_ADDRESS != 0 )

    void vPortSetupTimerInterrupt( void )
    {
        uint32_t ulCurrentTimeHigh, ulCurrentTimeLow;
        volatile uint32_t * const pulTimeHigh = ( volatile uint32_t * const ) ( ( configMTIME_BASE_ADDRESS ) + 4UL ); /* 8-byte type so high 32-bit word is 4 bytes up. */
        volatile uint32_t * const pulTimeLow = ( volatile uint32_t * const ) ( configMTIME_BASE_ADDRESS );
        volatile uint32_t ulHartId;

        __asm volatile ( "csrr %0, mhartid" : "=r" ( ulHartId ) );

        pullMachineTimerCompareRegister = ( volatile uint64_t * ) ( ullMachineTimerCompareRegisterBase + ( ulHartId * sizeof( uint64_t ) ) );

        do
        {
            ulCurrentTimeHigh = *pulTimeHigh;
            ulCurrentTimeLow = *pulTimeLow;
        } while( ulCurrentTimeHigh != *pulTimeHigh );

        ullNextTime = ( uint64_t ) ulCurrentTimeHigh;
        ullNextTime <<= 32ULL; /* High 4-byte word is 32-bits up. */
        ullNextTime |= ( uint64_t ) ulCurrentTimeLow;
        ullNextTime += ( uint64_t ) uxTimerIncrementsForOneTick;
        *pullMachineTimerCompareRegister = ullNextTime;

        /* Prepare the time to use after the next tick interrupt. */
        ullNextTime += ( uint64_t ) uxTimerIncrementsForOneTick;

        /* Enable timer interrupt */
        CLIC_EnableIRQ(CLIC_MT_IRQn);
    }

#endif /* ( configMTIME_BASE_ADDRESS != 0 ) && ( configMTIME_BASE_ADDRESS != 0 ) */
/*-----------------------------------------------------------*/

StackType_t xPortInitISRStack(void) {
    StackType_t xISRStackTop;
    //( StackType_t ) /*__freertos_irq_stack_top*/__StackTop;
    // todo fixup for allocated stacks
    if (get_core_num())
    {
        extern const uint32_t __StackOneTop[];
        xISRStackTop = ( StackType_t ) __StackOneTop;
    }
    else
    {
        extern const uint32_t __StackTop[];
        xISRStackTop = ( StackType_t ) __StackTop;
    }
    xISRStackTops[get_core_num()] = xISRStackTop;
    return xISRStackTop;
}
/*-----------------------------------------------------------*/

#if ( configNUMBER_OF_CORES != 1 )
    uint32_t ulDoorbell0To1Count = 0;
    static void prvDoorbell0To1InterruptHandler()
    {
        CLIC_DisableIRQ(DOORBELL_0TO1_IRQ);
        GPIO_ResetOutputPin(DOORBELL_0TO1_GPIO, DOORBELL_0TO1_PIN);
        ulDoorbell0To1Count++;
        portYIELD_FROM_ISR( pdTRUE );
    }

    uint32_t ulDoorbell1To0Count = 0;
    static void prvDoorbell1To0InterruptHandler()
    {
        CLIC_DisableIRQ(DOORBELL_1TO0_IRQ);
        GPIO_ResetOutputPin(DOORBELL_1TO0_GPIO, DOORBELL_1TO0_PIN);
        ulDoorbell1To0Count++;
        portYIELD_FROM_ISR( pdTRUE );
    }

    static void prvSetupDoorbellHandler(uint32_t port, uint32_t pin, uint32_t irq, irq_handler_t handler)
    {
        CRU_C0_C1_EXTI_SetSource(port, pin);
        CLIC_ConfigIRQ(irq,                             /* Interrupt */
                       CLIC_INTATTR_MODE_MACHINE,       /* Privilege mode */
                       1U,                              /* Level */
                       1U,                              /* Priority */
                       CLIC_INTATTR_SHV_VECTORED,       /* Vector mode */
                       CLIC_INTATTR_TRIG_TYPE_LEVEL,    /* Type */
                       CLIC_INTATTR_TRIG_POL_P);        /* Polarity */
        set_irq_handler(irq, handler);
        CLIC_EnableIRQ(irq);
    }

    static void prvInitAndClearDoorbell(GPIO_TypeDef *gpio, uint32_t pin)
    {
        GPIO_InitStruct_TypeDef GPIO_InitStruct;
        GPIO_StructInit(&GPIO_InitStruct);
        GPIO_Init(gpio, &GPIO_InitStruct);
        GPIO_SetPinMode(gpio, pin, GPIO_MODE_OUTPUT);
        GPIO_ResetOutputPin(gpio, pin);
    }
#endif /* ( configNUMBER_OF_CORES != 1) */
/*-----------------------------------------------------------*/

#if ( configNUMBER_OF_CORES != 1 )
    static BaseType_t xPortStartSchedulerOnCore( void )
    {
        extern void xPortStartFirstTask( void );
        #if ( configUSE_ISR_STACK == 1)
            StackType_t xISRStackTop = xPortInitISRStack();

            #if ( configASSERT_DEFINED == 1 )
                {
                    /* Check alignment of the interrupt stack - which is the same as the
                     * stack that was being used by main() prior to the scheduler being
                     * started. */
                    configASSERT( ( xISRStackTop & portBYTE_ALIGNMENT_MASK ) == 0 );

                    #ifdef configISR_STACK_SIZE_WORDS
                    {
                        memset( ( void * ) xISRStack, portISR_STACK_FILL_BYTE, sizeof( xISRStack ) );
                    }
                    #endif /* configISR_STACK_SIZE_WORDS */
                }
            #endif /* configASSERT_DEFINED */
        #endif /* configUSE_ISR_STACK */

        if( ucPrimaryCoreNum == get_core_num() ) {
            /* If there is a CLINT then it is ok to use the default implementation
             * in this file, otherwise vPortSetupTimerInterrupt() must be implemented to
             * configure whichever clock is to be used to generate the tick interrupt. */
            vPortSetupTimerInterrupt();

            #if ((configMTIME_BASE_ADDRESS != 0) && (configMTIMECMP_BASE_ADDRESS != 0))
                {
                    /* Enable mtime and external interrupts.  1<<7 for timer interrupt,
                     * 1<<11 for external interrupt.  _RB_ What happens here when mtime is
                     * not present as with pulpino? */
                    __asm volatile ( "csrs mie, %0" ::"r" ( 0x880 ) );
                }
            #endif /* ( configMTIME_BASE_ADDRESS != 0 ) && ( configMTIMECMP_BASE_ADDRESS != 0 ) */
        }

        /* Install Doorbell handler to receive interrupt from other core */
        if (portGET_CORE_ID() == 0) {
            prvSetupDoorbellHandler(DOORBELL_1TO0_PORT, DOORBELL_1TO0_PIN, DOORBELL_1TO0_IRQ, prvDoorbell1To0InterruptHandler);
        } else {
            prvSetupDoorbellHandler(DOORBELL_0TO1_PORT, DOORBELL_0TO1_PIN, DOORBELL_0TO1_IRQ, prvDoorbell0To1InterruptHandler);
        }

        xPortStartFirstTask();

        /* Should not get here as after calling xPortStartFirstTask() only tasks
         * should be executing. */
        return pdFAIL;
    }

    void prvDisableInterruptsAndPortStartSchedulerOnCore( void )
    {
        portDISABLE_INTERRUPTS();
        xPortStartSchedulerOnCore();
    }

    BaseType_t xPortStartScheduler( void )
    {
        /* start1 is a startup code for Core 1;
           by design start1 executes prvDisableInterruptsAndPortStartSchedulerOnCore() */
        extern void _start1( void );

        configASSERT( ucPrimaryCoreNum == INVALID_PRIMARY_CORE_NUM );

        spin_lock_init( configSMP_SPINLOCK_0 );
        spin_lock_init( configSMP_SPINLOCK_1 );

        prvInitAndClearDoorbell(DOORBELL_0TO1_GPIO, DOORBELL_0TO1_PIN);
        prvInitAndClearDoorbell(DOORBELL_1TO0_GPIO, DOORBELL_1TO0_PIN);

        ucPrimaryCoreNum = 0;
        configASSERT( get_core_num() == 0 ); /* we must be started on core 0 */

        /* Reset Core 1 */
        CRU->SYSCR0 &= ~(CRU_SYSCR0_CORE1RSTN | CRU_SYSCR0_CORE1FPRSTN | CRU_SYSCR0_CORE1CXRSTN);
        __delay_ms(10UL);

        /* Set Core 1 start address */
        CRU->SYSCR1 = (uint32_t) _start1;

        /* Enable Core 1 clock */
        CRU->SYSCR0 |= CRU_SYSCR0_CORE1CLKEN;

        /* Release Core 1 from reset */
        CRU->SYSCR0 |= (CRU_SYSCR0_CORE1RSTN | CRU_SYSCR0_CORE1FPRSTN | CRU_SYSCR0_CORE1CXRSTN);

        xPortStartSchedulerOnCore();

        /* Should not get here! */
        return 0;
    }
#else /* if ( configNUMBER_OF_CORES != 1 ) */
    BaseType_t xPortStartScheduler( void )
    {
        extern void xPortStartFirstTask( void );

        #if ( configUSE_ISR_STACK == 1)
            StackType_t xISRStackTop = xPortInitISRStack();

            #if ( configASSERT_DEFINED == 1 )
            {
                /* Check alignment of the interrupt stack - which is the same as the
                 * stack that was being used by main() prior to the scheduler being
                 * started. */
                configASSERT( ( xISRStackTop & portBYTE_ALIGNMENT_MASK ) == 0 );

                #ifdef configISR_STACK_SIZE_WORDS
                {
                    memset( ( void * ) xISRStack, portISR_STACK_FILL_BYTE, sizeof( xISRStack ) );
                }
                #endif /* configISR_STACK_SIZE_WORDS */
            }
            #endif /* configASSERT_DEFINED */
        #endif /* configUSE_ISR_STACK */

        /* If there is a CLINT then it is ok to use the default implementation
         * in this file, otherwise vPortSetupTimerInterrupt() must be implemented to
         * configure whichever clock is to be used to generate the tick interrupt. */
        vPortSetupTimerInterrupt();

    //    extern void freertos_risc_v_trap_handler(void);
    //    exception_set_exclusive_handler(0, freertos_risc_v_trap_handler);
        #if ( ( configMTIME_BASE_ADDRESS != 0 ) && ( configMTIMECMP_BASE_ADDRESS != 0 ) )
        {
            /* Enable mtime and external interrupts.  1<<7 for timer interrupt,
             * 1<<11 for external interrupt.  _RB_ What happens here when mtime is
             * not present as with pulpino? */
            __asm volatile ( "csrs mie, %0" ::"r" ( 0x880 ) );
        }
        #endif /* ( configMTIME_BASE_ADDRESS != 0 ) && ( configMTIMECMP_BASE_ADDRESS != 0 ) */

        ucPrimaryCoreNum = get_core_num();

        xPortStartFirstTask();

        /* Should not get here as after calling xPortStartFirstTask() only tasks
         * should be executing. */
        return pdFAIL;
    }
#endif /* ( configNUMBER_OF_CORES != 1 ) */
/*-----------------------------------------------------------*/

void vPortEndScheduler( void )
{
    /* Not implemented. */
    for( ; ; )
    {
    }
}
/*-----------------------------------------------------------*/

#if ( configNUMBER_OF_CORES != 1 )
    void vYieldCore( int xCoreID )
    {
        /* Remove warning if configASSERT is not defined.
         * xCoreID is not used in this function due to this is a dual-core system. The yielding core must be different from the current core. */
        ( void ) xCoreID;

        //configASSERT( xCoreID != ( int ) portGET_CORE_ID() );

    #if ( configNUMBER_OF_CORES != 1 )

        /* will cause interrupt on other core if not already pending */
        //configASSERT( cDoorbellNum >= 0);
        if ( portGET_CORE_ID() == 0 ) {
            GPIO_SetOutputPin(DOORBELL_0TO1_GPIO, DOORBELL_0TO1_PIN);
        } else {
            GPIO_SetOutputPin(DOORBELL_1TO0_GPIO, DOORBELL_1TO0_PIN);
        }
    #endif
    }
#endif // ( configNUMBER_OF_CORES != 1 )

/*-----------------------------------------------------------*/

/* todo need to move portASM.s back into portasm.c so we can  inline this */
UBaseType_t callTaskEnterCriticalFromISR(void) {
    #if ( configNUMBER_OF_CORES > 1 )
        return taskENTER_CRITICAL_FROM_ISR();
    #else
        return 0;
    #endif
}

/* todo need to move portASM.s back into portasm.c so we can  inline this */
void callTaskExitCriticalFromISR(UBaseType_t uxSavedInterruptStatus) {
    #if ( configNUMBER_OF_CORES > 1 )
        taskEXIT_CRITICAL_FROM_ISR( uxSavedInterruptStatus );
    #endif
}
