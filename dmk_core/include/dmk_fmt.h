#ifndef DMK_FMT_H
#define DMK_FMT_H

#include <stdarg.h>
#include <stddef.h>

int dmk_vsnprintf(char *buf, size_t size, const char *fmt, va_list args);
int dmk_snprintf(char *buf, size_t size, const char *fmt, ...) __attribute__((format(printf, 3, 4)));

#endif // DMK_FMT_H
