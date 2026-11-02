#include "fdk/pit.h"
#include "fdk/io.h"

#define FDK_PIT_BASE 1193182

void fdk_pit_init(u32 hz) {
    u32 divisor = FDK_PIT_BASE / hz;
    if (divisor == 0)    divisor = 1;
    if (divisor > 0xFFFF) divisor = 0xFFFF;
    fdk_outb(0x43, 0x36);                       /* channel 0, lo/hi, mode 3 */
    fdk_outb(0x40, (u8)(divisor & 0xFF));
    fdk_outb(0x40, (u8)((divisor >> 8) & 0xFF));
}
