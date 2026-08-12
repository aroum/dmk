#ifndef TASK_INTERNAL_H
#define TASK_INTERNAL_H

#define TASK_DELAY_DEF 20
#define TASK_DELAY_FAST 5

#define TASK_PRIO_DEF 1

// Tailored task stack sizes (in words, 1 word = 4 bytes on 32-bit Cortex-M)
#define TASK_STACK_LED 128
#define TASK_STACK_MATRIX 256
#define TASK_STACK_KEYBOARD 384
#define TASK_STACK_USB 256
#define TASK_STACK_RGB 512
#define TASK_STACK_DEF 384

#define TASK_PARAMS_DEF NULL
#define TASK_HANDLER_DEF NULL

// Sane queue sizes (saving RAM while preventing dropped keys)
#define QUEUE_DEF_SIZE 16
#define QUEUE_LED_SIZE 8

#define TASK_DEFS TASK_STACK_DEF, TASK_PARAMS_DEF, TASK_PRIO_DEF, TASK_HANDLER_DEF

#endif // TASK_INTERNAL_H
