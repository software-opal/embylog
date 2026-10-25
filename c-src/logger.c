#include <stdio.h>

void embylog_printf(char* format,...) {
  write_log(format)
}


int write_log(char* bytes, unsigned int length) {
  write(stdout, bytes, length);
}
