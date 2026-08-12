.section .text1.start

.globl _start1
.type _start1, %function

/* Entry point */
_start1:
    j Reset_Handler1

.section .text1

.set MTVT, 0x307
.set NMITVEC, 0x7C0

/* Reset vector */
.globl Reset_Handler1
.type Reset_Handler1, %function

Reset_Handler1:
    /* Set stack pointer (SP) to the top of the stack */
    la sp, __StackOneTop

    /* Set interrupt mode and trap handler address */
    la t0, freertos_risc_v_exception_handler
    ori t0, t0, 0x3
    csrw mtvec, t0

    /* Set interrupt vector table address */
    la t0, mtvt_clic_vector_table1
    csrw MTVT, t0

    /* Set handler for Non-Maskable Interrupts */
    la t0, NMI_Handler
    csrw NMITVEC, t0

    /* Initialize data in RAM */
//copy_data:
    //la t0, _sidata
    //la t1, _sdata
    //la t2, _edata

//copy_data_loop:
    //lb t3, 0(t0)
    //sb t3, 0(t1)
    //addi t0, t0, 1
    //addi t1, t1, 1
    //blt t1, t2, copy_data_loop

    /* Clear the .bss section */
//clear_bss:
    //la t0, _sbss
    //la t1, _ebss
    //li t2, 0

//clear_bss_loop:
    //sb t2, 0(t0)
    //addi t0, t0, 1
    //blt t0, t1, clear_bss_loop

    /* Call the main function */
    call prvDisableInterruptsAndPortStartSchedulerOnCore

    /* If main returns, loop infinitely */
    j .

/* Default ISR handler */
.balign 64, 0
.type Default_Handler, %function
Default_Handler:
    /* Infinite loop */
    j .

/* ISR vector table */
.section .text1.isr_vector,"a",%progbits

