#pragma once
#include<string>
#include<vector>
#include"Rect.h"

namespace sl
{
    class Font
    {
    public:
        struct Glyph
        {
            RectF rect{};
            float xadvance = 0.0f;
        };
    public:
        Font(Texture* atlas, std::vector<Glyph>&& bakedChars, int lineHeight, int ascent, char firstChar, char lastChar)
            : atlas(atlas), charData(std::move(bakedChars)), lineHeight(lineHeight), ascent(ascent), firstChar(firstChar), lastChar(lastChar) {}

        Texture* GetTextureAtlas() const { return atlas; }
        int GetLineHeight() const { return lineHeight; }
        int GetAscent() const { return ascent; }
        char GetFirstChar() const { return firstChar; }
        char GetLastChar() const { return lastChar; }
        const std::vector<Glyph>& GetCharData() const { return charData; }
    private:
        std::vector<Glyph> charData;
        Texture* atlas;
        int lineHeight = 0;
        int ascent = 0;
        char firstChar;
        char lastChar;
    };
}
