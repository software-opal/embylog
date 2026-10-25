#ifndef EMBY_LOG_H
#define EMBY_LOG_H

#include <stdarg.h>
#include <stdint.h>
#include <unistd.h>

void emby_log_transmit(uint8_t x);
void emby_log_transmit_array(const uint8_t *v, size_t length);
void emby_log_start_frame(size_t msg_id);

void emby_log_internal_transmit_array(const uint8_t *v, const size_t length);

#define emby_log_gen_type_fn(TYPE)                                             \
  void emby_log_send_##TYPE(TYPE x) {                                          \
    emby_log_internal_transmit_array((uint8_t *)&x, sizeof(TYPE));             \
  }

#define emby_log_gen_type_fn_dfns(TYPE) void emby_log_send_##TYPE(TYPE x);

emby_log_gen_type_fn_dfns(int8_t);
emby_log_gen_type_fn_dfns(uint8_t);
emby_log_gen_type_fn_dfns(int16_t);
emby_log_gen_type_fn_dfns(uint16_t);
emby_log_gen_type_fn_dfns(int32_t);
emby_log_gen_type_fn_dfns(uint32_t);
emby_log_gen_type_fn_dfns(int64_t);
emby_log_gen_type_fn_dfns(uint64_t);
emby_log_gen_type_fn_dfns(ssize_t);
emby_log_gen_type_fn_dfns(size_t);

#define emby_logf_send(X)                                                      \
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
      uint64_t: emby_log_send_uint64_t)(X)

#define EMBY_LOGF_EXPAND_1(...) __VA_ARGS__
#define EMBY_LOGF_EXPAND_2(...)                                                \
  EMBY_LOGF_EXPAND_1(EMBY_LOGF_EXPAND_1(__VA_ARGS__))
#define EMBY_LOGF_EXPAND_4(...)                                                \
  EMBY_LOGF_EXPAND_2(EMBY_LOGF_EXPAND_2(__VA_ARGS__))
#define EMBY_LOGF_EXPAND_8(...)                                                \
  EMBY_LOGF_EXPAND_4(EMBY_LOGF_EXPAND_4(__VA_ARGS__))
#define EMBY_LOGF_EXPAND_16(...)                                               \
  EMBY_LOGF_EXPAND_8(EMBY_LOGF_EXPAND_8(__VA_ARGS__))

#define EMBY_LOGF_PARENS ()
#define EMBY_LOGF_FOR_EACH(macro, ...)                                         \
  __VA_OPT__(EMBY_LOGF_EXPAND_16(EMBY_LOGF_FOR_EACH_HELPER(macro, __VA_ARGS__)))
#define EMBY_LOGF_FOR_EACH_HELPER(macro, a1, ...)                              \
  macro(a1) __VA_OPT__(                                                        \
      EMBY_LOGF_FOR_EACH_AGAIN EMBY_LOGF_PARENS(macro, __VA_ARGS__))
#define EMBY_LOGF_FOR_EACH_AGAIN() EMBY_LOGF_FOR_EACH_HELPER

#define __emby_logf_eval(X) emby_logf_send(X);
#define EMBY_LOG_ESCAPE_SYMBOL 0x42
#define EMBY_LOG_START_OF_FRAME 0x00

#if defined(__MACH__)
#define msg_t static const __attribute__((section("__TEXT,__emby_log"))) char
#else
#define msg_t static const __attribute__((section(".emby_log"))) char
#endif

msg_t MSG_OFFSET[] = "[EnbyLog] Offset";

#define emby_logf(Message, ...)                                                \
  do {                                                                         \
    msg_t __emby_log_msg[] = Message;                                          \
    emby_log_start_frame(((size_t)&__emby_log_msg) - ((size_t)&MSG_OFFSET));   \
    EMBY_LOGF_FOR_EACH(__emby_logf_eval, __VA_ARGS__)                          \
  } while (0)

#endif /* !EMBY_LOG_H */