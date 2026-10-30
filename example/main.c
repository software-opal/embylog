#include "embylog.h"
#include <unistd.h>

// MAIN PROGRAM BELOW

#include <stdio.h>
void write_log(uint8_t *bytes, unsigned int length) {
  fwrite(bytes, sizeof(char), length, stdout);
}
int main(int a, char **args) {
  embylogf("Hello world%d", (uint64_t)0x42024203420142);
  embylogf("Goodbye crewel world%d", (uint64_t)0x42024203420142);
}