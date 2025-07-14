#pragma once
#include<string>
#include<vector>
#include<iostream>

#include"stb/stb_truetype.h"

namespace sl
{
    class Font
    {
    public:
        Font(Texture* atlas, std::vector<stbtt_bakedchar>&& bakedChars, int lineHeight, int ascent, char firstChar, char lastChar)
            : atlas(atlas), charData(std::move(bakedChars)), lineHeight(lineHeight), ascent(ascent), firstChar(firstChar), lastChar(lastChar) {}

        Texture* GetTextureAtlas() const { return atlas; }
        int GetLineHeight() const { return lineHeight; }
        int GetAscent() const { return ascent; }
        char GetFirstChar() const { return firstChar; }
        char GetLastChar() const { return lastChar; }
        const std::vector<stbtt_bakedchar>& GetCharData() const { return charData; }
    private:
        std::vector<stbtt_bakedchar> charData;
        Texture* atlas;
        int lineHeight = 0;
        int ascent = 0;
        char firstChar;
        char lastChar;
    };
}