.balign 64, 0
.globl mtvt_clic_vector_table1
.type mtvt_clic_vector_table1, %object
mtvt_clic_vector_table1:
    .word freertos_risc_v_interrupt_handler        /* IRQ 0 */
    .word freertos_risc_v_interrupt_handler
    .word freertos_risc_v_interrupt_handler
    .word freertos_risc_v_interrupt_handler
    .word freertos_risc_v_interrupt_handler
    .word freertos_risc_v_interrupt_handler
    .word freertos_risc_v_interrupt_handler
    .word freertos_risc_v_mtimer_interrupt_handler /* IRQ 7 - Machine timer interrupt */
    .word freertos_risc_v_interrupt_handler
    .word freertos_risc_v_interrupt_handler
    .word freertos_risc_v_interrupt_handler
    .word freertos_risc_v_interrupt_handler
    .word freertos_risc_v_interrupt_handler
    .word freertos_risc_v_interrupt_handler
    .word freertos_risc_v_interrupt_handler
    .word freertos_risc_v_interrupt_handler        /* IRQ 15 */
    .word QSPI0_IRQHandler          /* QSPI0 interrupt handler */
    .word UART0_IRQHandler          /* UART0 interrupt handler */
    .word UART1_IRQHandler          /* UART1 interrupt handler */
    .word UART2_IRQHandler          /* UART2 interrupt handler */
    .word I2C0_IRQHandler           /* I2C0 interrupt handler */
    .word I2C1_IRQHandler           /* I2C1 interrupt handler */
    .word I2S0_IRQHandler           /* I2S0 interrupt handler */
    .word TIM0_Channel0_IRQHandler  /* TIM0 Channel 0 interrupt handler */
    .word TIM0_Channel1_IRQHandler  /* TIM0 Channel 1 interrupt handler */
    .word TIM0_Channel2_IRQHandler  /* TIM0 Channel 2 interrupt handler */
    .word TIM0_Channel3_IRQHandler  /* TIM0 Channel 3 interrupt handler */
    .word WDT0_IRQHandler           /* WDT0 interrupt handler */
    .word GPIO0_IRQHandler          /* GPIO0 interrupt handler */
    .word PWMG0_IRQHandler          /* PWMG0 interrupt handler */
    .word SPI0_IRQHandler           /* SPI0 interrupt handler */
    .word SPI1_IRQHandler           /* SPI1 interrupt handler */
    .word CANFD0_IRQHandler         /* CANFD0 interrupt handler */
    .word QSPI1_IRQHandler          /* QSPI1 interrupt handler */
    .word UART3_IRQHandler          /* UART3 interrupt handler */
    .word UART4_IRQHandler          /* UART4 interrupt handler */
    .word UART5_IRQHandler          /* UART5 interrupt handler */
    .word I2C2_IRQHandler           /* I2C2 interrupt handler */
    .word I2C3_IRQHandler           /* I2C3 interrupt handler */
    .word I2S1_IRQHandler           /* I2S1 interrupt handler */
    .word TIM1_Channel0_IRQHandler  /* TIM1 Channel 0 interrupt handler */
    .word TIM1_Channel1_IRQHandler  /* TIM1 Channel 1 interrupt handler */
    .word TIM1_Channel2_IRQHandler  /* TIM1 Channel 2 interrupt handler */
    .word TIM1_Channel3_IRQHandler  /* TIM1 Channel 3 interrupt handler */
    .word WDT1_IRQHandler           /* WDT1 interrupt handler */
    .word GPIO1_IRQHandler          /* GPIO1 interrupt handler */
    .word PWMG1_IRQHandler          /* PWMG1 interrupt handler */
    .word SPI2_IRQHandler           /* SPI2 interrupt handler */
    .word SPI3_IRQHandler           /* SPI3 interrupt handler */
    .word CANFD1_IRQHandler         /* CANFD1 interrupt handler */
    .word ADC0_IRQHandler           /* ADC0 interrupt handler */
    .word ADC1_IRQHandler           /* ADC1 interrupt handler */
    .word ADC2_IRQHandler           /* ADC2 interrupt handler */
    .word PWMA0_IRQHandler          /* PWMA0 interrupt handler */
    .word QE0_IRQHandler            /* QE0 interrupt handler */
    .word PWMA1_IRQHandler          /* PWMA1 interrupt handler */
    .word QE1_IRQHandler            /* QE1 interrupt handler */
    .word PWMA2_IRQHandler          /* PWMA2 interrupt handler */
    .word QE2_IRQHandler            /* QE2 interrupt handler */
    .word PWMA3_IRQHandler          /* PWMA3 interrupt handler */
    .word QE3_IRQHandler            /* QE3 interrupt handler */
    .word GPIO2_IRQHandler          /* GPIO2 interrupt handler */
    .word UART6_IRQHandler          /* UART6 interrupt handler */
    .word UART7_IRQHandler          /* UART7 interrupt handler */
    .word USB_IRQHandler            /* USB interrupt handler */
    .word DMA0_IRQHandler           /* DMA0 interrupt handler */
    .word DMA1_IRQHandler           /* DMA1 interrupt handler */
    .word DMA2_IRQHandler           /* DMA2 interrupt handler */
    .word CORE2_IRQHandler          /* CORE2 interrupt handler */
    .word EXTI_PA_IRQHandler        /* EXTI PA interrupt handler */
    .word EXTI_PB_IRQHandler        /* EXTI PB interrupt handler */
    .word EXTI_PC_IRQHandler        /* EXTI PC interrupt handler */
    .word CLKI_Mon_IRQHandler       /* CLKI Monitor interrupt handler */

.size mtvt_clic_vector_table1, . - mtvt_clic_vector_table1

.balign 4, 0
.weak NMI_Handler
.set NMI_Handler,Default_Handler

.balign 64, 0
.weak QSPI0_IRQHandler
.set QSPI0_IRQHandler,Default_Handler

.weak UART0_IRQHandler
.set UART0_IRQHandler,Default_Handler

.weak UART1_IRQHandler
.set UART1_IRQHandler,Default_Handler

.weak UART2_IRQHandler
.set UART2_IRQHandler,Default_Handler

.weak I2C0_IRQHandler
.set I2C0_IRQHandler,Default_Handler

.weak I2C1_IRQHandler
.set I2C1_IRQHandler,Default_Handler

.weak I2S0_IRQHandler
.set I2S0_IRQHandler,Default_Handler

.weak TIM0_Channel0_IRQHandler
.set TIM0_Channel0_IRQHandler,Default_Handler

.weak TIM0_Channel1_IRQHandler
.set TIM0_Channel1_IRQHandler,Default_Handler

.weak TIM0_Channel2_IRQHandler
.set TIM0_Channel2_IRQHandler,Default_Handler

.weak TIM0_Channel3_IRQHandler
.set TIM0_Channel3_IRQHandler,Default_Handler

