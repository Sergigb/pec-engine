#ifndef FONT_HPP
#define FONT_HPP

#include <ft2build.h>
#include FT_FREETYPE_H
#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <vector>
#include <unordered_map>
#include <memory>

#include "../core/common.hpp"


struct character{
    uint code; // unicode value
    uint glyph_index; // freetype index

    int width;             //size x
    int height;            //size y 
    int bearing_x;
    int bearing_y;
    int advance_x;
    int advance_y;

    // normalized texture coordinates
    float tex_x_min;
    float tex_x_max;
    float tex_y_min;
    float tex_y_max;
};

/*
 * Font atlas, where the texture with the atlas is held, along with the different parameters of the
 * characters. Is used by Text2D to render the text.
 */
class FontAtlas{ //rename to font atlas?
    private:
        FT_Library ft;
        FT_Face face;

        uint m_atlas_size;
        int m_font_height;
        std::unique_ptr<unsigned char[]> m_atlas;
        std::unordered_map<int, struct character> m_characters;
        std::vector<struct character> m_characters_vec;
        GLuint m_texture_id;
    public:
        FontAtlas();
        FontAtlas(uint atlas_size);
        ~FontAtlas();
        
        /*
         * Loads a font from a file path.
         * 
         * @path: path to a font file.
         * @size: size of the characters? Is the size used in the function FT_Set_Pixel_Sizes.
         */
        int loadFont(const char* path, int size);

        /*
         * Loads a ASCII character range. If a character is missing from the font, it loads the
         * default character.
         * 
         * @start: start of the range.
         * @end: end of the range.
         */
        void loadCharacterRange(uint start, uint end);

        /*
         * Loads a single ASCII character. If the character is missing from the font, it loads the
         * default character.
         * 
         * @code: ASCII code of the character.
         */
        void loadCharacter(uint code);

        /*
         * Creates the atlas texture from the charactes loaded using loadCharacter and
         * loadCharacter. If there is not enough place in the texture, the next characters are
         * dropped and an error message will be displayed. These charactes will have no texture.
         */
        void createAtlas(bool save_png);

        /*
         * Gets the parameters of a character such as the texture coordinates, check the character
         * struct defined above. The pointer to the character will be returned by the
         * the_character pointer.
         * 
         * @code: ASCII code of the character.
         * @the_character: pointer to pointer that will be modified with a pointer to the character
         * object.
         */
        int getCharacter(uint code, const character** the_character) const;

        /*
         * Returns the kerning between two ASCII characters. Not implemented, so it returns 0.
         * 
         * @code1: ASCII code of the first character.
         * @code2: ASCII code of the second character.
         */
        int getKerning(uint code1, uint code2) const;

        /*
         * Returns the size of the the atlas texture.
         */
        uint getAtlasSize() const;

        /*
         * Gets the height of the font, aka the amount of vertical displacement between the
         * characters of two different lines.
         */
        int getHeight() const;

        /*
         * Returns a pointer to the texture.
         */
        const unsigned char* getAtlas() const;
        
        /*
         * Binds atlas the texture. Gets bound to texture GL_TEXTURE0.
         */
        void bindTexture() const;

        // new function to calculate the width of a string?
};


#endif
