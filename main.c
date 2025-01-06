#include <gb/gb.h>
#include "Character.h"
#include "graphics/splashscreen.h"
#include "graphics/splash2.h"
#include "graphics/tilemap.h"
#include <rand.h>

Character pinpad[10];
Character segment[4];
uint8_t pin[4] = {0, 0, 0, 0};
uint8_t pinIndex = 0;
uint8_t key[16];
uint8_t state = 0;

void SetupBackground()
{
    // Load in background tiles
    set_bkg_data(0, splashscreen_tileset_size, splashscreen_tileset);

    // Set background tile map
    set_bkg_tiles(0, 0, 20, 18, splashscreen_tilemap);
}

void SetupBackground2()
{
    set_bkg_data(0, splash2_tileset_size, splash2_tileset);
    set_bkg_tiles(0, 0, 20, 18, splash2_tilemap);
}

void SetupSound()
{
    // these registers must be in this specific order!
    NR52_REG = 0x80; // 1000 0000 turns sound on globally
    NR50_REG = 0x77; // 0111 0111 sets the volume for both left/right channels to max
    NR51_REG = 0xff; // 1111 1111 panning, select all four channels on left/right
}

void PlayTickSound1()
{
    // channel 1 register 0, freq sweep settings
    // 7   unused
    // 6-4 sweep pace
    // 3   sweep increase (0) / decrease (1)
    // 2-0 sweep slope
    // 0 000 0 000
    NR10_REG = 0x00;

    // channel 1 register 1, wave pattern duty cycle and sound length
    // 7-6 wave duty
    // 5-0 initial length timer (the higher the number, the shorter the sound)
    // 10 110111
    NR11_REG = 0xb2;

    // channel 1 register 2, volume envelope
    // 7-4 initial volume (0-F) (0=No Sound)
    // 3   envelope direction (0=decrease, 1=increase)
    // 2-0 sweep pace (0=no sweep)
    // 1111 0 000
    NR12_REG = 0xf0;

    // channel 1 register 3: freq lsb and noise options
    // 7-0 least significant bits of frequency (3 msbs are set in register 4)
    NR13_REG = 0x00;

    // channel 1 register 4: playback and freq msb
    // 7   initialize (trigger channel start)
    // 6   sound length enable (1=stop output when length in NR11 expires)
    // 5-3 unused
    // 2-0 period values higher 3 bits
    // 1 1 000 010
    NR14_REG = 0xc2;
}

void PlayTickSound2()
{
    // channel 1 register 0, freq sweep settings
    // 7   unused
    // 6-4 sweep pace
    // 3   sweep increase (0) / decrease (1)
    // 2-0 sweep slope
    // 0 000 0 000
    NR10_REG = 0x00;

    // channel 1 register 1, wave pattern duty cycle and sound length
    // 7-6 wave duty
    // 5-0 initial length timer (the higher the number, the shorter the sound)
    // 10 110111
    NR11_REG = 0xb2;

    // channel 1 register 2, volume envelope
    // 7-4 initial volume (0-F) (0=No Sound)
    // 3   envelope direction (0=decrease, 1=increase)
    // 2-0 sweep pace (0=no sweep)
    // 1111 0 000
    NR12_REG = 0xf4;

    // channel 1 register 3: freq lsb and noise options
    // 7-0 least significant bits of frequency (3 msbs are set in register 4)
    NR13_REG = 0x01;

    // channel 1 register 4: playback and freq msb
    // 7   initialize (trigger channel start)
    // 6   sound length enable (1=stop output when length in NR11 expires)
    // 5-3 unused
    // 2-0 period values higher 3 bits
    // 1 1 000 010
    NR14_REG = 0xc5;
}

void DrawPin()
{
    for (uint8_t i = 0; i < 4; i++) 
    {
        uint8_t index = (pinIndex + i) % 4;
        uint8_t value = pin[index];

        LoadSpriteFrame(&segment[i], value);
        MoveCharacter(&segment[i], 112 + i*8, 56);
    }
}

uint16_t GetPin()
{
    uint16_t result = 0;
    result += pin[(pinIndex + 0) % 4] * 1000;
    result += pin[(pinIndex + 1) % 4] * 100;
    result += pin[(pinIndex + 2) % 4] * 10;
    result += pin[(pinIndex + 3) % 4] * 1;
    return result;
}

