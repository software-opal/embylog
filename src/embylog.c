#include <stdio.h>

#include <embylog.h>

static const uint8_t message_start_symbol[2] = {EMBYLOG_ESCAPE_SYMBOL,
                                                EMBYLOG_START_OF_FRAME};
static const uint8_t message_systick_symbol[2] = {EMBYLOG_ESCAPE_SYMBOL,
                                                EMBYLOG_START_OF_SYSTICK};
static const uint8_t escape_symbol[2] = {EMBYLOG_ESCAPE_SYMBOL,
                                         EMBYLOG_ESCAPE_SYMBOL + 1};

msg_t MSG_OFFSET[] = "[EnbyLog] Offset";

void embylog_send_byte(uint8_t x) {
  if (x == EMBYLOG_ESCAPE_SYMBOL)
    embylog_transmit_array(escape_symbol, 2);
  else
    embylog_transmit(x);
}

static uint32_t __embylog_last_tick = 0;

__attribute__((weak)) size_t embylog_systick() {
  return 0;
}

void embylog_start_frame(size_t msg_id) {
    if (__embylog_last_tick != embylog_systick()) {
        __embylog_last_tick =  embylog_systick();
        embylog_transmit_array(message_systick_symbol, 2);
        embylog_send_size_t(__embylog_last_tick);
    }
    
    embylog_transmit_array(message_start_symbol, 2);
    embylog_send_size_t(msg_id);
}

embylog_gen_type_fn(int16_t);
embylog_gen_type_fn(uint16_t);
embylog_gen_type_fn(int32_t);
embylog_gen_type_fn(uint32_t);
embylog_gen_type_fn(int64_t);
embylog_gen_type_fn(uint64_t);
embylog_gen_type_fn(ssize_t);
embylog_gen_type_fn(size_t);
embylog_gen_type_fn(unsigned);
embylog_gen_type_fn(int);

void embylog_internal_transmit_array(const uint8_t *v, const size_t length) {
  for (int i = 0; i < length; i++)
    embylog_send_byte(v[i]);
}
void embylog_send_int8_t(int8_t x) { embylog_send_byte(x); }
void embylog_send_uint8_t(uint8_t x) { embylog_send_byte(x); }

void embylog_send_charptr(const char* x) {
  embylog_internal_transmit_array((uint8_t*)&x, sizeof(x));
  // do {
  //   embylog_transmit(*x);
  // } while(*++x != 0);
}

__attribute__((weak)) void embylog_transmit(const uint8_t x) {
  write(STDOUT_FILENO, &x, 1);
}
__attribute__((weak)) void embylog_transmit_array(const uint8_t *v,
                                                          size_t length) {
  write(STDOUT_FILENO, v, length);
}