#ifndef FONT_HPP
#define FONT_HPP

#include <ft2build.h>
#include FT_FREETYPE_H

#include <stb/stb_image_write.h>

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <cstring>
#include <vector>
#include <algorithm>
#include <map>
#include <memory>

#include "common.hpp"
#include "log.hpp"
#include "utils.hpp"
#include "maths_funcs.hpp"


class FontAtlas{ //rename to font atlas?
    private:
        FT_Library ft;
        FT_Face face;

        uint m_atlas_size;
        int m_font_height;
        std::unique_ptr<unsigned char[]> m_atlas;
        std::map<int, struct character> m_characters;
        std::vector<struct character> m_characters_vec;
        GLuint m_texture_id;
    public:
        FontAtlas();
        FontAtlas(uint atlas_size);
        ~FontAtlas();
        int loadFont(const char* path, int size);

        void loadCharacterRange(uint start, uint end);
        void loadCharacter(uint code);
        void createAtlas(bool save_png);

        int getCharacter(uint code, const character** the_character) const;
        int getKerning(uint code1, uint code2) const;
        uint getAtlasSize() const;
        int getHeight() const;
        const unsigned char* getAtlas() const;
        void bindTexture() const;

        // new function to calculate the width of a string?
};


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


#endif
