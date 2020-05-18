 #include "Text2D.hpp"



Text2D::Text2D(){
}


Text2D::Text2D(int fb_width, int fb_height, const color& c, const FontAtlas* font, GLuint shader){
    m_font_atlas = font;
    m_num_vertices = 0;
    m_num_indices = 0;
    m_fb_width = fb_width;
    m_fb_height = fb_height;
    m_update_buffer = true;
    m_shader_programme = shader;

    initgl(c);
}


void Text2D::initgl(const color& c){
    glGenVertexArrays(1, &m_vao);
    glBindVertexArray(m_vao);

    glGenBuffers(1, &m_vbo_vert);
    glBindBuffer(GL_ARRAY_BUFFER, m_vbo_vert);
    glVertexAttribPointer(0, 2,  GL_FLOAT, GL_FALSE, 0, NULL);
    glEnableVertexAttribArray(0);

    glGenBuffers(1, &m_vbo_tex);
    glBindBuffer(GL_ARRAY_BUFFER, m_vbo_tex);
    glVertexAttribPointer(1, 2,  GL_FLOAT, GL_FALSE, 0, NULL);
    glEnableVertexAttribArray(1);

    glGenBuffers(1, &m_vbo_ind);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_vbo_ind);
    glVertexAttribPointer(2, 3, GL_UNSIGNED_SHORT, GL_FALSE, 0, NULL);
    glEnableVertexAttribArray(2);

    glUseProgram(m_shader_programme);

    m_color_location = glGetUniformLocation(m_shader_programme, "text_color");
    glUniform3f(m_color_location, c.r, c.g, c.b);
}


Text2D::~Text2D(){
    glDeleteBuffers(1, &m_vbo_vert);
    glDeleteBuffers(1, &m_vbo_tex);
    glDeleteBuffers(1, &m_vbo_ind);
    glDeleteVertexArrays(1, &m_vao);
}


