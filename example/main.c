#include "logger.c"
#include "logger.h"
#include <unistd.h>

// Logger support functions

inline __attribute__((weak)) void emby_log_transmit(const uint8_t x) {
  write(STDOUT_FILENO, &x, 1);
}
inline __attribute__((weak)) void emby_log_transmit_array(const uint8_t *v,
                                                          size_t length) {
  write(STDOUT_FILENO, v, length);
}

// MAIN PROGRAM BELOW

#include <stdio.h>
void write_log(uint8_t *bytes, unsigned int length) {
  fwrite(bytes, sizeof(char), length, stdout);
}
int main(int a, char **args) {
  emby_logf("Hello world%d", (uint64_t)0x42024203420142);
  emby_logf("Goodbye crewel world%d", (uint64_t)0x42024203420142);
}