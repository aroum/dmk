#ifndef DMK_HEAP_TEST_FREERTOS_H
#define DMK_HEAP_TEST_FREERTOS_H
#include <assert.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
typedef int BaseType_t;
typedef void *TaskHandle_t;
typedef uintptr_t portPOINTER_SIZE_TYPE;
typedef struct {
    size_t xAvailableHeapSpaceInBytes, xSizeOfLargestFreeBlockInBytes, xSizeOfSmallestFreeBlockInBytes;
    size_t xNumberOfFreeBlocks, xMinimumEverFreeBytesRemaining, xNumberOfSuccessfulAllocations, xNumberOfSuccessfulFrees;
} HeapStats_t;
#define configSUPPORT_DYNAMIC_ALLOCATION 1
#ifndef configTOTAL_HEAP_SIZE
#define configTOTAL_HEAP_SIZE 4096
#endif
#define configAPPLICATION_ALLOCATED_HEAP 0
#define configENABLE_HEAP_PROTECTOR 0
#define configUSE_MALLOC_FAILED_HOOK 1
#define INCLUDE_xTaskGetSchedulerState 1
#define taskSCHEDULER_NOT_STARTED 0
#define portMAX_DELAY UINT32_MAX
#define portBYTE_ALIGNMENT 8
#define portBYTE_ALIGNMENT_MASK 7
#define PRIVILEGED_DATA
#define PRIVILEGED_FUNCTION
#define configASSERT(x) assert(x)
#define mtCOVERAGE_TEST_MARKER() ((void)0)
#define traceMALLOC(p, n) ((void)0)
#define traceFREE(p, n) ((void)0)
#define taskENTER_CRITICAL() ((void)0)
#define taskEXIT_CRITICAL() ((void)0)
#define pdFALSE 0
#define pdTRUE 1
void vApplicationMallocFailedHook(void);
#endif
