 #include "Text2D.hpp"



Text2D::Text2D(){
    font = new FontAtlas(512);
    font->createAtlas("/usr/share/fonts/truetype/freefont/FreeSans.ttf", 45, true);
    m_num_vertices = 0;
    m_num_indices = 0;
    m_fb_width = 640;
    m_fb_height = 480;

    initgl(m_fb_width, m_fb_height, color{1.0, 1.0, 1.0});
}


Text2D::Text2D(int fb_width, int fb_height, const color& c, uint atlas_size, const char* font_path, int font_size){
    font = new FontAtlas(atlas_size);
    font->createAtlas(font_path, font_size, true);
    m_num_vertices = 0;
    m_num_indices = 0;
    m_fb_width = fb_width;
    m_fb_height = fb_height;

    initgl(m_fb_width, m_fb_height, c);
}


void Text2D::initgl(int fb_width, int fb_height, const color& c){
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

    m_shader_programme = create_programme_from_files("../shaders/text_vs.glsl",
                                                     "../shaders/text_fs.glsl");
    glUseProgram(m_shader_programme);

    m_projection = math::orthographic(fb_width, 0, fb_height , 0, 1.0f , -1.0f);
    m_proj_mat_location = glGetUniformLocation(m_shader_programme, "projection");
    m_color_location = glGetUniformLocation(m_shader_programme, "text_color");
    glUniform3f(m_color_location, c.r, c.g, c.b);
    glUniformMatrix4fv(m_proj_mat_location, 1, GL_FALSE, m_projection.m);

    glGenTextures(1, &m_texture_id);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, m_texture_id);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, font->getAtlasSize(), font->getAtlasSize(), 0, GL_RED, GL_UNSIGNED_BYTE, font->getAtlas());
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
}


Text2D::~Text2D(){
    glDeleteBuffers(1, &m_vbo_vert);
    glDeleteBuffers(1, &m_vbo_tex);
    glDeleteBuffers(1, &m_vbo_ind);
    glDeleteVertexArrays(1, &m_vao);
    glDeleteTextures(1, &m_texture_id);
    glDeleteShader(m_shader_programme); // doesn't seem to be deleting the shaders or the programme right, it leaks
}


void Text2D::updateBuffers(){
    uint total_num_characters = 0, acc = 0;
    GLfloat* vertex_buffer;
    GLfloat* tex_coords_buffer;
    GLushort* index_buffer;
    for(uint i=0; i < m_strings.size(); i++)
        total_num_characters += m_strings.at(i).strlen;

    m_num_vertices = total_num_characters * 4;
    m_num_indices = total_num_characters * 6;

    vertex_buffer = new GLfloat[2 * m_num_vertices];
    tex_coords_buffer = new GLfloat[2 * m_num_vertices];
    index_buffer = new GLushort[6 * total_num_characters];

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
                pen_y -= (font->getHeight() >> 6) * current_string->scale;
                j++;
                continue;
            }

            font->getCharacter(current_string->textbuffer[j], &ch);

            xpos = pen_x + (float)ch->bearing_x * current_string->scale;
            ypos = pen_y - (float)(ch->height - ch->bearing_y) * current_string->scale;
            w = (float)ch->width * current_string->scale;
            h = (float)ch->height * current_string->scale;

            index = (k + acc) * 8;
            vertex_buffer[index] = xpos;
            vertex_buffer[index + 1] = ypos;
            vertex_buffer[index + 2] = xpos;
            vertex_buffer[index + 3] = ypos + h;
            vertex_buffer[index + 4] = xpos + w;
            vertex_buffer[index + 5] = ypos + h;
            vertex_buffer[index + 6] = xpos + w;
            vertex_buffer[index + 7] = ypos;

            index = (k + acc) * 8;
            tex_coords_buffer[index] = ch->tex_x_min;
            tex_coords_buffer[index + 1] = ch->tex_y_max;
            tex_coords_buffer[index + 2] = ch->tex_x_min;
            tex_coords_buffer[index + 3] = ch->tex_y_min;
            tex_coords_buffer[index + 4] = ch->tex_x_max;
            tex_coords_buffer[index + 5] = ch->tex_y_min;
            tex_coords_buffer[index + 6] = ch->tex_x_max;
            tex_coords_buffer[index + 7] = ch->tex_y_max;

            disp = (k + acc) * 4;
            index = (k + acc) * 6;
            index_buffer[index] = disp;
            index_buffer[index + 1] = disp + 2;
            index_buffer[index + 2] = disp + 1;
            index_buffer[index + 3] = disp;
            index_buffer[index + 4] = disp + 3;
            index_buffer[index + 5] = disp + 2;

            pen_x += (float)(ch->advance_x >> 6) * current_string->scale;

            j++;
            k++;
        }
        acc += current_string->strlen;
    }

    glBindVertexArray(m_vao);

    glBindBuffer(GL_ARRAY_BUFFER, m_vbo_vert);
    glBufferData(GL_ARRAY_BUFFER, 2 * m_num_vertices * sizeof(GLfloat), vertex_buffer, GL_STATIC_DRAW);

    glBindBuffer(GL_ARRAY_BUFFER, m_vbo_tex);
    glBufferData(GL_ARRAY_BUFFER, 2 * m_num_vertices * sizeof(GLfloat), tex_coords_buffer, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_vbo_ind);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, 6 * total_num_characters * sizeof(GLushort), index_buffer, GL_STATIC_DRAW);

    delete[] vertex_buffer;
    delete[] tex_coords_buffer;
    delete[] index_buffer;
}


