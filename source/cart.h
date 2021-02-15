#pragma once

#include <string>

#include "graphics.h"


struct CartRomData
{
    union
    {
        struct 
        {
            uint8_t SpriteSheetData[128 * 64];
            uint8_t MapData[128 * 32];
            uint8_t SpriteFlagsData[256];
            struct song SongData[64];
            struct sfx SfxData[64];
        };

        uint8_t data[0x4300];
    };
};

class Cart {
    std::string fullCartText;

    void initCartRom();

    void setSpriteSheet(std::string spriteSheetString);
	void setSpriteFlags(std::string spriteFlagsString);
	void setMapData(std::string mapString);

    void setSfx(std::string sfxString);
    void setMusic(std::string musicString);

    bool loadCartFromPng(std::string filename);

    public:
    Cart (std::string filename);
    ~Cart();

    std::string Filename;

    std::string LuaString;

    std::string LoadError;

    std::string SpriteSheetString;
    std::string SpriteFlagsString;
    std::string MapString;
    std::string SfxString;
    std::string MusicString;

    CartRomData CartRom;
    uint8_t CartLuaData[15616];
};
