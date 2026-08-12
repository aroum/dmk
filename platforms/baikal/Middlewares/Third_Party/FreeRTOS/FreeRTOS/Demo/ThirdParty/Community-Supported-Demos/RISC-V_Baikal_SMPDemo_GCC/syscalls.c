#include <stdlib.h>
#include <stdint.h>
#include <errno.h>
#include <sys/stat.h>

extern int __io_putchar(int ch) __attribute__((weak));
extern int __io_getchar(void) __attribute__((weak));

/* Close a file. */
int _close(int file)
{
    return -1;
}

/* Status of an open file. */
int _fstat(int file, struct stat *st)
{
	st->st_mode = S_IFCHR;

	return 0;
}

/* Query whether output stream is a terminal. */
int _isatty(int file)
{
    return 1;
}

/* Set position in a file. */
off_t _lseek(int file, off_t ptr, int dir)
{
	return 0;
}

/* Read from a file. */
ssize_t _read(int file, char *ptr, size_t len)
{
    for (int i = 0; i < len; i++)
    {
        *ptr++ = __io_getchar();
    }

    return len;
}

/* Write to a file. */
ssize_t _write(int file, const char *ptr, size_t len)
{
    for (int i = 0; i < len; i++)
    {
        __io_putchar(*ptr++);
    }

    return len;
}

extern char _heap_start[]; /* Start of heap */
char *heap_ptr;

/* Increase program data space. */
char* _sbrk(int nbytes)
{
    char *base;

    if (!heap_ptr)
        heap_ptr = (char*)&_heap_start;

    base = heap_ptr;
    heap_ptr += nbytes;

    return base;
}

/* Exit a program without cleaning up files. */
void _exit(int exit_status)
{
    while (1);
}

/* Send a signal. */
int _kill(int pid, int sig)
{
    errno = EINVAL;

    return -1;
}

/* Get process id. */
int _getpid()
{
    return 1;
}
