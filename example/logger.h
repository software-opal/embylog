// Based on
// https://blog.gistre.epita.fr/posts/sofiane.meftah-2025-06-15-deferred_logging/

#include <stdint.h>

void write_log(uint8_t *bytes, unsigned int length);

inline void __embylog_send_i8(int8_t x) { write_log((uint8_t *)&x, 1); }
inline void __embylog_send_u8(uint8_t x) { write_log(&x, 1); }
inline void __embylog_send_i32(int32_t x) { write_log((uint8_t *)&x, 4); }
inline void __embylog_send_u32(uint32_t x) { write_log((uint8_t *)&x, 4); }

#define __embylogf_send(X)                                                     \
  _Generic((X),                                                                \
      int: __embylog_send_i32,                                                 \
      int32_t: __embylog_send_132,                                             \
      unsigned int: __embylog_send_u32,                                        \
      uint32_t: __embylog_send_u32,                                            \
                                                                               \
      char: __embylog_send_i8,                                                 \
      int8_t: __embylog_send_i8,                                               \
      unsigned char: __embylog_send_u8,                                        \
      uint8_t: __embylog_send_u8)(X)
