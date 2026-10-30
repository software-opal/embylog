#pragma once

#include <stdalign.h>
#include <stdarg.h>
#include <stdint.h>
#include <unistd.h>

size_t embylog_systick();

void embylog_transmit_array(const uint8_t *v, size_t length);
void embylog_start_frame(size_t msg_id, size_t pc);

void embylog_internal_transmit_array(const uint8_t *v, const size_t length);

#define embylog_gen_type_fn(TYPE)                                              \
  void embylog_send_##TYPE(TYPE x) {                                           \
    embylog_internal_transmit_array((uint8_t *)&x, sizeof(TYPE));              \
  }

#define embylog_gen_type_fn_dfns(TYPE) void embylog_send_##TYPE(TYPE x);

void embylog_send_charptr(const char *x);

// embylog_gen_type_fn_dfns(uint8_t);
// embylog_gen_type_fn_dfns(uint16_t);
#define embylog_send_uint8_t(X) embylog_send_uint32_t((uint32_t)X)
#define embylog_send_uint16_t(X) embylog_send_uint32_t((uint32_t)X)
embylog_gen_type_fn_dfns(uint32_t);
embylog_gen_type_fn_dfns(uint64_t);
embylog_gen_type_fn_dfns(size_t);
embylog_gen_type_fn_dfns(int);
embylog_gen_type_fn_dfns(long);

#define __embylogf_eval(X) __embylogf_send_size_t(X);

#define __embylogf_send_size_t(X)                                              \
  _Generic((X),                                                                \
      size_t: embylog_send_size_t(X),                                          \
      ssize_t: embylog_send_size_t(X),                                         \
      default: __embylogf_send_int8_to_64(X))

#define __embylogf_send_int8_to_64(X)                                          \
  _Generic((X),                                                                \
      int8_t: embylog_send_uint8_t((uint8_t)X),                                \
      uint8_t: embylog_send_uint8_t((uint8_t)X),                               \
                                                                               \
      int16_t: embylog_send_uint16_t(X),                                       \
      uint16_t: embylog_send_uint16_t(X),                                      \
                                                                               \
      int32_t: embylog_send_uint32_t(X),                                       \
      uint32_t: embylog_send_uint32_t(X),                                      \
                                                                               \
      int64_t: embylog_send_uint64_t(X),                                       \
      uint64_t: embylog_send_uint64_t(X),                                      \
      default: __embylogf_send_other_int_types(X))

#define __embylogf_send_other_int_types(X)                                     \
  _Generic((X),                                                                \
      unsigned int: embylog_send_int(X),                                       \
      int: embylog_send_int(X),                                                \
      unsigned long: embylog_send_long(X),                                     \
      long: embylog_send_long(X),                                              \
      default: __embylogf_send_charptr(X))

#define __embylogf_send_charptr(X)                                             \
  _Generic((X),                                                                \
      char *: embylog_send_charptr,                                            \
      const char *: embylog_send_charptr,                                      \
      default: embylog_send_size_t)(X)

#define EMBYLOGF_EXPAND_1(...) __VA_ARGS__
#define EMBYLOGF_EXPAND_2(...) EMBYLOGF_EXPAND_1(EMBYLOGF_EXPAND_1(__VA_ARGS__))
#define EMBYLOGF_EXPAND_4(...) EMBYLOGF_EXPAND_2(EMBYLOGF_EXPAND_2(__VA_ARGS__))
#define EMBYLOGF_EXPAND_8(...) EMBYLOGF_EXPAND_4(EMBYLOGF_EXPAND_4(__VA_ARGS__))
#define EMBYLOGF_EXPAND_16(...)                                                \
  EMBYLOGF_EXPAND_8(EMBYLOGF_EXPAND_8(__VA_ARGS__))

#define EMBYLOGF_PARENS ()
#define EMBYLOGF_FOR_EACH(macro, ...)                                          \
  __VA_OPT__(EMBYLOGF_EXPAND_16(EMBYLOGF_FOR_EACH_HELPER(macro, __VA_ARGS__)))
#define EMBYLOGF_FOR_EACH_HELPER(macro, a1, ...)                               \
  macro(a1)                                                                    \
      __VA_OPT__(EMBYLOGF_FOR_EACH_AGAIN EMBYLOGF_PARENS(macro, __VA_ARGS__))
#define EMBYLOGF_FOR_EACH_AGAIN() EMBYLOGF_FOR_EACH_HELPER

#if defined(__MACH__)
#define __embylog_msg_t const __attribute__((section("__TEXT,__embylog"))) char
#define __embylog_align(X) 1
#else /* !defined(__MACH__) */
#define __embylog_msg_t const __attribute__((section(".embylog"))) char
#define __embylog_align(X) alignof(X)
#endif

#if defined(__arm__)
#define __embylog_program_counter __current_pc()
#else
#define __embylog_program_counter 0
#endif

#define embylogf(Message, ...)                                                 \
  do {                                                                         \
    static __embylog_msg_t __embylog_msg[] = Message;                          \
    embylog_start_frame(                                                       \
        (((size_t)&__embylog_msg) / __embylog_align(__embylog_msg)),           \
        __embylog_program_counter);                                            \
    EMBYLOGF_FOR_EACH(__embylogf_eval, __VA_ARGS__)                            \
  } while (0)

int embylog_write(char *bytes, unsigned int length);
