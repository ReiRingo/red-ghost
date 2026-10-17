/*
 * lib.h — Single-header GBA Hardware Utility Library
 * Copyright (c) 2026 Rei Taylor of VolatileDev
 *
 * GBA registers, DMA, OAM, and input helpers.
 */

#ifndef LIB_H
#define LIB_H

// ------------------------------------
// Implementation begins here
// ------------------------------------

/**
 * PS:
 * This is a bespoke / proprietary library for my silly hobby
 * gba project
 * frankly, this my first ever gba library,
 * (referencing Tonc's docs) so some things may
 * be questionable T~T
 */

typedef signed int s32;
typedef unsigned int u32;
typedef signed short s16;
typedef unsigned short u16;
typedef signed char s8;
typedef unsigned char u8;

typedef void (*irq_handler_fn)(void);

#define IRQ_NULL ((void *)0)

#define DISPCNT (*(volatile u16 *)0x04000000)

#define VRAM ((volatile u16 *)0x06000000)
#define VRAM_OBJ ((volatile u16*)0x06010000)
#define PAL_OBJ ((volatile u16 *)0x05000200)

#define OAM_MEM ((volatile struct oam_attr *)0X07000000)
#define OAM_MAT_MEM ((volatile struct oam_matrix *)0x07000000)

#define VCOUNT (*(volatile u16 *)0x04000006)
#define DMA3_SOURCE (*(volatile u32 *)0x040000D4)
#define DMA3_DEST (*(volatile u32 *)0x040000D8)
#define DMA3_CONT (*(volatile u32 *)0x040000DC)

#define KEY_IN (*(volatile u16 *)0x04000130)
#define BLEND_CONT (*(volatile u16 *)0x04000050)
#define LIGHT_COEF (*(volatile u16 *)0x04000054)

#define IE (*(volatile u16 *)0x04000200)
#define IF (*(volatile u16 *)0x04000202)
#define WAITCNT (*(volatile u16 *)0x04000204)
#define IME (*(volatile u16 *)0x04000208)
#define INT_ADDR (*(volatile u32 *)0x03007FFC)

#define DISPSTAT (*(volatile u16 *)0x04000004)
#define INTR_CHECK (*(volatile u16 *)0x03007FF8)

// dma3
enum {
    DMA_ON = 0x80000000,
    DMA_32 = 0x04000000,
    DMA_16 = 0x00000000,
    DMA_FIXED = 0x01000000,
    DMA_NOW = 0,
};

// modes
enum {
    MODE0,
    MODE1,
    MODE2,
    MODE3,
    MODE4,
    MODE5,

// bgs
    BG0 = 256,
    BG1 = 512,
    BG2 = 1024,
    BG3 = 2048,

// object mapping
    OBJ_ON = 4096,
    OBJ_1D = 0x0040,
};

// screen helpers
enum {
    SCREEN_WIDTH = 240,
    SCREEN_HEIGHT = 160,
    SCREEN_PIXELS = SCREEN_WIDTH * SCREEN_HEIGHT,
};

// keys
enum {
    KEY_A = 1,
    KEY_B = 2,
    KEY_L = 512,
    KEY_R = 256,
    KEY_SELECT = 4,
    KEY_START = 8,
    KEY_UP = 128,
    KEY_DOWN = 64,
    KEY_LEFT = 32,
    KEY_RIGHT = 16,
};

enum {
    OBJ_SHAPE_SQUARE = 0,
    OBJ_SHAPE_WIDE = 1,
    OBJ_SHAPE_TALL = 2,
};

enum {
    OBJ_SIZE_0 = 0,
    OBJ_SIZE_1 = 1,
    OBJ_SIZE_2 = 2,
    OBJ_SIZE_3 = 3,
};

enum {
    OBJ_MODE_NORMAL = 0,
    OBJ_MODE_AFFINE = 1 << 8,
    OBJ_MODE_HIDE = 2 << 8,
    OBJ_MODE_AFFINE_EXPANDED = 3 << 8,
};

// ブレンド対象
enum {
    BLEND_BG0 = (1 << 0),
    BLEND_BG1 = (1 << 1),
    BLEND_BG2 = (1 << 2),
    BLEND_BG3 = (1 << 3),
    BLEND_OBJ = (1 << 4),
    BLEND_BD = (1 << 5),
    BLEND_ALL = 0x003F,
};

