




        //std::cout << face->num_glyphs << std::endl;


    /*FT_UInt index;
    FT_ULong character = FT_Get_First_Char(face, &index);

    while(true){
        character = FT_Get_Next_Char(face, character, &index);
        FT_Load_Glyph(face, character, FT_LOAD_RENDER);
        if (!index) break;
        std::cout << (uint)character << "  " << face->glyph->bitmap.width << "  " << face->glyph->bitmap.rows << std::endl;

        std::string s = std::to_string(character);
        std::string filename = "../data/" + s + ".bmp";
        std::cout << filename << std::endl;

        greyscale_to_bmp(filename.c_str(), face->glyph->bitmap.width, face->glyph->bitmap.rows, face->glyph->bitmap.buffer);
    }*/

    //slot = face->glyph;

    //if(FT_Load_Char(face, u'é', FT_LOAD_RENDER)){
        //check errors
    //}



        std::cout << characters[i].coord_tl.v[0] << "  " << characters[i].coord_tl.v[1]
         << "  " << characters[i].coord_tr.v[0] << "  " << characters[i].coord_tr.v[1]
         << "  " << characters[i].coord_bl.v[0] << "  " << characters[i].coord_bl.v[1]
         << "  " << characters[i].coord_br.v[0] << "  " << characters[i].coord_br.v[1] << std::endl;







////////////////////StringBuffer/////////////////////



StringBuffer::StringBuffer(){
}


StringBuffer::StringBuffer(const wchar_t* string, uint string_len, uint pos_x, uint pos_y, GLfloat scale, const FontAtlas& font, int fb_width, int fb_height, const color& c){
    // will move this to a new function probably or not
    const character* ch;
    uint  index, disp;
    float w, h, pen_x = pos_x, pen_y = pos_y, xpos, ypos;
    GLfloat* vertex_buffer;
    GLfloat* tex_coords_buffer;
    GLushort* index_buffer;

    m_num_vertices = string_len * 4;
    m_num_indices = string_len * 6;

    vertex_buffer = new GLfloat[2 * m_num_vertices];
    tex_coords_buffer = new GLfloat[2 * m_num_vertices];
    index_buffer = new GLushort[6 * string_len];

    // move this into a new function?
    for(uint i=0; i<string_len; i++){
        if(string[i] == '\n'){
            pen_x = pos_x;
            pen_y -= (font.getHeight() >> 6) * scale;
        }

        font.getCharacter(string[i], &ch);

        xpos = pen_x + (float)ch->bearing_x * scale;
        ypos = pen_y - (float)(ch->height - ch->bearing_y) * scale;
        w = (float)ch->width * scale;
        h = (float)ch->height * scale;

        index = i * 8;
        vertex_buffer[index] = xpos;
        vertex_buffer[index + 1] = ypos;
        vertex_buffer[index + 2] = xpos;
        vertex_buffer[index + 3] = ypos + h;
        vertex_buffer[index + 4] = xpos + w;
        vertex_buffer[index + 5] = ypos + h;
        vertex_buffer[index + 6] = xpos + w;
        vertex_buffer[index + 7] = ypos;

        index = i * 8;
        tex_coords_buffer[index] = ch->tex_x_min;
        tex_coords_buffer[index + 1] = ch->tex_y_max;
        tex_coords_buffer[index + 2] = ch->tex_x_min;
        tex_coords_buffer[index + 3] = ch->tex_y_min;
        tex_coords_buffer[index + 4] = ch->tex_x_max;
        tex_coords_buffer[index + 5] = ch->tex_y_min;
        tex_coords_buffer[index + 6] = ch->tex_x_max;
        tex_coords_buffer[index + 7] = ch->tex_y_max;

        disp = i * 4;
        index = i * 6;
        index_buffer[index] = disp;
        index_buffer[index + 1] = disp + 2;
        index_buffer[index + 2] = disp + 1;
        index_buffer[index + 3] = disp;
        index_buffer[index + 4] = disp + 3;
        index_buffer[index + 5] = disp + 2;

        pen_x += (float)(ch->advance_x >> 6) * scale;
    }

    // temporal stuff
    glGenVertexArrays(1, &m_vao);
    glBindVertexArray(m_vao);

    //vertices
    glGenBuffers(1, &m_vbo_vert);
    glBindBuffer(GL_ARRAY_BUFFER, m_vbo_vert);
    glBufferData(GL_ARRAY_BUFFER, 2 * m_num_vertices * sizeof(GLfloat), vertex_buffer, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 2,  GL_FLOAT, GL_FALSE, 0, NULL);
    glEnableVertexAttribArray(0);

    //texture
    glGenBuffers(1, &m_vbo_tex);
    glBindBuffer(GL_ARRAY_BUFFER, m_vbo_tex);
    glBufferData(GL_ARRAY_BUFFER, 2 * m_num_vertices * sizeof(GLfloat), tex_coords_buffer, GL_STATIC_DRAW);
    glVertexAttribPointer(1, 2,  GL_FLOAT, GL_FALSE, 0, NULL);
    glEnableVertexAttribArray(1);

    //indices
    glGenBuffers(1, &m_vbo_ind);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_vbo_ind);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, 6 * string_len * sizeof(GLushort), index_buffer, GL_STATIC_DRAW);
    glVertexAttribPointer(2, 3, GL_UNSIGNED_SHORT, GL_FALSE, 0, NULL);
    glEnableVertexAttribArray(2);

    //hope this works
    m_shader_programme = create_programme_from_files("../shaders/text_vs.glsl",
                                                     "../shaders/text_fs.glsl");
    glUseProgram(m_shader_programme);

    m_projection = math::orthographic(fb_width, 0, fb_height , 0, 1.0f , -1.0f);
    m_proj_mat_location = glGetUniformLocation(m_shader_programme, "projection");
    m_color_location = glGetUniformLocation(m_shader_programme, "text_color");
    glUniform3f(m_color_location, c.r, c.g, c.b);
    glUniformMatrix4fv(m_proj_mat_location, 1, GL_FALSE, m_projection.m);
    
    //set up the texture
    glGenTextures(1, &m_texture_id);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, m_texture_id);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, font.getAtlasSize(), font.getAtlasSize(), 0, GL_RED, GL_UNSIGNED_BYTE, font.getAtlas());
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

    delete[] vertex_buffer;
    delete[] tex_coords_buffer;
    delete[] index_buffer;
}


