#pragma once

#include <limits.h>
#include <stdalign.h>
#include <stdarg.h>
#include <stdint.h>
#include <stddef.h>

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
void embylog_hexdump(const void *value, size_t length);

#if SIZE_MAX == INT32_MAX
#define EMBYLOG_PTR_SIZE 32
#else
#define EMBYLOG_PTR_SIZE 64
#endif

// embylog_gen_type_fn_dfns(uint8_t);
// embylog_gen_type_fn_dfns(uint16_t);
#define embylog_send_uint8_t embylog_send_uint32_t
#define embylog_send_uint16_t embylog_send_uint32_t
embylog_gen_type_fn_dfns(uint32_t);
embylog_gen_type_fn_dfns(uint64_t);
embylog_gen_type_fn_dfns(size_t);
embylog_gen_type_fn_dfns(int);
embylog_gen_type_fn_dfns(long);

#define __embylogf_eval(X) embylogf_send(X);
#define embylogf_send(X)                                                      \
  _Generic((X),                                                               \
      char*: embylog_send_charptr,                                            \
      const char*: embylog_send_charptr,                                      \
      uint32_t: embylog_send_uint32_t,                                        \
      uint64_t: embylog_send_uint64_t,                                        \
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
#else /* !defined(__MACH__) */
#define __embylog_msg_t const __attribute__((section(".embylog"))) char
#endif

#define __embylog_align(X) alignof(typeof(X))

#if defined(__arm__)
#define __embylog_program_counter 0
// __current_pc()
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
