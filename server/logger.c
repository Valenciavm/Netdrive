#include <stdio.h>
#include <stdarg.h>

void log_printf(const char *formato, ...) {
    va_list args;

    va_start(args, formato);
    vprintf(formato, args);  // Consola
    va_end(args);

    FILE *f = fopen("log.txt", "a");
    if (f) {
        va_start(args, formato);
        vfprintf(f, formato, args); // Archivo
        va_end(args);
        fclose(f);
    }
}

#define printf(...) log_printf(__VA_ARGS__)