StringBuffer::~StringBuffer(){
    glDeleteBuffers(1, &m_vbo_vert);
    glDeleteBuffers(1, &m_vbo_tex);
    glDeleteBuffers(1, &m_vbo_ind);
    glDeleteVertexArrays(1, &m_vao);
    glDeleteTextures(1, &m_texture_id);
    glDeleteShader(m_shader_programme); // doesn't seem to be deleting the shaders or the programme right, it leaks
}


void StringBuffer::render(){
    glUseProgram(m_shader_programme);
    glBindVertexArray(m_vao);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, m_texture_id);
    glDrawElements(GL_TRIANGLES, m_num_indices, GL_UNSIGNED_SHORT, NULL);
}


void StringBuffer::onFramebufferSizeUpdate(int width, int height){
    glUseProgram(m_shader_programme);
    m_projection = math::orthographic(width, 0, height, 0, 1.0f , -1.0f);
    glUniformMatrix4fv(m_proj_mat_location, 1, GL_FALSE, m_projection.m);
}



// create separate files?
/*
Builds and holds the buffers that are then copied to the Text2D general buffers. It does not deal with 
whether relative or absolute text position, just pass the coordinates and this class should build all
the necessary buffers.
*/
class StringBuffer{
    private:
        GLuint m_num_vertices, m_num_indices; // we  should have as many indices as tex. coords.

        // temporal stuff
        GLuint m_vao, m_vbo_vert, m_vbo_tex, m_vbo_ind;
        math::mat4 m_projection;
        GLuint m_proj_mat_location, m_shader_programme, m_texture_id, m_color_location;
    public:
        StringBuffer();
        StringBuffer(const wchar_t* string, uint string_len, uint pos_x, uint pos_y, GLfloat scale, const FontAtlas& font, int fb_width, int fb_height, const color& c);
        ~StringBuffer();

        void render(); //temporal function, will move above
        void onFramebufferSizeUpdate(int width, int height); // this is temporal too

        void updateCoords(GLushort pos_x, GLushort pos_y, GLfloat scale, const FontAtlas& font);
};

#endif