void Text2D::updateBuffers(){
    uint total_num_characters = 0, acc = 0;
    std::unique_ptr<GLfloat[]> vertex_buffer;
    std::unique_ptr<GLfloat[]> tex_coords_buffer;
    std::unique_ptr<GLushort[]> index_buffer;
    for(uint i=0; i < m_strings.size(); i++)
        total_num_characters += m_strings.at(i).strlen;

    m_num_vertices = total_num_characters * 4;
    m_num_indices = total_num_characters * 6;

    vertex_buffer.reset(new GLfloat[2 * m_num_vertices]);
    tex_coords_buffer.reset(new GLfloat[2 * m_num_vertices]);
    index_buffer.reset(new GLushort[6 * total_num_characters]);

    for(uint i=0; i < m_strings.size(); i++){
        struct string* current_string = &m_strings.at(i);
        float pen_x = current_string->posx, pen_y = current_string->posy, w, h, xpos, ypos;
        uint index, disp;
        const character* ch;

        if(current_string->placement == STRING_DRAW_ABSOLUTE_BL || current_string->placement == STRING_DRAW_ABSOLUTE_TL)
            pen_x = current_string->posx;
        else
            pen_x = m_fb_width - current_string->posx;

        if(current_string->placement == STRING_DRAW_ABSOLUTE_BL || current_string->placement == STRING_DRAW_ABSOLUTE_BR)
            pen_y = current_string->posy;
        else
            pen_y = m_fb_height - current_string->posy;

        uint j = 0, k = 0; // k is used to skip the possible line breaks
        while(current_string->textbuffer[j] != '\0'){
            if(current_string->textbuffer[j] == '\n'){
                pen_x = current_string->posx; // will not work when the string placement is not bl absolute, to be fixed at some point
                pen_y -= (m_font_atlas->getHeight() >> 6) * current_string->scale;
                j++;
                continue;
            }

            m_font_atlas->getCharacter(current_string->textbuffer[j], &ch);

            xpos = pen_x + (float)ch->bearing_x * current_string->scale;
            ypos = pen_y - (float)(ch->height - ch->bearing_y) * current_string->scale;
            w = (float)ch->width * current_string->scale;
            h = (float)ch->height * current_string->scale;

            index = (k + acc) * 8;
            vertex_buffer.get()[index] = xpos;
            vertex_buffer.get()[index + 1] = ypos;
            vertex_buffer.get()[index + 2] = xpos;
            vertex_buffer.get()[index + 3] = ypos + h;
            vertex_buffer.get()[index + 4] = xpos + w;
            vertex_buffer.get()[index + 5] = ypos + h;
            vertex_buffer.get()[index + 6] = xpos + w;
            vertex_buffer.get()[index + 7] = ypos;

            index = (k + acc) * 8;
            tex_coords_buffer.get()[index] = ch->tex_x_min;
            tex_coords_buffer.get()[index + 1] = ch->tex_y_max;
            tex_coords_buffer.get()[index + 2] = ch->tex_x_min;
            tex_coords_buffer.get()[index + 3] = ch->tex_y_min;
            tex_coords_buffer.get()[index + 4] = ch->tex_x_max;
            tex_coords_buffer.get()[index + 5] = ch->tex_y_min;
            tex_coords_buffer.get()[index + 6] = ch->tex_x_max;
            tex_coords_buffer.get()[index + 7] = ch->tex_y_max;

            disp = (k + acc) * 4;
            index = (k + acc) * 6;
            index_buffer.get()[index] = disp;
            index_buffer.get()[index + 1] = disp + 2;
            index_buffer.get()[index + 2] = disp + 1;
            index_buffer.get()[index + 3] = disp;
            index_buffer.get()[index + 4] = disp + 3;
            index_buffer.get()[index + 5] = disp + 2;

            pen_x += (float)(ch->advance_x >> 6) * current_string->scale;

            j++;
            k++;
        }
        acc += current_string->strlen;
    }

    glBindVertexArray(m_vao);

    glBindBuffer(GL_ARRAY_BUFFER, m_vbo_vert);
    glBufferData(GL_ARRAY_BUFFER, 2 * m_num_vertices * sizeof(GLfloat), vertex_buffer.get(), GL_STATIC_DRAW);

    glBindBuffer(GL_ARRAY_BUFFER, m_vbo_tex);
    glBufferData(GL_ARRAY_BUFFER, 2 * m_num_vertices * sizeof(GLfloat), tex_coords_buffer.get(), GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_vbo_ind);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, 6 * total_num_characters * sizeof(GLushort), index_buffer.get(), GL_STATIC_DRAW);
}


void Text2D::render(){
    if(m_update_buffer){
        updateBuffers();
        m_update_buffer = false;
    }
    glUseProgram(m_shader_programme);
    glBindVertexArray(m_vao);
    m_font_atlas->bindTexture();
    glDrawElements(GL_TRIANGLES, m_num_indices, GL_UNSIGNED_SHORT, NULL);
}


void Text2D::addString(const wchar_t* text, uint x, uint y, float scale, int placement){
    uint i = m_strings.size(), j = 0;
    m_strings.push_back(string());
    m_strings.at(i).posx = x;
    m_strings.at(i).posy = y;
    m_strings.at(i).scale = scale;
    m_strings.at(i).placement = placement;
    wstrcpy(m_strings.at(i).textbuffer, text, STRING_MAX_LEN);

    m_strings.at(i).strlen = 0;
    while(m_strings.at(i).textbuffer[j] != '\0' && j <= STRING_MAX_LEN){
        j++;
        if(m_strings.at(i).textbuffer[j] != '\n')
            m_strings.at(i).strlen++;
    }
    m_update_buffer = true;
}


void Text2D::clearStrings(){
    m_strings.clear();
}


void Text2D::onFramebufferSizeUpdate(int fb_width, int fb_height){
    m_fb_width = fb_width;
    m_fb_height = fb_height;
    m_update_buffer = true;
}

