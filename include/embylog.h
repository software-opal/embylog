#pragma once

#include <limits.h>
#include <stdalign.h>
#include <stdarg.h>
#include <stdint.h>
#include <stddef.h>

#define EMBYLOG_ESCAPE_SYMBOL             0x42
#define EMBYLOG_START_OF_FRAME            0x00
#define EMBYLOG_START_OF_SYSTICK          0x01
#define EMBYLOG_START_OF_HEXDUMP          0x02
#define EMBYLOG_START_OF_PROGRAM_COUNTER  0x03
#define EMBYLOG_START_OF_STAMP            0x04

#define EMBYLOG_LEVEL_NONE    0
#define EMBYLOG_LEVEL_FATAL   1
#define EMBYLOG_LEVEL_ERROR   2
#define EMBYLOG_LEVEL_WARNING 3
#define EMBYLOG_LEVEL_INFO    4
#define EMBYLOG_LEVEL_DEBUG   5
#define EMBYLOG_LEVEL_TRACE   6

/// @brief structure of log sent to host
typedef struct embylog_log_s {
  uint8_t magic[2];
  uint8_t level;
  uint8_t param_count;
  uint16_t message_index;
} embylog_log;

size_t embylog_systick();

void embylog_transmit_array(const uint8_t *v, size_t length);
void embylog_start_frame(const embylog_log *msg_id, size_t pc);

void embylog_internal_transmit_array(const uint8_t *v, const size_t length);

#define embylog_gen_type_fn(TYPE)                                              \
  void embylog_send_##TYPE(TYPE x) {                                           \
    embylog_internal_transmit_array((uint8_t *)&x, sizeof(TYPE));              \
  }

#define embylog_gen_type_fn_dfns(TYPE) void embylog_send_##TYPE(TYPE x);

void embylog_send_charptr(const char *x);
void embylog_hexdump(const void *value, size_t length);

#if SIZE_MAX == UINT32_MAX
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

#define __embylogf_cast(X) (size_t)(X),
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

// Supports 0-10 arguments
#define EMBYLOG_VA_NARGS_IMPL(_0, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, N, ...) N
// ## deletes preceding comma if _VA_ARGS__ is empty (GCC, Clang)
#define EMBYLOG_VA_NARGS(...) EMBYLOG_VA_NARGS_IMPL(_, ## __VA_ARGS__, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0)

#if defined(__MACH__)
#define __embylog_section __attribute__((section("__TEXT,__embylog")))
#else /* !defined(__MACH__) */
#define __embylog_section __attribute__((section(".embylog")))
#endif

#define __embylog_msg_t const __embylog_section char

#define __embylog_align(X) alignof(typeof(X))

#if defined(__arm__)
unsigned int __current_pc(void);
#define __embylog_program_counter __current_pc()
#else
#define __embylog_program_counter 0
#endif

#define embylogf(Message, ...) embylogf_level(EMBYLOG_LEVEL_NONE, Message, __VA_ARGS__)
#define embylogf_level(Level, Message, ...)                                    \
  do {                                                                         \
    static __embylog_msg_t __embylog_msg[] = Message;                          \
    embylog_log log = {                                                        \
        .magic = { EMBYLOG_ESCAPE_SYMBOL, EMBYLOG_START_OF_FRAME },            \
        .level = Level,                                                        \
        .param_count = EMBYLOG_VA_NARGS(__VA_ARGS__),                          \
        .message_index =                                                       \
          ((size_t)&__embylog_msg) / __embylog_align(&__embylog_msg),          \
    };                                                                         \
    size_t params[] = {                                                        \
        EMBYLOGF_FOR_EACH(__embylogf_cast, __VA_ARGS__)                        \
    };                                                                         \
    embylog_start_frame(&log, __embylog_program_counter);                      \
    embylog_transmit_array((const uint8_t *)&params, sizeof(params));          \
  } while (0)

int embylog_write(char *bytes, unsigned int length);
