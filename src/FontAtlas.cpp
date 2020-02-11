#include "FontAtlas.hpp"


FontAtlas::FontAtlas(){
    if (FT_Init_FreeType(&ft)){
        std::cerr << "Freetype error: could not initialize the FreeType" << std::endl;
        log("Freetype error: could not initialize the FreeType");
    }
    m_atlas_size = 512;
    m_atlas = nullptr;
}


FontAtlas::FontAtlas(uint atlas_size){
    if (FT_Init_FreeType(&ft)){
        std::cerr << "Freetype error: could not initialize FreeType" << std::endl;
        log("Freetype error: could not initialize FreeType");
    }
    m_atlas_size = atlas_size;
    m_atlas = nullptr;
}


FontAtlas::~FontAtlas(){
    if(m_atlas != nullptr)
        delete[] m_atlas;
    FT_Done_FreeType(ft);
}


bool comparator(const character& a, const character& b){
    return a.height > b.height;
}



void FontAtlas::loadCharacterRange(std::vector<struct character> &characters, uint start, uint end){
    uint glyph_index, index = 0;

    if(!characters.empty())
        index = characters.size();

    for(uint i=start; i<=end; i++){
        glyph_index = FT_Get_Char_Index(face, i);

        if(!FT_Load_Glyph(face, glyph_index, FT_LOAD_RENDER) && glyph_index){
            characters.push_back(character());
            characters.at(index).code = i;
            characters.at(index).glyph_index = glyph_index;
            characters.at(index).width = face->glyph->bitmap.width;
            characters.at(index).height = face->glyph->bitmap.rows;
            characters.at(index).bearing_x = face->glyph->bitmap_left;
            characters.at(index).bearing_y = face->glyph->bitmap_top;
            characters.at(index).advance_x = face->glyph->advance.x;
            characters.at(index).advance_y = face->glyph->advance.y;
            index++;
        }
    }
}


void FontAtlas::loadCharacter(std::vector<struct character> &characters, uint code, bool load_default){
    uint glyph_index, res, index = 0;

    if(!characters.empty())
        index = characters.size();

    glyph_index = FT_Get_Char_Index(face, code);
    res = FT_Load_Glyph(face, glyph_index, FT_LOAD_RENDER);

    if((!res && glyph_index) || (!res && load_default)){
        characters.push_back(character());
        characters.at(index).code = code;
        characters.at(index).glyph_index = glyph_index;
        characters.at(index).width = face->glyph->bitmap.width;
        characters.at(index).height = face->glyph->bitmap.rows;
        characters.at(index).bearing_x = face->glyph->bitmap_left;
        characters.at(index).bearing_y = face->glyph->bitmap_top;
        characters.at(index).advance_x = face->glyph->advance.x;
        characters.at(index).advance_y = face->glyph->advance.y;
    }
}


int FontAtlas::createAtlas(const char* path, int size, bool save_png){ // check error codes
    uint glyph_index, pen_x = 1, pen_y = 1, max_heigth_row;
    std::vector<struct character> characters;
    m_atlas = new unsigned char[m_atlas_size*m_atlas_size];

    if (FT_New_Face(ft, path, 0, &face)){
        std::cout << "Freetype error: failed to load font " << path << std::endl;
        log("Freetype error: failed to load font ", path);
        return EXIT_FAILURE;
    }
    FT_Set_Pixel_Sizes(face, 0, size);

    m_font_height = face->size->metrics.ascender - face->size->metrics.descender;
    
    loadCharacterRange(characters, 32, 255); // ascii
    loadCharacterRange(characters, 913, 1023); // greek and coptic
    loadCharacter(characters, 0, true); // null character
    
    std::sort(characters.begin(), characters.end(), comparator);

    //let's create the atlas
    std::memset(m_atlas, 0, m_atlas_size*m_atlas_size);

    max_heigth_row = characters[0].height;

    for(uint i=0; i<characters.size(); i++){
        if(pen_x + characters[i].width + 2 > m_atlas_size){
            pen_y += max_heigth_row + 2; // new "row"
            max_heigth_row = characters[i].height;
            pen_x = 0;
            if(pen_y + max_heigth_row + 2 > m_atlas_size){
                std::cerr << "Font atlas: no space left (" << characters.size() - i << " characters left)" << std::endl;
                log("Font atlas: no space left (", characters.size() - i, " characters left)");
                break;
            } // if there's no space left for the next row we quit. naive, but ill work for now        
        }

        characters[i].tex_x_min = (float)pen_x / m_atlas_size;
        characters[i].tex_x_max = (float)(pen_x + characters[i].width) / m_atlas_size;
        characters[i].tex_y_min = (float)pen_y / m_atlas_size;
        characters[i].tex_y_max = (float)(pen_y  + characters[i].height) / m_atlas_size;

        glyph_index = FT_Get_Char_Index(face, characters[i].code);
        FT_Load_Glyph(face, glyph_index, FT_LOAD_RENDER);

        for(int j=0; j<characters[i].height; j++){
            std::copy(&face->glyph->bitmap.buffer[characters[i].width * j],
                      &face->glyph->bitmap.buffer[characters[i].width * (j+1)],
                      m_atlas + pen_y * m_atlas_size + pen_x + m_atlas_size * j);
        }
        pen_x += characters[i].width + 2;
    }

    for(uint i=0; i<characters.size(); i++)
        m_characters[characters[i].code] = characters[i];

    if(save_png)
        if(!stbi_write_png("../data/atlas.png", m_atlas_size, m_atlas_size, 1, m_atlas, m_atlas_size))
            std::cerr << "ooopsie woopsie" << std::endl;

    return EXIT_SUCCESS;
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
    return m_atlas;
}


uint FontAtlas::getAtlasSize() const{
    return m_atlas_size;
}


int FontAtlas::getHeight() const{
    return m_font_height;
}


