.syntax unified
.cpu cortex-m3
.fpu softvfp
.thumb

/* start address for the initialization values of the .data section.
defined in linker script */
.word _sidata
/* start address for the .data section. defined in linker script */
.word _sdata
/* end address for the .data section. defined in linker script */
.word _edata
/* start address for the .bss section. defined in linker script */
.word _sbss
/* end address for the .bss section. defined in linker script */
.word _ebss

.global __Vectors
.global Default_Handler

/******************************************************************************
*
* The minimal vector table for a Cortex M3.  Note that the proper constructs
* must be placed on this to ensure that it ends up at physical address
* 0x0000.0000.
*
******************************************************************************/
  .section .isr_vector,"a",%progbits
  .type __Vectors, %object
  .size __Vectors, .-__Vectors

__Vectors:
  .word _estack             /* Top of Stack */
  .word Reset_Handler       /* Reset Handler */
  .word NMI_Handler         /* NMI Handler */
  .word HardFault_Handler   /* Hard Fault Handler */
  .word MemManage_Handler   /* MPU Fault Handler */
  .word BusFault_Handler    /* Bus Fault Handler */
  .word UsageFault_Handler  /* Usage Fault Handler */
  .word 0                   /* Reserved */
  .word 0                   /* Reserved */
  .word 0                   /* Reserved */
  .word 0                   /* Reserved */
  .word SVC_Handler         /* SVCall Handler */
  .word DebugMon_Handler    /* Debug Monitor Handler */
  .word 0                   /* Reserved */
  .word PendSV_Handler      /* PendSV Handler */
  .word SysTick_Handler     /* SysTick Handler */
  /* External Interrupts*/
  .word CAN1_IRQHandler            /*   0 CAN1 Handler */
  .word CAN2_IRQHandler            /*   1 CAN2 Handler */
  .word USB_IRQHandler             /*   2 USB Host Handler */
  .word 0                          /*     Reserved */
  .word 0                          /*     Reserved */
  .word DMA_IRQHandler             /*   5 DMA Handler */
  .word UART1_IRQHandler           /*   6 UART1 Handler */
  .word UART2_IRQHandler           /*   7 UART2 Handler */
  .word SSP1_IRQHandler            /*   8 SSP1 Handler */
  .word 0                          /*     Reserved */
  .word I2C_IRQHandler             /*  10 I2C Handler */
  .word POWER_IRQHandler           /*  11 POWER Handler */
  .word WWDG_IRQHandler            /*  12 WWDG Handler */
  .word 0                          /*     Reserved */
  .word Timer1_IRQHandler          /*  14 Timer1 Handler */
  .word Timer2_IRQHandler          /*  15 Timer2 Handler */
  .word Timer3_IRQHandler          /*  16 Timer3 Handler */
  .word ADC_IRQHandler             /*  17 ADC Handler */
  .word 0                          /*     Reserved */
  .word COMPARATOR_IRQHandler      /*  19 Comparator Handler */
  .word SSP2_IRQHandler            /*  20 SSP2 Handler */
  .word 0                          /*     Reserved */
  .word 0                          /*     Reserved */
  .word 0                          /*     Reserved */
  .word 0                          /*     Reserved */
  .word 0                          /*     Reserved */
  .word 0                          /*     Reserved */
  .word BACKUP_IRQHandler          /*  27 BKP Handler */
  .word EXT_INT1_IRQHandler        /*  28 EXT_INT1 Handler */
  .word EXT_INT2_IRQHandler        /*  29 EXT_INT2 Handler */
  .word EXT_INT3_IRQHandler        /*  30 EXT_INT3 Handler */
  .word EXT_INT4_IRQHandler        /*  31 EXT_INT4 Handler */


/*******************************************************************************
 * @brief  This is the code that gets called when the processor first
 *          starts execution following a reset event. Only the absolutely
 *          necessary set is performed, after which the application
 *          supplied main() routine is called.
 * @param  None
 * @retval : None
*******************************************************************************/

  .section .text.Reset_Handler
  .weak Reset_Handler
  .type Reset_Handler, %function
Reset_Handler:

/* Call the clock system initialization function.*/
    bl  SystemInit

/* Copy the data segment initializers from flash to SRAM */
  ldr r0, =_sdata
  ldr r1, =_edata
  ldr r2, =_sidata
  movs r3, #0
  b LoopCopyDataInit