void Text2D::render(){
    if(m_update_buffer){
        updateBuffers();
        m_update_buffer = false;
    }
    glUseProgram(m_shader_programme);
    glBindVertexArray(m_vao);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, m_texture_id);
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
    glUseProgram(m_shader_programme);
    m_projection = math::orthographic(fb_width, 0, fb_height, 0, 1.0f , -1.0f);
    glUniformMatrix4fv(m_proj_mat_location, 1, GL_FALSE, m_projection.m);
    m_update_buffer = true;
}


void debug_info_box(Text2D** t, int fb_width, int fb_height){
    *t = new Text2D(fb_width, fb_height, color{1.0, 1.0, 0.0}, 256, "../data/fonts/Liberastika-Regular.ttf", 15);
    
    const GLubyte* vendor = glGetString(GL_VENDOR); // Returns the vendor
    const GLubyte* renderer = glGetString(GL_RENDERER); // Returns a hint to the model
    const GLubyte* gl_version = glGetString(GL_VERSION);
    char modelname[64];
    std::ostringstream oss;
    wchar_t vendor_w[128];
    wchar_t renderer_w[128];
    wchar_t glversion_w[128];
    wchar_t modelname_w[64];
    wchar_t totalmemory_w[32];
    unsigned long long mem_bytes;
    float mem_gb;

    wstrcpy(vendor_w, L"Vendor: ", 128);
    wstrcpy(renderer_w, L"Renderer: ", 128);
    wstrcpy(glversion_w, L"GL version: ", 128);

    ucs2wcs(vendor_w+8, vendor, 128-8);
    ucs2wcs(renderer_w+10, renderer, 128-10);
    ucs2wcs(glversion_w+12, gl_version, 128-12);
    
    (*t)->addString(vendor_w, 15, 25, 1, STRING_DRAW_ABSOLUTE_TL);
    (*t)->addString(renderer_w, 15, 45, 1, STRING_DRAW_ABSOLUTE_TL);
    (*t)->addString(glversion_w, 15, 65, 1, STRING_DRAW_ABSOLUTE_TL);

    get_cpu_model(modelname);
    mbstowcs(modelname_w, modelname, 64);
    (*t)->addString(modelname_w+8, 15, 85, 1, STRING_DRAW_ABSOLUTE_TL);

    mem_bytes = get_sys_memory();
    mem_gb = (float)mem_bytes / 0x40000000;
    oss.precision(3);
    oss << "System memory: " << mem_gb << " GB";
    mbstowcs(totalmemory_w, oss.str().c_str(), 64);
    (*t)->addString(totalmemory_w, 15, 105, 1, STRING_DRAW_ABSOLUTE_TL);
}

