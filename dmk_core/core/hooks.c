// FreeRTOS includes
#include "FreeRTOS.h" /* Must come first. */
#include "task.h"     /* RTOS task related API prototypes. */
#include <stdbool.h>
#include <stdint.h>
#include "hooks.h"


void vApplicationIdleHook(void) {
#if defined(__ARM_ARCH) || defined(__arm__)
    __asm__ volatile("wfi");
#endif
}

#if (configSUPPORT_STATIC_ALLOCATION == 1)
void vApplicationGetIdleTaskMemory(StaticTask_t **ppxIdleTaskTCBBuffer, StackType_t **ppxIdleTaskStackBuffer,
                                   uint32_t *pulIdleTaskStackSize) {
    static StaticTask_t xIdleTaskTCB;
    static StackType_t uxIdleTaskStack[configMINIMAL_STACK_SIZE];

    *ppxIdleTaskTCBBuffer = &xIdleTaskTCB;
    *ppxIdleTaskStackBuffer = uxIdleTaskStack;
    *pulIdleTaskStackSize = configMINIMAL_STACK_SIZE;
}

#if (configUSE_TIMERS == 1)
void vApplicationGetTimerTaskMemory(StaticTask_t **ppxTimerTaskTCBBuffer, StackType_t **ppxTimerTaskStackBuffer,
                                    uint32_t *pulTimerTaskStackSize) {
    static StaticTask_t xTimerTaskTCB;
    static StackType_t uxTimerTaskStack[configTIMER_TASK_STACK_DEPTH];

    *ppxTimerTaskTCBBuffer = &xTimerTaskTCB;
    *ppxTimerTaskStackBuffer = uxTimerTaskStack;
    *pulTimerTaskStackSize = configTIMER_TASK_STACK_DEPTH;
}
#endif
#endif

// Default weak implementations for module hooks
__attribute__((weak)) void hook_early_init(void) {}

__attribute__((weak)) void hook_layer_change(uint8_t active_layer) {
    (void)active_layer;
}

__attribute__((weak)) bool hook_matrix_change(uint8_t row, uint8_t col, bool pressed) {
    (void)row;
    (void)col;
    (void)pressed;
    return false;
}

__attribute__((weak)) bool hook_process_key(uint32_t keycode, bool pressed) {
    (void)keycode;
    (void)pressed;
    return false;
}

__attribute__((weak)) void hook_key_sent(uint16_t keycode, bool pressed) {
    (void)keycode;
    (void)pressed;
}

__attribute__((weak)) void hook_hid_led_change(uint8_t led_mask) {
    (void)led_mask;
}

__attribute__((weak)) bool hook_mouse_move(int8_t *dx, int8_t *dy) {
    (void)dx;
    (void)dy;
    return true;
}

__attribute__((weak)) bool hook_mouse_scroll(int8_t *wheel, int8_t *pan) {
    (void)wheel;
    (void)pan;
    return true;
}

__attribute__((weak)) void hook_mouse_report(uint8_t buttons, int8_t dx, int8_t dy, int8_t wheel, int8_t pan) {
    (void)buttons;
    (void)dx;
    (void)dy;
    (void)wheel;
    (void)pan;
}

__attribute__((weak)) bool hook_gamepad_report(int8_t *x, int8_t *y, int8_t *z, int8_t *rz, int8_t *rx, int8_t *ry,
                                               uint8_t *hat, uint32_t *buttons) {
    (void)x;
    (void)y;
    (void)z;
    (void)rz;
    (void)rx;
    (void)ry;
    (void)hat;
    (void)buttons;
    return true;
}

__attribute__((weak)) void hook_midi_send(const uint8_t *msg, uint8_t len) {
    (void)msg;
    (void)len;
}

__attribute__((weak)) void hook_midi_receive(const uint8_t packet[4]) {
    (void)packet;
}

// A sentinel ensures that the linker creates the subscriber section even when
// no module registers callbacks. GNU ld exposes the bounds of named sections.
DMK_HOOK_SUBSCRIBER(s_hook_sentinel, .early_init = NULL);
extern const dmk_hook_subscriber_t __start_dmk_hooks[];
extern const dmk_hook_subscriber_t __stop_dmk_hooks[];

