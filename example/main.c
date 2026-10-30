#include <embylog.h>
#include <unistd.h>

// MAIN PROGRAM BELOW

#include <stdio.h>

inline void embylog_transmit_array(const uint8_t *v, size_t length) {
  write(STDOUT_FILENO, v, length);
}

int main(int a, char **args) {
  embylogf("Hello world%d", (uint64_t)0x42024203420142);
  embylogf("Goodbye crewel world%d", (int64_t)0x42024203420142);
}
