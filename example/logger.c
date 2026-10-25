#include "logger.h"
#include <stdio.h>

static const uint8_t message_start_symbol[2] = {EMBY_LOG_ESCAPE_SYMBOL,
                                                EMBY_LOG_START_OF_FRAME};
static const uint8_t escape_symbol[2] = {EMBY_LOG_ESCAPE_SYMBOL,
                                         EMBY_LOG_ESCAPE_SYMBOL + 1};

void emby_log_send_byte(uint8_t x) {
  if (x == EMBY_LOG_ESCAPE_SYMBOL)
    emby_log_transmit_array(escape_symbol, 2);
  else
    emby_log_transmit(x);
}

void emby_log_start_frame(size_t msg_id) {
  emby_log_transmit_array(message_start_symbol, 2);
  emby_log_send_size_t(msg_id);
}

emby_log_gen_type_fn(int16_t);
emby_log_gen_type_fn(uint16_t);
emby_log_gen_type_fn(int32_t);
emby_log_gen_type_fn(uint32_t);
emby_log_gen_type_fn(int64_t);
emby_log_gen_type_fn(uint64_t);
emby_log_gen_type_fn(ssize_t);
emby_log_gen_type_fn(size_t);

void emby_log_internal_transmit_array(const uint8_t *v, const size_t length) {
  for (int i = 0; i < length; i++)
    emby_log_send_byte(v[i]);
}
void emby_log_send_int8_t(int8_t x) { emby_log_send_byte(x); }
void emby_log_send_uint8_t(uint8_t x) { emby_log_send_byte(x); }