// ブレンドモード
enum {
    BLEND_MODE_OFF = (0 << 6),
    BLEND_MODE_ALPHA = (1 << 6),
    BLEND_MODE_WHITE = (2 << 6),
    BLEND_MODE_BLACK = (3 << 6),
};

enum {
    IRQ_VBLANK = 1 << 0,
    IRQ_HBLANK = 1 << 1,
    IRQ_VCOUNT = 1 << 2,
    IRQ_TIMER0 = 1 << 3,
    IRQ_TIMER1 = 1 << 4,
    IRQ_TIMER2 = 1 << 5,
    IRQ_TIMER3 = 1 << 6,
    IRQ_SERIAL = 1 << 7,
    IRQ_DMA0 = 1 << 8,
    IRQ_DMA1 = 1 << 9,
    IRQ_DMA2 = 1 << 10,
    IRQ_DMA3 = 1 << 11,
    IRQ_KEYPAD = 1 << 12,
    IRQ_GAMEPAK = 1 << 13,
};

struct oam_attr {
    u16 a0, a1, a2;
    s16 _pad;
} __attribute__((packed, aligned(4)));

struct oam_matrix {
    s16 fill0[3];
    s16 pa;
    s16 fill1[3];
    s16 pb;
    s16 fill2[3];
    s16 pc;
    s16 fill3[3];
    s16 pd;
} __attribute__((packed, aligned(4)));

static inline void dma3_copy(const void *src, void *dst, u32 count, u32 control) {
    DMA3_CONT = 0u; 
    DMA3_SOURCE = (u32)src;
    DMA3_DEST = (u32)dst;
    DMA3_CONT = count | control | DMA_ON;
}

static inline void dma3_fill(const void *src_ptr, void *dst, u32 count, u32 control) {
    DMA3_CONT = 0u; 
    DMA3_SOURCE = (u32)src_ptr;
    DMA3_DEST = (u32)dst;
    DMA3_CONT = count | control | DMA_FIXED | DMA_ON;
}

static inline u16 rgb5(u16 r, u16 g, u16 b) {
    return r | (g << 5u) | (b << 10u);
}

static inline void vblank_wait(void) {
    while (VCOUNT >= 160u);
    while (VCOUNT < 160u);
}

static inline void display_set_mode(u16 flag) {
    DISPCNT = flag;
}

static inline void oam_init(volatile struct oam_attr *so) {
    for (int i = 0; i < 128; i++) {
        so[i].a0 = 160; // 画面外（非表示）
        so[i].a1 = 0;
        so[i].a2 = 0;
        so[i]._pad = 0;
    }
}

static inline void oam_submit_shadow(volatile struct oam_attr *so) {
    dma3_copy((const void *)so, (void *)OAM_MEM, 512, DMA_16);
}

static inline void oam_set_x(volatile struct oam_attr *oam, s16 x) {
    oam->a1 = (oam->a1 & ~0x01FFu) | ((u16)x & 0x01FFu);
}

static inline void oam_set_y(volatile struct oam_attr *oam, s16 y) {
    oam->a0 = (oam->a0 & ~0x00FFu) | ((u16)y & 0x00FFu);
}

static inline s16 oam_get_x(const volatile struct oam_attr *oam) {
    u16 x = oam->a1 & 0x01FFu;
    return (x >= 256u) ? (s16)(x - 512u) : (s16)x;
}

static inline s16 oam_get_y(const volatile struct oam_attr *oam) {
    u16 y = oam->a0 & 0x00FFu;
    return (y >= 160u) ? (s16)(y - 256u) : (s16)y;
}

static inline void oam_set_shape(volatile struct oam_attr *oam, s16 shape) {
    oam->a0 = (oam->a0 & ~(3u << 14u)) | ((u16)shape << 14u);
}

static inline void oam_set_size(volatile struct oam_attr *oam, s16 size) {
    oam->a1 = (oam->a1 & ~(3u << 14u)) | ((u16)size << 14u);
}

