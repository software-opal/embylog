#include <embylog.h>

#define EMBYLOG_ESCAPE_SYMBOL 0x42
#define EMBYLOG_START_OF_FRAME 0x00
#define EMBYLOG_START_OF_SYSTICK 0x01
#define EMBYLOG_START_OF_HEXDUMP 0x02
#define EMBYLOG_START_OF_PROGRAM_COUNTER 0x03

static const uint8_t message_start_symbol[2] = {EMBYLOG_ESCAPE_SYMBOL,
                                                EMBYLOG_START_OF_FRAME};
static const uint8_t message_systick_symbol[2] = {EMBYLOG_ESCAPE_SYMBOL,
                                                  EMBYLOG_START_OF_SYSTICK};
static const uint8_t message_hexdump_symbol[2] = {EMBYLOG_ESCAPE_SYMBOL,
                                                  EMBYLOG_START_OF_HEXDUMP};
static const uint8_t message_program_counter_symbol[2] = {
    EMBYLOG_ESCAPE_SYMBOL, EMBYLOG_START_OF_PROGRAM_COUNTER};
static const uint8_t escape_symbol[2] = {EMBYLOG_ESCAPE_SYMBOL,
                                         EMBYLOG_ESCAPE_SYMBOL + 1};

void embylog_send_byte(uint8_t x) {
  if (x == EMBYLOG_ESCAPE_SYMBOL)
    embylog_transmit_array(escape_symbol, 2);
  else
    embylog_transmit_array(&x, 1);
}
// embylog_gen_type_fn(uint16_t);
embylog_gen_type_fn(uint32_t);
embylog_gen_type_fn(uint64_t);
embylog_gen_type_fn(size_t);
embylog_gen_type_fn(unsigned);
embylog_gen_type_fn(int);

static uint32_t __embylog_last_tick = 0;
__attribute__((weak)) size_t embylog_systick() { return 0; }
void embylog_start_frame(size_t msg_id, size_t pc) {
  if (__embylog_last_tick != embylog_systick()) {
    __embylog_last_tick = embylog_systick();
    embylog_transmit_array(message_systick_symbol, 2);
    embylog_send_size_t(__embylog_last_tick);
  }

  if (pc != 0) {
    embylog_transmit_array(message_program_counter_symbol, 2);
    embylog_send_size_t(pc);
  }

  embylog_transmit_array(message_start_symbol, 2);
  embylog_send_size_t(msg_id);
}

void embylog_internal_transmit_array(const uint8_t *v, const size_t length) {
  for (int i = 0; i < length; i++)
    embylog_send_byte(v[i]);
}

void embylog_send_charptr(const char *value) {
  embylog_internal_transmit_array((uint8_t *)&value, sizeof(value));
}

void embylog_hexdump(const void *value, size_t length) {
  embylog_transmit_array(message_hexdump_symbol, 2);
  embylog_internal_transmit_array((const uint8_t *)value, length);
}
