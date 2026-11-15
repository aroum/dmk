#include <sys/stat.h>
#include <errno.h>

int _close(int file) { (void)file; return -1; }
int _fstat(int file, struct stat *st) { (void)file; (void)st; return -1; }
int _isatty(int file) { (void)file; return 1; }
int _lseek(int file, int ptr, int dir) { (void)file; (void)ptr; (void)dir; return 0; }
int _read(int file, char *ptr, int len) { (void)file; (void)ptr; (void)len; return 0; }
int _write(int file, char *ptr, int len) { (void)file; (void)ptr; return len; }
