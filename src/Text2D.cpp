#include "Text2D.hpp"
#include "RenderContext.hpp"


Text2D::Text2D(){
    m_init = false;
}


Text2D::Text2D(int fb_width, int fb_height, color& c, const FontAtlas* font, GLuint shader, const RenderContext* render_context){
    m_font_atlas = font;
    m_num_vertices = 0;
    m_num_indices = 0;
    m_fb_width = fb_width;
    m_fb_height = fb_height;
    m_update_buffer = true;
    m_shader_programme = shader;
    m_render_context = render_context;
    m_init = true;
    m_color = c;
    m_disp = math::vec2(0.0, 0.0);

    initgl();
}


void Text2D::initgl(){
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
    m_disp_location = glGetUniformLocation(m_shader_programme, "disp");
}


Text2D::~Text2D(){
    if(m_init){
        glDeleteBuffers(1, &m_vbo_vert);
        glDeleteBuffers(1, &m_vbo_tex);
        glDeleteBuffers(1, &m_vbo_ind);
        glDeleteVertexArrays(1, &m_vao);
    }
}


void Text2D::getPenXY(float& pen_x, float& pen_y, struct string* string_){
    if(string_->placement == STRING_DRAW_RELATIVE){
        pen_x = std::floor(string_->relative_x * m_fb_width);
        pen_y = std::floor(string_->relative_y * m_fb_height);
    }
    else{
        if(string_->placement == STRING_DRAW_ABSOLUTE_BL || string_->placement == STRING_DRAW_ABSOLUTE_TL){
            pen_x = string_->posx;
        }
        else{
            pen_x = m_fb_width - string_->posx;
        }
        if(string_->placement == STRING_DRAW_ABSOLUTE_BL || string_->placement == STRING_DRAW_ABSOLUTE_BR){
            pen_y = string_->posy;
        }
        else{
            pen_y = m_fb_height - string_->posy;
        }
    }

    if(string_->alignment == STRING_ALIGN_CENTER_X || string_->alignment == STRING_ALIGN_CENTER_XY){
        pen_x -= string_->width/2;
    }
    if(string_->alignment == STRING_ALIGN_CENTER_Y || string_->alignment == STRING_ALIGN_CENTER_XY){
        pen_y += (string_->height/2) - ((m_font_atlas->getHeight() >> 6) * string_->scale);
    }

    if(string_->alignment == STRING_ALIGN_LEFT){
        pen_x -= string_->width;
    }
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
        getPenXY(pen_x, pen_y, current_string);
        uint j = 0, k = 0; // k is used to skip the possible line breaks
        while(current_string->textbuffer[j] != '\0'){
            if(current_string->textbuffer[j] == '\n'){
                j++;
                getPenXY(pen_x, pen_y, current_string);
                pen_y -= ((m_font_atlas->getHeight() >> 6) * current_string->scale) * (j - k);
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
    m_render_context->useProgram(m_shader_programme);
    m_render_context->bindVao(m_vao);

    glUniform3f(m_color_location, m_color.r, m_color.g, m_color.b);
    glUniform2fv(m_disp_location, 1, m_disp.v);

    m_font_atlas->bindTexture();
    glDrawElements(GL_TRIANGLES, m_num_indices, GL_UNSIGNED_SHORT, NULL);
}


void Text2D::addString(const wchar_t* string, float relative_x, float relative_y, float scale, int alignment){
    uint i;

    addString(string, 0, 0, scale, STRING_DRAW_RELATIVE, alignment);

    // dirty dirty...
    i = m_strings.size();
    struct string& str = m_strings.at(i-1);
    str.relative_x = relative_x;
    str.relative_y = relative_y;
}


void Text2D::addString(const wchar_t* text, uint x, uint y, float scale, int placement, int alignment){
    const character* ch;
    uint i = m_strings.size(), j = 0, w = 0;

    m_strings.push_back(string());
    struct string& str = m_strings.at(i);

    str.posx = x;
    str.posy = y;
    str.scale = scale;
    str.placement = placement;
    str.alignment = alignment;
    str.width = 0;
    str.height = (m_font_atlas->getHeight() >> 6) * scale;
    wstrcpy(str.textbuffer, text, STRING_MAX_LEN);

    str.strlen = 0;
    while(str.textbuffer[j] != '\0' && j <= STRING_MAX_LEN){
        j++;

        m_font_atlas->getCharacter(str.textbuffer[j], &ch);
        w += (float)(ch->advance_x >> 6) * scale;

        if(str.textbuffer[j] != '\n'){
            str.strlen++;
        }
        else{
            str.height += (m_font_atlas->getHeight() >> 6) * scale;
            if(str.width < w){
                str.width = w;
            }
            w = 0;
        }
    }
    if(str.width < w){
        str.width = w;
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


void Text2D::setDisplacement(const math::vec2& disp){
    m_disp = disp;
}

