#include "FreeRTOS.h" /* Must come first. */
#include "queue.h"
#include "semphr.h"
#include "task.h"

#include "bmcu_common.h"
#include "bmcu_cru.h"
#include "bmcu_gpio.h"
#include "bmcu_uart.h"

#define mainQUEUE_RECEIVE_TASK_PRIORITY ( tskIDLE_PRIORITY + 2 )
#define mainQUEUE_SEND_TASK_PRIORITY ( tskIDLE_PRIORITY + 1 )
#define mainEVENT_SEMAPHORE_TASK_PRIORITY ( configMAX_PRIORITIES - 1 )

#define mainQUEUE_SEND_PERIOD_MS pdMS_TO_TICKS( 200 )

#define mainQUEUE_LENGTH ( 1 )

static void prvSetupHardware( void );

static void prvQueueReceiveTask( void *pvParameters );
static void prvQueueSendTask( void *pvParameters );
static void prvEventSemaphoreTask( void *pvParameters );

static QueueHandle_t xQueue = NULL;
static SemaphoreHandle_t xEventSemaphore = NULL;

static volatile uint32_t ulCountOfItemsReceivedOnQueue = 0;
static volatile uint32_t ulCountOfItemsSentToQueue = 0;
static volatile uint32_t ulCountOfReceivedSemaphores = 0;

/*-----------------------------------------------------------*/

int __io_putchar(int ch)
{
    while ((UART_GetLineStatus(UART0) & UART_LINE_STATUS_THRE) == 0UL);
    UART_TransmitData8b(UART0, (uint8_t)ch);

    return ch;
}

int main(void)
{

    prvSetupHardware();

    xQueue = xQueueCreate( mainQUEUE_LENGTH, sizeof( uint32_t ) );

    /* **NOTE:** A semaphore is used for example purposes,
       using a direct to task notification will be faster
       and use less RAM. */
    xEventSemaphore = xSemaphoreCreateBinary();

    TaskHandle_t xHandle1, xHandle2, xHandle3;
    xTaskCreate( prvQueueReceiveTask,
                 "Rx",
                 configMINIMAL_STACK_SIZE,
                 NULL,
                 mainQUEUE_RECEIVE_TASK_PRIORITY,
                 &xHandle1 );

    vTaskCoreAffinitySet( xHandle1, (1 << 1) );

    xTaskCreate( prvQueueSendTask,
                 "TX",
                  configMINIMAL_STACK_SIZE,
                  NULL,
                  mainQUEUE_SEND_TASK_PRIORITY,
                  &xHandle2 );

    vTaskCoreAffinitySet( xHandle2, (1 << 0) );

    xTaskCreate( prvEventSemaphoreTask,
                 "Sem",
                 configMINIMAL_STACK_SIZE,
                 NULL,
                 mainEVENT_SEMAPHORE_TASK_PRIORITY,
                 &xHandle3 );

    vTaskCoreAffinitySet( xHandle3, (1 << 1) );

    vTaskStartScheduler();

    for( ;; );
}
/*-----------------------------------------------------------*/

static void prvQueueSendTask( void *pvParameters )
{
const uint32_t ulValueToSend = 100UL;

    for( ;; )
    {
        xQueueSend( xQueue, &ulValueToSend, 0 );
        __io_putchar('2');
        ulCountOfItemsSentToQueue++;
        vTaskDelay( mainQUEUE_SEND_PERIOD_MS );
    }
}
/*-----------------------------------------------------------*/

static void prvQueueReceiveTask( void *pvParameters )
{
uint32_t ulReceivedValue;

    for( ;; )
    {
	xQueueReceive( xQueue, &ulReceivedValue, portMAX_DELAY );
	__io_putchar('1');

        if( ulReceivedValue == 100UL )
        {
            ulCountOfItemsReceivedOnQueue++;
        }
    }
}

/*-----------------------------------------------------------*/

static void prvEventSemaphoreTask( void *pvParameters )
{
    for( ;; )
    {
        xSemaphoreTake( xEventSemaphore, portMAX_DELAY );

        /* Count the number of times the semaphore is received. */
        ulCountOfReceivedSemaphores++;
    }
}

/*-----------------------------------------------------------*/

void vApplicationTickHook( void )
{
BaseType_t xHigherPriorityTaskWoken = pdFALSE;
static uint32_t ulCount = 0;

    ulCount++;
    if( ulCount >= 500UL )
    {
        bsp_led_toggle();
           /* xHigherPriorityTaskWoken was initialised to pdFALSE, and will be set to
           pdTRUE by xSemaphoreGiveFromISR() if giving the semaphore unblocked a
           task that has equal or higher priority than the interrupted task. */
        xSemaphoreGiveFromISR( xEventSemaphore, &xHigherPriorityTaskWoken );
        ulCount = 0UL;
    }

 /* If xHigherPriorityTaskWoken is pdTRUE then a context switch should
    normally be performed before leaving the interrupt (because during the
    execution of the interrupt a task of equal or higher priority than the
    running task was unblocked).

    In this case, the function is running in the context of the tick interrupt,
    which will automatically check for the higher priority task to run anyway,
    so no further action is required. */
}

/*-----------------------------------------------------------*/

void vApplicationStackOverflowHook( TaskHandle_t xTask, char *pcTaskName )
{
( void ) pcTaskName;
( void ) xTask;

     for( ;; );
}
/*-----------------------------------------------------------*/

void vAssertCalled( void )
{
    volatile uint32_t ulSetTo1ToExitFunction = 0;

    while( ulSetTo1ToExitFunction != 1 )
    {
        __asm volatile ( "NOP" );
    }
}
/*-----------------------------------------------------------*/

void UART0_Init(void)
{
    /* Enable UART0 clock */
    CRU_APB0_EnableClock(CRU_APB0_PERIPH_UART0);

    /* Enable GPIO0 clock */
    CRU_APB0_EnableClock(CRU_APB0_PERIPH_GPIO0);

    /* Configure alternate function of UART0 pins */
    CRU_SetPinAF(CRU_PORT_A, CRU_PIN_6, CRU_PIN_AF_1);
    CRU_SetPinAF(CRU_PORT_A, CRU_PIN_7, CRU_PIN_AF_1);

    UART_DeInit(UART0);

    UART_InitStruct_TypeDef UART_InitStruct;
    UART_StructInit(&UART_InitStruct);

    UART_InitStruct.BaudRate = 115200UL;
    UART_InitStruct.DataWidth = UART_DATAWIDTH_8B;
    UART_InitStruct.StopBits = UART_STOP_1BIT;
    UART_InitStruct.Parity = UART_PARITY_NONE;
    UART_InitStruct.Transfer9b = UART_TRANSFER_9B_DISABLE;
    UART_InitStruct.CtrlFIFO = DISABLE;

    UART_Init(UART0, &UART_InitStruct);
}

static void prvSetupHardware( void )
{
    bsp_led_init();

    UART0_Init();

    CLIC_Config(1U, 1U);
    CLIC_SetLevelThreshold(0U);

    /* Configure timer interrupt */
    CLIC_ConfigIRQ(CLIC_MT_IRQn,                    /* Interrupt */
                   CLIC_INTATTR_MODE_MACHINE,       /* Privilege mode */
                   1U,                              /* Level */
                   1U,                              /* Priority */
                   CLIC_INTATTR_SHV_VECTORED,       /* Vector mode */
                   CLIC_INTATTR_TRIG_TYPE_LEVEL,    /* Type */
                   CLIC_INTATTR_TRIG_POL_P);        /* Polarity */
}