CopyDataInit:
  ldr r4, [r2, r3]
  str r4, [r0, r3]
  adds r3, r3, #4

LoopCopyDataInit:
  adds r4, r0, r3
  cmp r4, r1
  bcc CopyDataInit
  
/* Zero fill the bss segment. */
  ldr r2, =_sbss
  ldr r4, =_ebss
  movs r3, #0
  b LoopFillZerobss

FillZerobss:
  str  r3, [r2]
  adds r2, r2, #4

LoopFillZerobss:
  cmp r2, r4
  bcc FillZerobss

/* Call static constructors */
    bl __libc_init_array
/* Call the application's entry point.*/
  bl main
  bx lr
.size Reset_Handler, .-Reset_Handler


/*******************************************************************************
 * @brief  This is the code that gets called when the processor receives an
 *         unexpected interrupt.  This simply enters an infinite loop, preserving
 *         the system state for examination by a debugger.
 *
 * @param  None
 * @retval : None
*******************************************************************************/
    .section .text.Default_Handler,"ax",%progbits
Default_Handler:
Infinite_Loop:
  b Infinite_Loop
  .size Default_Handler, .-Default_Handler


/*******************************************************************************
*
* Provide weak aliases for each Exception handler to the Default_Handler.
* As they are weak aliases, any function with the same name will override
* this definition.
*
*******************************************************************************/

  .weak NMI_Handler
  .thumb_set NMI_Handler,Default_Handler

  .weak HardFault_Handler
  .thumb_set HardFault_Handler,Default_Handler

  .weak MemManage_Handler
  .thumb_set MemManage_Handler,Default_Handler

  .weak BusFault_Handler
  .thumb_set BusFault_Handler,Default_Handler

  .weak UsageFault_Handler
  .thumb_set UsageFault_Handler,Default_Handler

  .weak SVC_Handler
  .thumb_set SVC_Handler,Default_Handler

  .weak DebugMon_Handler
  .thumb_set DebugMon_Handler,Default_Handler

  .weak PendSV_Handler
  .thumb_set PendSV_Handler,Default_Handler

  .weak SysTick_Handler
  .thumb_set SysTick_Handler,Default_Handler

  .weak CAN1_IRQHandler
  .thumb_set CAN1_IRQHandler,Default_Handler
  
  .weak CAN2_IRQHandler
  .thumb_set CAN2_IRQHandler,Default_Handler

  .weak USB_IRQHandler
  .thumb_set USB_IRQHandler,Default_Handler

  .weak DMA_IRQHandler
  .thumb_set DMA_IRQHandler,Default_Handler

  .weak UART1_IRQHandler
  .thumb_set UART1_IRQHandler,Default_Handler

  .weak UART2_IRQHandler
  .thumb_set UART2_IRQHandler,Default_Handler

  .weak SSP1_IRQHandler
  .thumb_set SSP1_IRQHandler,Default_Handler

  .weak I2C_IRQHandler
  .thumb_set I2C_IRQHandler,Default_Handler

  .weak POWER_IRQHandler
  .thumb_set POWER_IRQHandler,Default_Handler

  .weak WWDG_IRQHandler
  .thumb_set WWDG_IRQHandler,Default_Handler

  .weak Timer1_IRQHandler
  .thumb_set Timer1_IRQHandler,Default_Handler

  .weak Timer2_IRQHandler
  .thumb_set Timer2_IRQHandler,Default_Handler

  .weak Timer3_IRQHandler
  .thumb_set Timer3_IRQHandler,Default_Handler

  .weak ADC_IRQHandler
  .thumb_set ADC_IRQHandler,Default_Handler

  .weak COMPARATOR_IRQHandler
  .thumb_set COMPARATOR_IRQHandler,Default_Handler

  .weak SSP2_IRQHandler
  .thumb_set SSP2_IRQHandler,Default_Handler

  .weak BACKUP_IRQHandler
  .thumb_set BACKUP_IRQHandler,Default_Handler

  .weak EXT_INT1_IRQHandler
  .thumb_set EXT_INT1_IRQHandler,Default_Handler

  .weak EXT_INT2_IRQHandler
  .thumb_set EXT_INT2_IRQHandler,Default_Handler

  .weak EXT_INT3_IRQHandler
  .thumb_set EXT_INT3_IRQHandler,Default_Handler

  .weak EXT_INT4_IRQHandler
  .thumb_set EXT_INT4_IRQHandler,Default_Handler
