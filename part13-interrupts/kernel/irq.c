#include "kernel.h"

void enable_interrupt_controller() {
    // Point our two timer interrupts at core 0, then unblock them in the
    // distributor. Each set_enable word covers 32 interrupts, so interrupt
    // n lives at bit (n % 32) of word (n / 32)
    REGS_GICD->target[SYS_TIMER_IRQ_1] = 1;
    REGS_GICD->target[SYS_TIMER_IRQ_3] = 1;

    REGS_GICD->set_enable[SYS_TIMER_IRQ_1 / 32] = 1 << (SYS_TIMER_IRQ_1 % 32);
    REGS_GICD->set_enable[SYS_TIMER_IRQ_3 / 32] = 1 << (SYS_TIMER_IRQ_3 % 32);
}

void disable_interrupt_controller() {
    REGS_GICD->clear_enable[SYS_TIMER_IRQ_1 / 32] = 1 << (SYS_TIMER_IRQ_1 % 32);
    REGS_GICD->clear_enable[SYS_TIMER_IRQ_3 / 32] = 1 << (SYS_TIMER_IRQ_3 % 32);
}

void handle_irq() {
    unsigned int ack = REGS_GICC->interrupt_ack;   // Claim the interrupt...
    unsigned int irq = ack & 0x3ff;                // ...its number is in the low 10 bits

    while (irq != 1023) {                          // 1023 means "nothing (left) to handle"
        if (irq == SYS_TIMER_IRQ_1) handle_timer_1();
        if (irq == SYS_TIMER_IRQ_3) handle_timer_3();

        REGS_GICC->end_of_interrupt = ack;         // Tell the GIC we're done with this one

        ack = REGS_GICC->interrupt_ack;            // Anything else waiting?
        irq = ack & 0x3ff;
    }
}
