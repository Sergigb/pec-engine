#include "FontAtlas.hpp"


FontAtlas::FontAtlas(){
    if (FT_Init_FreeType(&ft)){
        std::cerr << "Freetype error: could not initialize the FreeType" << std::endl;
        log("Freetype error: could not initialize the FreeType");
    }
    m_atlas_size = 512;
    m_atlas.reset(nullptr);
}


FontAtlas::FontAtlas(uint atlas_size){
    if (FT_Init_FreeType(&ft)){
        std::cerr << "Freetype error: could not initialize FreeType" << std::endl;
        log("Freetype error: could not initialize FreeType");
    }
    m_atlas_size = atlas_size;
    m_atlas.reset(nullptr);
}


FontAtlas::~FontAtlas(){
    FT_Done_FreeType(ft);
}


int FontAtlas::loadFont(const char* path, int size){
    if (FT_New_Face(ft, path, 0, &face)){
        std::cout << "Freetype error: failed to load font " << path << std::endl;
        log("Freetype error: failed to load font ", path);
        return EXIT_FAILURE;
    }
    FT_Set_Pixel_Sizes(face, 0, size);

    return EXIT_SUCCESS;
}


bool comparator(const character& a, const character& b){
    return a.height > b.height;
}


void FontAtlas::loadCharacterRange(uint start, uint end){
    uint glyph_index, index = 0;

    if(!m_characters_vec.empty())
        index = m_characters_vec.size();

    for(uint i=start; i<=end; i++){
        glyph_index = FT_Get_Char_Index(face, i);

        if(!FT_Load_Glyph(face, glyph_index, FT_LOAD_RENDER) && glyph_index){
            m_characters_vec.push_back(character());
            m_characters_vec.at(index).code = i;
            m_characters_vec.at(index).glyph_index = glyph_index;
            m_characters_vec.at(index).width = face->glyph->bitmap.width;
            m_characters_vec.at(index).height = face->glyph->bitmap.rows;
            m_characters_vec.at(index).bearing_x = face->glyph->bitmap_left;
            m_characters_vec.at(index).bearing_y = face->glyph->bitmap_top;
            m_characters_vec.at(index).advance_x = face->glyph->advance.x;
            m_characters_vec.at(index).advance_y = face->glyph->advance.y;
            index++;
        }
    }
}


void FontAtlas::loadCharacter(uint code, bool load_default){
    uint glyph_index, res, index = 0;

    if(!m_characters_vec.empty())
        index = m_characters_vec.size();

    glyph_index = FT_Get_Char_Index(face, code);
    res = FT_Load_Glyph(face, glyph_index, FT_LOAD_RENDER);

    if((!res && glyph_index) || (!res && load_default)){
        m_characters_vec.push_back(character());
        m_characters_vec.at(index).code = code;
        m_characters_vec.at(index).glyph_index = glyph_index;
        m_characters_vec.at(index).width = face->glyph->bitmap.width;
        m_characters_vec.at(index).height = face->glyph->bitmap.rows;
        m_characters_vec.at(index).bearing_x = face->glyph->bitmap_left;
        m_characters_vec.at(index).bearing_y = face->glyph->bitmap_top;
        m_characters_vec.at(index).advance_x = face->glyph->advance.x;
        m_characters_vec.at(index).advance_y = face->glyph->advance.y;
    }
}


void FontAtlas::createAtlas(bool save_png){ // check error codes
    uint glyph_index, pen_x = 1, pen_y = 1, max_heigth_row;
    m_atlas.reset(new unsigned char[m_atlas_size*m_atlas_size]);

    m_font_height = face->size->metrics.ascender - face->size->metrics.descender;

    std::sort(m_characters_vec.begin(), m_characters_vec.end(), comparator);

    //let's create the atlas
    std::memset(m_atlas.get(), 0, m_atlas_size*m_atlas_size);

    max_heigth_row = m_characters_vec[0].height;

    for(uint i=0; i<m_characters_vec.size(); i++){
        if(pen_x + m_characters_vec[i].width + 2 > m_atlas_size){
            pen_y += max_heigth_row + 2; // new "row"
            max_heigth_row = m_characters_vec[i].height;
            pen_x = 0;
            if(pen_y + max_heigth_row + 2 > m_atlas_size){
                std::cerr << "Font atlas: no space left (" << m_characters_vec.size() - i << " characters left)" << std::endl;
                log("Font atlas: no space left (", m_characters_vec.size() - i, " characters left)");
                break;
            } // if there's no space left for the next row we quit. naive, but ill work for now        
        }

        m_characters_vec[i].tex_x_min = (float)pen_x / m_atlas_size;
        m_characters_vec[i].tex_x_max = (float)(pen_x + m_characters_vec[i].width) / m_atlas_size;
        m_characters_vec[i].tex_y_min = (float)pen_y / m_atlas_size;
        m_characters_vec[i].tex_y_max = (float)(pen_y  + m_characters_vec[i].height) / m_atlas_size;

        glyph_index = FT_Get_Char_Index(face, m_characters_vec[i].code);
        FT_Load_Glyph(face, glyph_index, FT_LOAD_RENDER);

        for(int j=0; j<m_characters_vec[i].height; j++){
            std::copy(&face->glyph->bitmap.buffer[m_characters_vec[i].width * j],
                      &face->glyph->bitmap.buffer[m_characters_vec[i].width * (j+1)],
                      m_atlas.get() + pen_y * m_atlas_size + pen_x + m_atlas_size * j);
        }
        pen_x += m_characters_vec[i].width + 2;
    }

    for(uint i=0; i<m_characters_vec.size(); i++)
        m_characters[m_characters_vec[i].code] = m_characters_vec[i];

    if(save_png)
        if(!stbi_write_png("../data/atlas.png", m_atlas_size, m_atlas_size, 1, m_atlas.get(), m_atlas_size))
            std::cerr << "ooopsie woopsie" << std::endl;
}


int FontAtlas::getCharacter(uint code, const character** the_character) const{
    try{
        *the_character = &m_characters.at(code);
    }
    catch(const std::out_of_range& e){
        // if no character matches it returns the null character.
        *the_character = &m_characters.at(0);
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}


int FontAtlas::getKerning(uint code1, uint code2) const{
    // TODO
    UNUSED(code1);
    UNUSED(code2);
    return 0;
}


const unsigned char* FontAtlas::getAtlas() const{
    return m_atlas.get();
}


uint FontAtlas::getAtlasSize() const{
    return m_atlas_size;
}


int FontAtlas::getHeight() const{
    return m_font_height;
}


