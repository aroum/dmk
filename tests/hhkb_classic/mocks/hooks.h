#include <stdbool.h>
#include <stdint.h>
typedef struct { bool (*process_key)(uint32_t,bool); void (*hid_led_change)(uint8_t); } dmk_hook_subscriber_t;
#define DMK_HOOK_SUBSCRIBER(n,...) static const dmk_hook_subscriber_t n __attribute__((used)) = { __VA_ARGS__ }
