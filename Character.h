#include <gb/gb.h>
#include <stdio.h>

typedef struct Character
{
    // The starting ID of the sprite for this character
    // Remaining sprites should follow in sequency
    uint8_t spriteId;

    // The number of tiles wide this sprite is
    uint8_t spriteTileWidth;

    // The number of tiles high this sprite is
    uint8_t spriteTileHeight;

    // The total number of animated frames
    uint8_t spriteFrames;

    // The current frame the sprite is on
    uint8_t spriteCurrentFrame;

    // The start index where the tiles start for this sprite
    uint8_t tilemapStart;

    // the sprite X position (top left)
    uint8_t x;

    // the sprite Y position (top left)
    uint8_t y;

    // The current speed of travel in the X direction
    uint8_t velocityX;

    // The current speed of travel in the Y direction
    uint8_t velocityY;

    // A pointer to the tilemap
    const unsigned char *tilemap;

    // Color palette
    uint8_t colorPalette;

} Character;

void LoadSpriteFrame(Character *character, uint8_t frame)
{
    // Set current frame
    character->spriteCurrentFrame = frame;

    // Get total sprites
    uint8_t spriteCount = character->spriteTileWidth * character->spriteTileHeight;

    // 

    for (uint8_t i = 0; i != spriteCount; i++)
    {
        // Set sprite tile
        set_sprite_tile(character->spriteId + i, character->tilemap[character->tilemapStart + i + frame * spriteCount]);
    }


}

void MoveCharacter(Character *character, uint8_t x, uint8_t y)
{
    character->x = x;
    character->y = y;

    for (uint8_t iy = 0; iy != character->spriteTileHeight; iy++)
    {
        for (uint8_t ix = 0; ix != character->spriteTileWidth; ix++)
        {
            // Calculate sprite index
            uint8_t index = character->spriteId + ix + (iy * character->spriteTileWidth);

            index = index + 1;
            index -= 1;
            // Move character to screen
            move_sprite(index, x + (ix * 8), y + (iy * 8));
        }
    }
}

void ScrollCharacter(Character *character, uint8_t x, uint8_t y)
{
    character->x += x;
    character->y += y;

    MoveCharacter(character, character->x, character->y);
}

void MoveCharacterWithJoypad(Character *character)
{
    // Read buttons
    uint8_t buttons = joypad();

    uint8_t moveX = 0;
    uint8_t moveY = 0;

    if (buttons & J_LEFT)
    {
        moveX -= 1;
    }
    else if (buttons & J_RIGHT)
    {
        moveX += 1;
    }

    if (buttons & J_UP)
    {
        moveY -= 1;
    }
    else if (buttons & J_DOWN)
    {
        moveY += 1;
    }

    // Scroll Character
    ScrollCharacter(character, moveX, moveY);
}

void SetupCharacter(Character *character, uint8_t spriteId,
    uint8_t tileWidth, uint8_t tileHeight, uint8_t tilemapStart,
    uint8_t totalFrames, const unsigned char *tilemap)
{
    // Set player sprite ID
    character->spriteId = spriteId;

    // Set size
    character->spriteTileWidth = tileWidth;
    character->spriteTileHeight = tileHeight;

    // Set tile start position (in tilemap)
    character->tilemapStart = tilemapStart;

    // Set number of frames
    character->spriteFrames = totalFrames;

    // Store pointer to tilemap
    character->tilemap = tilemap;

    // Load first sprite frame
    LoadSpriteFrame(character, 0);

    // Set color palette (used to invert sprite)
    character->colorPalette = 0;
}

void SetCharacterColorPalette(Character *character, uint8_t colorPalette)
{
    character->colorPalette = colorPalette;

    for (uint8_t iy = 0; iy != character->spriteTileHeight; iy++)
    {
        for (uint8_t ix = 0; ix != character->spriteTileWidth; ix++)
        {
            // Calculate sprite index
            uint8_t index = character->spriteId + ix + (iy * character->spriteTileWidth);

            // invert meta sprite
            if (character->colorPalette)
            {
                set_sprite_prop(index, get_sprite_prop(index) | S_PALETTE);
            }
            else
            {
                set_sprite_prop(index, get_sprite_prop(index) & ~S_PALETTE);
            }
        }
    }
}

void InvertCharacterColorPalette(Character *character)
{
    character->colorPalette = !character->colorPalette;
    SetCharacterColorPalette(character, character->colorPalette);
}