static inline void oam_set_mode(volatile struct oam_attr *oam, u16 mode) {
    oam->a0 = (oam->a0 & ~(3u << 8u)) | (mode & (3u << 8u));
}

static inline void oam_set_affine_index(volatile struct oam_attr *oam, u32 index) {
    oam->a0 = (oam->a0 & ~768u) | OBJ_MODE_AFFINE;
    oam->a1 = (oam->a1 & ~(31u << 9)) | ((index & 31u) << 9);
}

static inline void oam_set_matrix(volatile struct oam_matrix *mat_array, u32 index, s16 pa, s16 pb, s16 pc, s16 pd) {
    volatile struct oam_matrix *m = &mat_array[index & 31u];

    m->pa = pa;
    m->pb = pb;
    m->pc = pc;
    m->pd = pd;
}

u16 key_curr = 0;
u16 key_prev = 0;

static inline void key_poll(void) {
    key_prev = key_curr;
    key_curr = (~KEY_IN) & 0x03FF;
}

static inline u16 key_held(u16 key) {
    return key_curr & key;
}

static inline u16 key_pressed(u16 key) {
    return (key_curr & key) & ~(key_prev & key);
}

static inline void fade(s32 config, u16 start, u16 end, u16 time) {
    BLEND_CONT = config;

    if (!time) {
        LIGHT_COEF = end;

        if (!end) {
            BLEND_CONT = BLEND_MODE_OFF;
        }

        return;
    }

    for (u32 t = 0u; t <= time; t += 1u) {
        s32 delta = (s32)end - (s32)start;
        LIGHT_COEF = (u16)((s32)start + delta * (s32)t / time);

        vblank_wait();
    }

    LIGHT_COEF = end;

    if (!end) {
        BLEND_CONT = BLEND_MODE_OFF;
    }
}

extern irq_handler_fn irq_handlers[14];
void irq_user_top(void);
void irq_c_dispatcher(void);

static inline void irq_initialise(void) {
    IME = 0u;
    IE = 0u;
    IF = 0x3FFFu;
    INTR_CHECK = 0u;

    for (u32 i = 0u; i < 14u; i += 1u) {
        irq_handlers[i] = IRQ_NULL;
    }

    INT_ADDR = (u32)irq_user_top;
}

static inline void irq_set_handler(u32 irq_bit, irq_handler_fn handler) {
    if (irq_bit == 0u || (irq_bit & ~0x3FFFu)) {
        return;
    }

    for (u32 i = 0u; i < 14u; i++) {
        if (irq_bit & (1u << i)) {
            irq_handlers[i] = handler;
            IE |= (1u << i);
        }
    }
}

static inline void irq_enable_all(void)  { IME = 1u; }
static inline void irq_disable_all(void) { IME = 0u; }

#ifdef LIB_IMPLEMENTATION

__attribute__((weak)) irq_handler_fn irq_handlers[14];

__attribute__((target("arm"), noinline, weak)) void irq_c_dispatcher(void) {
    u16 pending = IE & IF & 0x3FFFu;
    u16 executed_mask = 0u;

    for (u32 i = 0u; i < 14u; i++) {
        u16 mask = (u16)(1u << i);
        if (pending & mask) {
            if (irq_handlers[i]) {
                irq_handlers[i]();
            }
            executed_mask |= mask;
        }
    }

    IF = executed_mask;
    INTR_CHECK |= executed_mask;
}

__attribute__((target("arm"), naked, weak)) void irq_user_top(void) {
    // この関数に何度苦しめられたことか…マジで頭おかしくなりそう
    __asm__ volatile (
        "mov r3, #0x04000000\n"
        "add r3, r3, #0x200\n"
        "add r3, r3, #8\n"
        "ldr r2, [r3]\n"
        "mov r1, #0\n"
        "strh r1, [r3]\n"
        "stmfd sp!, {r2, lr}\n"
        "bl irq_c_dispatcher\n"
        "ldmfd sp!, {r2, lr}\n"
        "mov r3, #0x04000000\n"
        "add r3, r3, #0x200\n"
        "add r3, r3, #8\n"
        "strh r2, [r3]\n"
        "bx lr\n"
    );
}

#endif // LIB_IMPLEMENTATION

#endif // LIB_H