void main () 
{   
    SetupSound();
    SetupBackground();

    // Fill tileset bank 0 with character sprites
    set_sprite_data(0, tileset_size, tileset);

    // setup pinpad sprites
    for (uint8_t i = 0; i < 10; i++)
    {
        SetupCharacter(
            // character
            &pinpad[i],

            // sprite id
            i, //4*i,

            // 2x2 meta-sprite
            1, 1, //2, 2,

            // tiles start at multiples of one
            i, // 1*i,

            // total animated frames
            1,

            // pointer to numbers tilemap
            tilemap
        );

        if (i == 0)
        {
            MoveCharacter(&pinpad[i], 5*8+8, 11*8+16);
        }
        else
        {
            uint8_t offsetX = 3*8 + 8;
            uint8_t ix = ((i - 1) % 3)*16;
            uint8_t offsetY = 5*8 + 16;
            uint8_t iy = (9 - i)/3*16;
            MoveCharacter(&pinpad[i], offsetX + ix, offsetY + iy);
        }
    }

    // setup segment sprites
    for (uint8_t i = 0; i < 4; i++) {
        //SetupCharacter(&segment[i], 2*i + 10, 1, 2, 2*i + 10, 1, segment_tilemap);
        SetupCharacter(
            // character
            &segment[i],

            // sprite id
            10+i*2,

            // tileWidth, tileHeight
            1, 2, // 1x2 meta-sprite

            // tiles start at multiples of two
            10,

            // total animated frames
            10,

            // pointer to numbers tilemap
            tilemap
        );
    }

    DrawPin();

    SHOW_BKG;
    SHOW_SPRITES;
    DISPLAY_ON;


    uint8_t prev_keys;
    uint8_t keys;
    uint8_t count = 0;

    uint8_t cursor = 0;

    while (1) {
        keys = joypad();

        if (keys != prev_keys) {
           
            prev_keys = keys;

            if (state == 0 && (keys & J_LEFT))
            {
                if (cursor > 0)
                {
                    SetCharacterColorPalette(&pinpad[cursor], 0);
                    SetCharacterColorPalette(&pinpad[--cursor], 1);
                    PlayTickSound1();
                }
            }

            if (state == 0 && (keys & J_RIGHT))
            {
                if (cursor < 9)
                {
                    SetCharacterColorPalette(&pinpad[cursor], 0);
                    SetCharacterColorPalette(&pinpad[++cursor], 1);
                    PlayTickSound1();
                }
            }

            if (state == 0 && (keys & J_UP))
            {
                if (cursor < 7)
                {
                    SetCharacterColorPalette(&pinpad[cursor], 0);
                    if (cursor == 0) {
                        cursor += 2;
                    } else {
                        cursor += 3;
                    }
                    SetCharacterColorPalette(&pinpad[cursor], 1);
                    PlayTickSound1();
                }
            }

            if (state == 0 && (keys & J_DOWN)) {
                if (cursor > 0) {
                    SetCharacterColorPalette(&pinpad[cursor], 0);
                    if (cursor > 3) {
                        cursor -= 3;
                    } else {
                        cursor = 0;
                    }
                    SetCharacterColorPalette(&pinpad[cursor], 1);
                    PlayTickSound1();
                }
            }

            if (state == 0 && (keys & (J_A | J_B)))
            {
                pin[pinIndex] = cursor;
                pinIndex = (pinIndex + 1) % 4;
                DrawPin();
                PlayTickSound2();
            }

            if (keys & J_START) 
            {
                if (state == 0) 
                {
                    uint16_t seed = GetPin();
                    initrand(seed);
                    for (uint8_t i = 0; i < 16; i++) {
                        key[i] = rand();
                    }

                    for (uint16_t i = 0; i < 16 * splash2_tileset_size; i++) {
                        splash2_tileset[i] ^= key[i%16];
                    }

                    HIDE_SPRITES;
                    SetupBackground2();
                    state = 1;
                } else
                {
                    for (uint16_t i = 0; i < 16 * splash2_tileset_size; i++) {
                        splash2_tileset[i] ^= key[i%16];
                    }
                    
                    SetupBackground();
                    SHOW_SPRITES;
                    state = 0;
                }
            }
        }

        if (count % 24 == 0) {
            InvertCharacterColorPalette(&pinpad[cursor]);
            count = 0;
        }
        
        count++;
        wait_vbl_done();
    }




}