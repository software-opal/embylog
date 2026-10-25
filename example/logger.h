#ifndef TRUNK_H
#define TRUNK_H

#include <stdarg.h>
#include <stdint.h>
#include <unistd.h>

void trunk_transmit(uint8_t x);
void trunk_escape_transmit(const uint8_t x);
void trunk_start_frame(size_t msg_id);

#define __trunk_uint(BITS) uint##BITS##_t
#define __trunk_int(BITS) int##BITS##_t
#define __trunk_send_ufn(BITS) trunk_send_u##BITS
#define __trunk_send_ifn(BITS) trunk_send_i##BITS

#define __trunk_signed_type_fn(BITS)                                           \
  void __trunk_send_ifn(BITS)(__trunk_int(BITS) x) {                           \
    __trunk_send_ufn(BITS)((__trunk_uint(BITS))x);                             \
  }

#define __trunk_conv_fn_dfns(BITS)                                             \
  void __trunk_send_ufn(BITS)(__trunk_uint(BITS) x);                           \
  void __trunk_send_ifn(BITS)(__trunk_int(BITS) x);

__trunk_conv_fn_dfns(8);
__trunk_conv_fn_dfns(16);
__trunk_conv_fn_dfns(32);
__trunk_conv_fn_dfns(64);
void trunk_send_size_t(size_t x);
void trunk_send_ssize_t(ssize_t x);

#define trunkf_send(X)                                                         \
  _Generic((X),                                                                \
      size_t: trunk_send_size_t,                                               \
      ssize_t: trunk_send_ssize_t,                                             \
      int8_t: trunk_send_i8,                                                   \
      uint8_t: trunk_send_u8,                                                  \
                                                                               \
      int16_t: trunk_send_i16,                                                 \
      uint16_t: trunk_send_u16,                                                \
                                                                               \
      int32_t: trunk_send_i32,                                                 \
      uint32_t: trunk_send_u32,                                                \
                                                                               \
      int64_t: trunk_send_i64,                                                 \
      uint64_t: trunk_send_u64)(X)

#define TRUNKF_EXPAND_1(...) __VA_ARGS__
#define TRUNKF_EXPAND_2(...) TRUNKF_EXPAND_1(TRUNKF_EXPAND_1(__VA_ARGS__))
#define TRUNKF_EXPAND_4(...) TRUNKF_EXPAND_2(TRUNKF_EXPAND_2(__VA_ARGS__))
#define TRUNKF_EXPAND_8(...) TRUNKF_EXPAND_4(TRUNKF_EXPAND_4(__VA_ARGS__))
#define TRUNKF_EXPAND_16(...) TRUNKF_EXPAND_8(TRUNKF_EXPAND_8(__VA_ARGS__))

#define TRUNKF_PARENS ()
#define TRUNKF_FOR_EACH(macro, ...)                                            \
  __VA_OPT__(TRUNKF_EXPAND_16(TRUNKF_FOR_EACH_HELPER(macro, __VA_ARGS__)))
#define TRUNKF_FOR_EACH_HELPER(macro, a1, ...)                                 \
  macro(a1) __VA_OPT__(TRUNKF_FOR_EACH_AGAIN TRUNKF_PARENS(macro, __VA_ARGS__))
#define TRUNKF_FOR_EACH_AGAIN() TRUNKF_FOR_EACH_HELPER

#define __trunkf_eval(X) trunkf_send(X);
#define TRUNK_ESCAPE_SYMBOL 0x42
#define TRUNK_START_OF_FRAME 0x00
#define trunkf(Message, ...)                                                   \
  do {                                                                         \
    /* static __attribute__((section(".trunk"))) const char __trunk_msg[] =    \
     * Message; */                                                             \
    static const char __trunk_msg[] = Message;                                 \
    trunk_start_frame((size_t)&__trunk_msg);                                   \
    TRUNKF_FOR_EACH(__trunkf_eval, __VA_ARGS__)                                \
  } while (0)

#endif /* !TRUNK_H */