#include "logger.h"
#include <stdarg.h>
#include <stdio.h>
#include <unistd.h>

#include "trunk.h"

#include <unistd.h>

const char *REL_STR = "[EmbyLog] Relative to this string";
const size_t REL_STR_PTR = (size_t)&REL_STR;



__attribute__((weak)) int trunk_transmit(const uint8_t x)
{
  return write(STDOUT_FILENO, &x, 1);
}

void trunk_start_frame(uint32_t msg_id)
{
    trunk_transmit(TRUNK_ESCAPE_SYMBOL);
    trunk_transmit(TRUNK_START_OF_FRAME);
    trunk_send_u32(msg_id);
}

void trunk_send_u8(uint8_t x) {
    if (x == TRUNK_ESCAPE_SYMBOL)
        trunk_transmit(x++);
    trunk_transmit(x);
}

void trunk_send_u32(uint32_t x)
{
    // Force transmit in little endian
    trunk_send_u8((x >>  0) & 0xFF);
    trunk_send_u8((x >>  8) & 0xFF);
    trunk_send_u8((x >> 16) & 0xFF);
    trunk_send_u8((x >> 24) & 0xFF);
}



// void embylog_printf(char *format, int count, ...)
//     __attribute__((format(printf, 1, 3))) {
//   size_t ptr = REL_STR_PTR - ((size_t)format);

//   write_log(((uint8_t *)(&ptr)), sizeof(uint8_t *));

//   va_list args;
//   va_start(args, count);

//   for (int i = 0; i < count; ++i)
//     printf("%d", va_arg(args, int));

//   va_end(args);
// }





void write_log(uint8_t *bytes, unsigned int length) {
  fwrite(bytes, sizeof(char), length, stdout);
}



int main(int a, char **args) {
  embylog_printf("Hello world%d", 1, 12312312);
  embylog_printf("Goodbye world%d", 1, 12312312);
}