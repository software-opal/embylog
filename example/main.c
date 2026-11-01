#include <embylog.h>
#include <unistd.h>

// MAIN PROGRAM BELOW

#include <stdio.h>

inline void embylog_transmit_array(const uint8_t *v, size_t length) {
  write(STDOUT_FILENO, v, length);
}

int main(int a, char **args) {
  const uint64_t val = 0x42024203420142;
  const int64_t signed_val = 0x42024203420142;
  embylogf("Hello world%d", val);
  embylogf("Goodbye crewel world%d", signed_val);
}