void hooks_dispatch_early_init(void) {
    hook_early_init();
    for (const dmk_hook_subscriber_t *h = __start_dmk_hooks; h < __stop_dmk_hooks; ++h) {
        if (h->early_init) h->early_init();
    }
}

void hooks_dispatch_layer_change(uint8_t layer) {
    hook_layer_change(layer);
    for (const dmk_hook_subscriber_t *h = __start_dmk_hooks; h < __stop_dmk_hooks; ++h) {
        if (h->layer_change) h->layer_change(layer);
    }
}

bool hooks_dispatch_matrix_change(uint8_t row, uint8_t col, bool pressed) {
    if (hook_matrix_change(row, col, pressed)) return true;
    for (const dmk_hook_subscriber_t *h = __start_dmk_hooks; h < __stop_dmk_hooks; ++h) {
        if (h->matrix_change && h->matrix_change(row, col, pressed)) return true;
    }
    return false;
}

bool hooks_dispatch_process_key(uint32_t key, bool pressed) {
    if (hook_process_key(key, pressed)) return true;
    for (const dmk_hook_subscriber_t *h = __start_dmk_hooks; h < __stop_dmk_hooks; ++h) {
        if (h->process_key && h->process_key(key, pressed)) return true;
    }
    return false;
}

void hooks_dispatch_key_sent(uint16_t key, bool pressed) {
    hook_key_sent(key, pressed);
    for (const dmk_hook_subscriber_t *h = __start_dmk_hooks; h < __stop_dmk_hooks; ++h) {
        if (h->key_sent) h->key_sent(key, pressed);
    }
}

void hooks_dispatch_hid_led_change(uint8_t leds) {
    hook_hid_led_change(leds);
    for (const dmk_hook_subscriber_t *h = __start_dmk_hooks; h < __stop_dmk_hooks; ++h) {
        if (h->hid_led_change) h->hid_led_change(leds);
    }
}

bool hooks_dispatch_mouse_move(int8_t *x, int8_t *y) {
    if (!hook_mouse_move(x, y)) return false;
    for (const dmk_hook_subscriber_t *h = __start_dmk_hooks; h < __stop_dmk_hooks; ++h) {
        if (h->mouse_move && !h->mouse_move(x, y)) return false;
    }
    return true;
}

bool hooks_dispatch_mouse_scroll(int8_t *wheel, int8_t *pan) {
    if (!hook_mouse_scroll(wheel, pan)) return false;
    for (const dmk_hook_subscriber_t *h = __start_dmk_hooks; h < __stop_dmk_hooks; ++h) {
        if (h->mouse_scroll && !h->mouse_scroll(wheel, pan)) return false;
    }
    return true;
}

void hooks_dispatch_mouse_report(uint8_t buttons, int8_t x, int8_t y, int8_t wheel, int8_t pan) {
    hook_mouse_report(buttons, x, y, wheel, pan);
    for (const dmk_hook_subscriber_t *h = __start_dmk_hooks; h < __stop_dmk_hooks; ++h) {
        if (h->mouse_report) h->mouse_report(buttons, x, y, wheel, pan);
    }
}

bool hooks_dispatch_gamepad_report(int8_t *x, int8_t *y, int8_t *z, int8_t *rz, int8_t *rx, int8_t *ry, uint8_t *hat, uint32_t *buttons) {
    if (!hook_gamepad_report(x, y, z, rz, rx, ry, hat, buttons)) return false;
    for (const dmk_hook_subscriber_t *h = __start_dmk_hooks; h < __stop_dmk_hooks; ++h) {
        if (h->gamepad_report && !h->gamepad_report(x, y, z, rz, rx, ry, hat, buttons)) return false;
    }
    return true;
}

void hooks_dispatch_midi_send(const uint8_t *msg, uint8_t len) {
    hook_midi_send(msg, len);
    for (const dmk_hook_subscriber_t *h = __start_dmk_hooks; h < __stop_dmk_hooks; ++h) {
        if (h->midi_send) h->midi_send(msg, len);
    }
}

void hooks_dispatch_midi_receive(const uint8_t *packet) {
    hook_midi_receive(packet);
    for (const dmk_hook_subscriber_t *h = __start_dmk_hooks; h < __stop_dmk_hooks; ++h) {
        if (h->midi_receive) h->midi_receive(packet);
    }
}
