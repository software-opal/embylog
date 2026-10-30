#pragma once

#include <stdalign.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <unistd.h>

size_t embylog_systick();

void embylog_transmit(uint8_t x);
void embylog_transmit_array(const uint8_t *v, size_t length);
void embylog_start_frame(size_t msg_id);

void embylog_internal_transmit_array(const uint8_t *v, const size_t length);

#define embylog_gen_type_fn(TYPE)                                              \
  void embylog_send_##TYPE(TYPE x) {                                           \
    embylog_internal_transmit_array((uint8_t *)&x, sizeof(TYPE));              \
  }

#define embylog_gen_type_fn_dfns(TYPE) void embylog_send_##TYPE(TYPE x);

void embylog_send_charptr(const char *x);

embylog_gen_type_fn_dfns(uint32_t);
embylog_gen_type_fn_dfns(uint64_t);
embylog_gen_type_fn_dfns(ssize_t);
embylog_gen_type_fn_dfns(size_t);

#define embylogf_send(X) __embylogf_send_size_t(X)

#define __embylogf_send_size_t(X)                                              \
  _Generic((X),                                                                \
      size_t: emby_log_send_size_t(X),                                         \
      ssize_t: emby_log_send_ssize_t(X),                                       \
      char *: embylog_send_charptr,                                            \
      const char *: embylog_send_charptr,                                      \
      default: __embylogf_send_int8_to_64(X))

#define __embylogf_send_int8_to_64(X)                                          \
  _Generic((X),                                                                \
      int8_t: emby_log_send_int8_t,                                            \
      uint8_t: emby_log_send_uint8_t,                                          \
                                                                               \
      int16_t: emby_log_send_int16_t,                                          \
      uint16_t: emby_log_send_uint16_t,                                        \
                                                                               \
      int32_t: emby_log_send_int32_t,                                          \
      uint32_t: emby_log_send_uint32_t,                                        \
                                                                               \
      int64_t: emby_log_send_int64_t,                                          \
      uint64_t: emby_log_send_uint64_t,                                        \
      default: __embylogf_send_other_int_types)

#define __embylogf_send_other_int_types(X)                                     \
  _Generic((X),                                                                \
      unsigned: emby_log_send_unsigned,                                        \
      int: emby_log_send_int,                                                  \
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

#define __embylogf_eval(X) embylogf_send(X);

#define EMBYLOG_ESCAPE_SYMBOL 0x42
#define EMBYLOG_START_OF_FRAME 0x00
#define EMBYLOG_START_OF_SYSTICK 0x01
#define EMBYLOG_START_OF_HEXDUMP 0x02
#define EMBYLOG_START_OF_PROGRAM_COUNTER 0x02

#if defined(__MACH__)
#define msg_t static const __attribute__((section("__TEXT,__embylog"))) char
#else
#define msg_t __attribute__((section(".embylog"))) const char
#endif

extern msg_t MSG_OFFSET[];

extern size_t __embylog_start;

#define embylogf(Message, ...)                                                 \
  do {                                                                         \
    static msg_t __embylog_msg[] = Message;                                    \
    embylog_start_frame(((size_t)&__embylog_msg) / alignof(&__embylog_msg));   \
    EMBYLOGF_FOR_EACH(__embylogf_eval, __VA_ARGS__)                            \
  } while (0)

int embylog_write(char *bytes, unsigned int length);
