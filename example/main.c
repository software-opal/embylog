#include "logger.h"
#include <unistd.h>

inline __attribute__((weak)) void trunk_transmit(const uint8_t x) {
  write(STDOUT_FILENO, &x, 1);
}
inline __attribute__((weak)) void trunk_transmit_array(const uint8_t *v,
                                                       size_t length) {
  write(STDOUT_FILENO, v, length);
}
inline __attribute__((weak)) void trunk_escape_transmit(const uint8_t x) {
  trunk_transmit(TRUNK_ESCAPE_SYMBOL);
  trunk_transmit(x);
}

static const uint8_t message_start_symbol[2] = {TRUNK_ESCAPE_SYMBOL,
                                                TRUNK_START_OF_FRAME};
static const uint8_t escape_symbol[2] = {TRUNK_ESCAPE_SYMBOL,
                                         TRUNK_ESCAPE_SYMBOL + 1};
void __trunk_transmit_array(const uint8_t *v, const size_t length) {
  size_t start = 0;
  for (size_t i = 0; start + i < length; i++) {
    if (v[i] == TRUNK_ESCAPE_SYMBOL) {
      if (i > 0)
        trunk_transmit_array(&v[start], i);
      trunk_transmit_array(escape_symbol, 2);
      start = i + 1;
      i = 0;
    }
  }
  if (start + 1 < length)
    trunk_transmit_array(&v[start], length - start);
}

void trunk_start_frame(size_t msg_id) {
  trunk_transmit_array(message_start_symbol, 2);
  trunkf_send(msg_id);
}

__trunk_signed_type_fn(8);
__trunk_signed_type_fn(16);
__trunk_signed_type_fn(32);
__trunk_signed_type_fn(64);

inline void trunk_send_u8(uint8_t x) {
  if (x == TRUNK_ESCAPE_SYMBOL)
    trunk_transmit_array(escape_symbol, 2);
  else
    trunk_transmit(x);
}
void trunk_send_u16(uint16_t x) {
  // Force transmit in little endian
  __trunk_transmit_array((uint8_t *)&x, 2);
}
void trunk_send_u32(uint32_t x) {
  // Force transmit in little endian
  trunk_send_u8((x >> 0) & 0xFF);
  trunk_send_u8((x >> 8) & 0xFF);
  trunk_send_u8((x >> 16) & 0xFF);
  trunk_send_u8((x >> 24) & 0xFF);
}
inline void trunk_send_u64(uint64_t x) {
  // Force transmit in little endian
  for (int i = 0; i < 64; i += 8)
    trunk_send_u8((x >> i) & 0xFF);
}
inline void trunk_send_size_t(size_t x) {
  for (int i = 0; i < sizeof(x) * 8; i += 8)
    trunk_send_u8((x >> i) & 0xFF);
}

// MAIN PROGRAM BELOW

#include <stdio.h>
void write_log(uint8_t *bytes, unsigned int length) {
  fwrite(bytes, sizeof(char), length, stdout);
}
int main(int a, char **args) {
  trunkf("Hello world%d", (uint64_t)0x42024203420142);
  // trunkf("Goodbye world%d", 1, 12312312);
}