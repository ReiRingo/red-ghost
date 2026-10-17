
#define LIB_IMPLEMENTATION
#include "lib.h"
#include <maxmod.h>

#include "soundbank.h"
#include "soundbank_bin.h"

#include "demoSpr.h"
#include "demoSpr2.h"
#include "shared_palette.h"

static struct oam_attr shadow_oam[128];

static inline void pre_logic(void) {
    mmFrame();
    vblank_wait();
    key_poll();
}

int main(void) {
    irq_initialise();
    irq_set_handler(IRQ_VBLANK, mmVBlank);
    DISPSTAT |= (1 << 3);
    irq_enable_all();

    mmInitDefault((mm_addr)soundbank_bin, 8u);
    mmStart(MOD_ISOLATED, MM_PLAY_LOOP);

    oam_init(shadow_oam);

    oam_set_shape(&shadow_oam[0], OBJ_SHAPE_SQUARE);
    oam_set_size(&shadow_oam[0], OBJ_SIZE_2);
    oam_set_x(&shadow_oam[0], 96);
    oam_set_y(&shadow_oam[0], 64);
    shadow_oam[0].a2 = 0;

    oam_set_mode(&shadow_oam[0], OBJ_MODE_HIDE);

    oam_set_shape(&shadow_oam[1], OBJ_SHAPE_SQUARE);
    oam_set_size(&shadow_oam[1], OBJ_SIZE_2);
    oam_set_x(&shadow_oam[1], 128);
    oam_set_y(&shadow_oam[1], 64);
    shadow_oam[1].a2 = 16; // あのー……ToT

    dma3_copy(demoSprTiles, (void *)VRAM_OBJ, demoSprTilesLen >> 1, DMA_16);
    dma3_copy(demoSpr2Tiles, (void *)((u8 *)VRAM_OBJ + demoSprTilesLen), demoSpr2TilesLen >> 1, DMA_16);
    dma3_copy(sharedPal, (void *)PAL_OBJ, sharedPalLen, DMA_16);

    oam_submit_shadow(shadow_oam);

    display_set_mode(MODE0 | BG0 | BG1 | BG2 | BG3 | OBJ_ON | OBJ_1D);

    fade(BLEND_ALL | BLEND_MODE_WHITE, 0, 16, 60);
    oam_set_mode(&shadow_oam[0], OBJ_MODE_NORMAL);
    oam_submit_shadow(shadow_oam);
    fade(BLEND_ALL | BLEND_MODE_WHITE, 16, 0, 120);

    _Bool pause = 0;
    _Bool init = 0;
    s32 scale = 256;
    struct oam_matrix *shadow_mats = (struct oam_matrix *)shadow_oam;

    while (1) {
        pre_logic();

        if (!init) {
            init = 1;
            oam_set_x(&shadow_oam[0], oam_get_x(&shadow_oam[0]) - 16);
            oam_set_y(&shadow_oam[0], oam_get_y(&shadow_oam[0]) - 16);

            oam_set_affine_index(&shadow_oam[0], 0); 
            oam_set_mode(&shadow_oam[0], OBJ_MODE_AFFINE_EXPANDED);
        }

        if (key_held(KEY_LEFT)) {
            oam_set_x(&shadow_oam[0], oam_get_x(&shadow_oam[0]) - 1);
        }
        else if (key_held(KEY_RIGHT)) {
            oam_set_x(&shadow_oam[0], oam_get_x(&shadow_oam[0]) + 1);
        }

        if (key_held(KEY_UP)) {
            scale -= 4;
            if (scale < 64) scale = 64;
        }
        else if (key_held(KEY_DOWN)) {
            scale += 4;
            if (scale > 1024) scale = 1024; 
        }

        oam_set_matrix(shadow_mats, 0, scale, 0, 0, scale);
        
        oam_submit_shadow(shadow_oam);

        if (key_pressed(KEY_B)) {
            pause = 1;
            mmPause();

            while (pause) {
                pre_logic();

                if (key_pressed(KEY_B)) {
                    pause = 0;
                    mmResume();
                }
            }
        }
    }
}
