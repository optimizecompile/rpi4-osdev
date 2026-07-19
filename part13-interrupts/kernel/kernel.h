#define PERIPHERAL_BASE 0xFE000000
#define CLOCKHZ 1000000

struct timer_regs {
    volatile unsigned int control_status;
    volatile unsigned int counter_lo;
    volatile unsigned int counter_hi;
    volatile unsigned int compare[4];
};

#define REGS_TIMER ((struct timer_regs *)(PERIPHERAL_BASE + 0x00003000))

enum timer_cs {
    TIMER_CS_M0 = 1,
    TIMER_CS_M1 = 2,
    TIMER_CS_M2 = 4,
    TIMER_CS_M3 = 8
};

// The GIC-400 interrupt controller. The firmware's boot stub has already
// switched it on (distributor and per-core CPU interfaces enabled, every
// interrupt in group 1, nothing masked by priority) - we just route and
// unblock the interrupts we care about

struct gic400_dist_regs {                     // "GICD_*" in Arm's documentation
    volatile unsigned int ctl;                // 0x000
    volatile unsigned int type;               // 0x004
    volatile unsigned int iid;                // 0x008
    volatile unsigned int res0[29];
    volatile unsigned int group[32];          // 0x080: one bit per interrupt
    volatile unsigned int set_enable[32];     // 0x100: one bit per interrupt, write 1 to enable
    volatile unsigned int clear_enable[32];   // 0x180: one bit per interrupt, write 1 to disable
    volatile unsigned int set_pending[32];    // 0x200
    volatile unsigned int clear_pending[32];  // 0x280
    volatile unsigned int set_active[32];     // 0x300
    volatile unsigned int clear_active[32];   // 0x380
    volatile unsigned char priority[1020];    // 0x400: one byte per interrupt
    volatile unsigned int res1;
    volatile unsigned char target[1020];      // 0x800: one byte per interrupt, one bit per core
};

struct gic400_cpu_regs {                      // "GICC_*" in Arm's documentation
    volatile unsigned int ctl;                // 0x00
    volatile unsigned int priority_mask;      // 0x04
    volatile unsigned int binary_point;       // 0x08
    volatile unsigned int interrupt_ack;      // 0x0c: reading this claims the interrupt...
    volatile unsigned int end_of_interrupt;   // 0x10: ...and writing here completes it
};

#define REGS_GICD ((struct gic400_dist_regs *)0xFF841000)
#define REGS_GICC ((struct gic400_cpu_regs *)0xFF842000)

enum vc_irqs {
    // VideoCore peripheral interrupts arrive at the GIC numbered up from 96
    SYS_TIMER_IRQ_1 = 96 + 1,
    SYS_TIMER_IRQ_3 = 96 + 3
};

void irq_init_vectors();
void irq_enable();
void irq_barrier();
void irq_disable();
void enable_interrupt_controller();
void disable_interrupt_controller();

void handle_timer_1();
void handle_timer_3();