.weak WDT0_IRQHandler
.set WDT0_IRQHandler,Default_Handler

.weak GPIO0_IRQHandler
.set GPIO0_IRQHandler,Default_Handler

.weak PWMG0_IRQHandler
.set PWMG0_IRQHandler,Default_Handler

.weak SPI0_IRQHandler
.set SPI0_IRQHandler,Default_Handler

.weak SPI1_IRQHandler
.set SPI1_IRQHandler,Default_Handler

.weak CANFD0_IRQHandler
.set CANFD0_IRQHandler,Default_Handler

.weak QSPI1_IRQHandler
.set QSPI1_IRQHandler,Default_Handler

.weak UART3_IRQHandler
.set UART3_IRQHandler,Default_Handler

.weak UART4_IRQHandler
.set UART4_IRQHandler,Default_Handler

.weak UART5_IRQHandler
.set UART5_IRQHandler,Default_Handler

.weak I2C2_IRQHandler
.set I2C2_IRQHandler,Default_Handler

.weak I2C3_IRQHandler
.set I2C3_IRQHandler,Default_Handler

.weak I2S1_IRQHandler
.set I2S1_IRQHandler,Default_Handler

.weak TIM1_Channel0_IRQHandler
.set TIM1_Channel0_IRQHandler,Default_Handler

.weak TIM1_Channel1_IRQHandler
.set TIM1_Channel1_IRQHandler,Default_Handler

.weak TIM1_Channel2_IRQHandler
.set TIM1_Channel2_IRQHandler,Default_Handler

.weak TIM1_Channel3_IRQHandler
.set TIM1_Channel3_IRQHandler,Default_Handler

.weak WDT1_IRQHandler
.set WDT1_IRQHandler,Default_Handler

.weak GPIO1_IRQHandler
.set GPIO1_IRQHandler,Default_Handler

.weak PWMG1_IRQHandler
.set PWMG1_IRQHandler,Default_Handler

.weak SPI2_IRQHandler
.set SPI2_IRQHandler,Default_Handler

.weak SPI3_IRQHandler
.set SPI3_IRQHandler,Default_Handler

.weak CANFD1_IRQHandler
.set CANFD1_IRQHandler,Default_Handler

.weak ADC0_IRQHandler
.set ADC0_IRQHandler,Default_Handler

.weak ADC1_IRQHandler
.set ADC1_IRQHandler,Default_Handler

.weak ADC2_IRQHandler
.set ADC2_IRQHandler,Default_Handler

.weak PWMA0_IRQHandler
.set PWMA0_IRQHandler,Default_Handler

.weak QE0_IRQHandler
.set QE0_IRQHandler,Default_Handler

.weak PWMA1_IRQHandler
.set PWMA1_IRQHandler,Default_Handler

.weak QE1_IRQHandler
.set QE1_IRQHandler,Default_Handler

.weak PWMA2_IRQHandler
.set PWMA2_IRQHandler,Default_Handler

.weak QE2_IRQHandler
.set QE2_IRQHandler,Default_Handler

.weak PWMA3_IRQHandler
.set PWMA3_IRQHandler,Default_Handler

.weak QE3_IRQHandler
.set QE3_IRQHandler,Default_Handler

.weak GPIO2_IRQHandler
.set GPIO2_IRQHandler,Default_Handler

.weak UART6_IRQHandler
.set UART6_IRQHandler,Default_Handler

.weak UART7_IRQHandler
.set UART7_IRQHandler,Default_Handler

.weak USB_IRQHandler
.set USB_IRQHandler,Default_Handler

.weak DMA0_IRQHandler
.set DMA0_IRQHandler,Default_Handler

.weak DMA1_IRQHandler
.set DMA1_IRQHandler,Default_Handler

.weak DMA2_IRQHandler
.set DMA2_IRQHandler,Default_Handler

.weak CORE2_IRQHandler
.set CORE2_IRQHandler,Default_Handler

.weak EXTI_PA_IRQHandler
.set EXTI_PA_IRQHandler,Default_Handler

.weak EXTI_PB_IRQHandler
.set EXTI_PB_IRQHandler,Default_Handler

.weak EXTI_PC_IRQHandler
.set EXTI_PC_IRQHandler,Default_Handler

.weak CLKI_Mon_IRQHandler
.set CLKI_Mon_IRQHandler,Default_Handler
