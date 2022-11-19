#include <cstring>

#include <stb/stb_image.h>

#include "PartsPanelGUI.hpp"
#include "StagingPanelGUI.hpp"
#include "EditorGUI.hpp"
#include "../Sprite.hpp"
#include "../FontAtlas.hpp"
#include "../Text2D.hpp"
#include "../../core/RenderContext.hpp"
#include "../../core/Input.hpp"
#include "../../core/log.hpp"


EditorGUI::EditorGUI(){
    m_fb_update = true;
    m_init = false;
}


EditorGUI::EditorGUI(const FontAtlas* atlas, const RenderContext* render_context, const Input* input){
    m_render_context = render_context;
    m_render_context->getDefaultFbSize(m_fb_width, m_fb_height);
    m_fb_update = true;
    m_init = true;
    m_font_atlas = atlas;
    m_input = input;
    m_tab_option = TAB_OPTION_PARTS;
    m_input->getMousePos(m_mouse_y, m_mouse_x);
    m_mouse_y = m_fb_height - m_mouse_y;
    m_symmetric_sides = 1;
    m_max_symmetric_sides = 8;
    m_radial_align = true;

    color c{0.85, 0.85, 0.85};
    m_main_text.reset(new Text2D(m_fb_width, m_fb_height, c, m_font_atlas, render_context));

    m_parts_panel.reset(new PartsPanelGUI(EDITOR_GUI_LP_W - EDITOR_GUI_PP_MARGIN * 2,
                                          m_fb_height - EDITOR_GUI_TP_H - EDITOR_GUI_PP_MARGIN * 2 - EDITOR_GUI_PP_LOW_MARGIN - TAB_HEIGTH,
                                          m_font_atlas, m_render_context, m_input));
    m_staging_panel.reset(new StagingPanelGUI(EDITOR_GUI_LP_W - EDITOR_GUI_PP_MARGIN * 2,
                                              m_fb_height - EDITOR_GUI_TP_H - EDITOR_GUI_PP_MARGIN * 2 - EDITOR_GUI_PP_LOW_MARGIN - TAB_HEIGTH,
                                              m_font_atlas, m_render_context, m_input));
    //m_test_sprite.reset(new Sprite(render_context, math::vec2(350.0f, 350.0f), SPRITE_DRAW_RELATIVE,
    //                               "../data/test_texture.png", 50.0f));
    m_test_sprite.reset(new Sprite(render_context, math::vec2(.5f, .5f), SPRITE_DRAW_RELATIVE,
                                   "../data/test_texture.png", 50.0f));


    m_disp_location = m_render_context->getUniformLocation(SHADER_GUI, "disp");

    // gl init
    glGenVertexArrays(1, &m_vao);
    m_render_context->bindVao(m_vao);

    glGenBuffers(1, &m_vbo_vert);
    glBindBuffer(GL_ARRAY_BUFFER, m_vbo_vert);
    glVertexAttribPointer(0, 2,  GL_FLOAT, GL_FALSE, 0, NULL);
    glEnableVertexAttribArray(0);

    glGenBuffers(1, &m_vbo_clr);
    glBindBuffer(GL_ARRAY_BUFFER, m_vbo_clr);
    glVertexAttribPointer(1, 4,  GL_FLOAT, GL_FALSE, 0, NULL);
    glEnableVertexAttribArray(1);
    // color never changes so there's no reason to change it in the updateBuffers method
    GLfloat gui_color[4 * EDITOR_GUI_VERTEX_NUM] = {EDITOR_GUI_PANEL_COLOR,
                                                    EDITOR_GUI_PANEL_COLOR,
                                                    EDITOR_GUI_PANEL_COLOR,
                                                    EDITOR_GUI_PANEL_COLOR,
                                                    EDITOR_GUI_PANEL_COLOR,
                                                    EDITOR_GUI_PANEL_COLOR,
                                                    EDITOR_GUI_PANEL_COLOR,
                                                    BUTTON_COLOR_DEFAULT,
                                                    BUTTON_COLOR_DEFAULT,
                                                    BUTTON_COLOR_DEFAULT,
                                                    BUTTON_COLOR_DEFAULT,
                                                    BUTTON_COLOR_DEFAULT,
                                                    BUTTON_COLOR_DEFAULT,
                                                    BUTTON_COLOR_DEFAULT,
                                                    BUTTON_COLOR_DEFAULT,
                                                    BUTTON_COLOR_DEFAULT,
                                                    BUTTON_COLOR_DEFAULT,
                                                    BUTTON_COLOR_DEFAULT,
                                                    BUTTON_COLOR_DEFAULT,
                                                    DELETE_AREA_COLOR,
                                                    DELETE_AREA_COLOR,
                                                    DELETE_AREA_COLOR,
                                                    DELETE_AREA_COLOR,
                                                    TAB_COLOR_SELECTED,
                                                    TAB_COLOR_SELECTED,
                                                    TAB_COLOR_SELECTED,
                                                    TAB_COLOR_SELECTED,
                                                    TAB_COLOR_UNSELECTED,
                                                    TAB_COLOR_UNSELECTED,
                                                    TAB_COLOR_UNSELECTED,
                                                    TAB_COLOR_UNSELECTED};

    glBufferData(GL_ARRAY_BUFFER, 4 * EDITOR_GUI_VERTEX_NUM * sizeof(GLfloat), gui_color, GL_STATIC_DRAW);

    glGenBuffers(1, &m_vbo_tex);
    glBindBuffer(GL_ARRAY_BUFFER, m_vbo_tex);
    glVertexAttribPointer(2, 3,  GL_FLOAT, GL_FALSE, 0, NULL);
    glEnableVertexAttribArray(2);

    GLfloat tex_coords[3 * EDITOR_GUI_VERTEX_NUM] = {0.0, 1.0, 0.0,
                                                     0.0, 0.0, 0.0,
                                                     1.0, 1.0, 0.0,
                                                     1.0, 0.0, 0.0,
                                                     0.0, 1.0, 0.0,
                                                     1.0, 1.0, 0.0,
                                                     1.0, 0.0, 0.0,
                                                     0.0, 0.2734, 1.0, // start buttons
                                                     0.0, 0.4101, 1.0,
                                                     0.1367, 0.4101, 1.0,
                                                     0.1367, 0.2734, 1.0,
                                                     0.0, 0.0, 1.0,
                                                     0.0, 0.0, 1.0,
                                                     0.0, 0.0, 1.0,
                                                     0.0, 0.0, 1.0,
                                                     0.4101, 0.4101, 1.0,
                                                     0.4101, 0.5468, 1.0,
                                                     0.5468, 0.5468, 1.0,
                                                     0.5468, 0.4101, 1.0, // end buttons
                                                     0.0, 0.0, 1.0,
                                                     0.0, 0.2734, 1.0,
                                                     0.9375, 0.2734, 1.0,
                                                     0.9375, 0.0, 1.0,
                                                     0.0, 0.0, 0.0,
                                                     0.0, 0.0, 0.0,
                                                     0.0, 0.0, 0.0,
                                                     0.0, 0.0, 0.0,
                                                     0.0, 0.0, 0.0,
                                                     0.0, 0.0, 0.0,
                                                     0.0, 0.0, 0.0,
                                                     0.0, 0.0, 0.0};

    glBufferData(GL_ARRAY_BUFFER, 3 * EDITOR_GUI_VERTEX_NUM * sizeof(GLfloat), tex_coords, GL_STATIC_DRAW);

    glGenBuffers(1, &m_vbo_ind);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_vbo_ind);
    glVertexAttribPointer(3, 3, GL_UNSIGNED_SHORT, GL_FALSE, 0, NULL);
    glEnableVertexAttribArray(3);

    // panels
    GLushort index_buffer[EDITOR_GUI_INDEX_NUM];
    index_buffer[0] = 0;
    index_buffer[1] = 2;
    index_buffer[2] = 1;
    index_buffer[3] = 1;
    index_buffer[4] = 2;
    index_buffer[5] = 3;
    index_buffer[6] = 1;
    index_buffer[7] = 4;
    index_buffer[8] = 5;
    index_buffer[9] = 1;
    index_buffer[10] = 5;
    index_buffer[11] = 6;

    // buttons if I'm not mistaken
    for(char i=0; i < EDITOR_GUI_N_TOP_BUTTONS; i++){
        int disp = i * 4;

        index_buffer[12 + i * 6] = disp + 7;
        index_buffer[13 + i * 6] = disp + 8;
        index_buffer[14 + i * 6] = disp + 9;
        index_buffer[15 + i * 6] = disp + 9;
        index_buffer[16 + i * 6] = disp + 10;
        index_buffer[17 + i * 6] = disp + 7;
    }

    // delete area
    int disp = 17 + (EDITOR_GUI_N_TOP_BUTTONS - 1) * 6;
    index_buffer[disp + 1] = 19;
    index_buffer[disp + 2] = 20;
    index_buffer[disp + 3] = 21;
    index_buffer[disp + 4] = 21;
    index_buffer[disp + 5] = 22;
    index_buffer[disp + 6] = 19;

    disp += 6;

    // panel tabs
    index_buffer[disp + 1] = 23;
    index_buffer[disp + 2] = 24;
    index_buffer[disp + 3] = 25;

    index_buffer[disp + 4] = 24;
    index_buffer[disp + 5] = 26;
    index_buffer[disp + 6] = 25;

    index_buffer[disp + 7] = 27;
    index_buffer[disp + 8] = 28;
    index_buffer[disp + 9] = 29;

    index_buffer[disp + 10] = 28;
    index_buffer[disp + 11] = 30;
    index_buffer[disp + 12] = 29;

    glBufferData(GL_ELEMENT_ARRAY_BUFFER, EDITOR_GUI_INDEX_NUM * sizeof(GLushort), index_buffer, GL_STATIC_DRAW);

    // texture atlas loading test
    int x, y, n;
    unsigned char* image_data = stbi_load("../data/editor_atlas.png", &x, &y, &n, 4);
    if(!image_data) {
        std::cerr << "EditorGUI::EditorGUI - could not load GUI texture atlas" << std::endl;
        log("EditorGUI::EditorGUI - could not load GUI texture atlas");
    }
    else{
        glGenTextures(1, &m_texture_atlas);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, m_texture_atlas);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, x, y, 0, GL_RGBA, GL_UNSIGNED_BYTE, image_data);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    }
    stbi_image_free(image_data);

    // part list panel
    glGenVertexArrays(1, &m_left_panel_vao);
    m_render_context->bindVao(m_left_panel_vao);

    glGenBuffers(1, &m_left_panel_vbo_vert);
    glBindBuffer(GL_ARRAY_BUFFER, m_left_panel_vbo_vert);
    glVertexAttribPointer(0, 2,  GL_FLOAT, GL_FALSE, 0, NULL);
    glEnableVertexAttribArray(0);
    GLfloat parts_panel_vert[12] = {EDITOR_GUI_PP_MARGIN, EDITOR_GUI_PP_MARGIN + EDITOR_GUI_PP_LOW_MARGIN,
                                    EDITOR_GUI_LP_W - EDITOR_GUI_PP_MARGIN, EDITOR_GUI_PP_MARGIN + EDITOR_GUI_PP_LOW_MARGIN,
                                    EDITOR_GUI_PP_MARGIN, m_fb_height - EDITOR_GUI_TP_H - EDITOR_GUI_PP_MARGIN - TAB_HEIGTH,
                                    EDITOR_GUI_LP_W - EDITOR_GUI_PP_MARGIN, EDITOR_GUI_PP_MARGIN + EDITOR_GUI_PP_LOW_MARGIN,
                                    EDITOR_GUI_LP_W - EDITOR_GUI_PP_MARGIN, m_fb_height - EDITOR_GUI_TP_H - EDITOR_GUI_PP_MARGIN - TAB_HEIGTH,
                                    EDITOR_GUI_PP_MARGIN, m_fb_height - EDITOR_GUI_TP_H - EDITOR_GUI_PP_MARGIN - TAB_HEIGTH};
    glBufferData(GL_ARRAY_BUFFER, 12 * sizeof(GLfloat), parts_panel_vert, GL_STATIC_DRAW);

    glGenBuffers(1, &m_left_panel_vbo_clr);
    glBindBuffer(GL_ARRAY_BUFFER, m_left_panel_vbo_clr);
    glVertexAttribPointer(1, 4,  GL_FLOAT, GL_FALSE, 0, NULL);
    glEnableVertexAttribArray(1);
    GLfloat parts_panel_clr[24] = {0.0, 0.0, 0.0, 0.0,
                                   0.0, 0.0, 0.0, 0.0,
                                   0.0, 0.0, 0.0, 0.0,
                                   0.0, 0.0, 0.0, 0.0,
                                   0.0, 0.0, 0.0, 0.0,
                                   0.0, 0.0, 0.0, 0.0};
    glBufferData(GL_ARRAY_BUFFER, 24 * sizeof(GLfloat), parts_panel_clr, GL_STATIC_DRAW);

    glGenBuffers(1, &m_left_panel_vbo_tex);
    glBindBuffer(GL_ARRAY_BUFFER, m_left_panel_vbo_tex);
    glVertexAttribPointer(2, 3,  GL_FLOAT, GL_FALSE, 0, NULL);
    glEnableVertexAttribArray(2);
    GLfloat parts_panel_tex[18] = {0.0, 0.0, 1.0,
                                   1.0, 0.0, 1.0,
                                   0.0, 1.0, 1.0,
                                   1.0, 0.0, 1.0,
                                   1.0, 1.0, 1.0,
                                   0.0, 1.0, 1.0};
    glBufferData(GL_ARRAY_BUFFER, 18 * sizeof(GLfloat), parts_panel_tex, GL_STATIC_DRAW);
}


EditorGUI::~EditorGUI(){
    if(m_init){
        glDeleteBuffers(1, &m_vbo_vert);
        glDeleteBuffers(1, &m_vbo_tex);
        glDeleteBuffers(1, &m_vbo_ind);
        glDeleteBuffers(1, &m_vbo_clr);
        glDeleteVertexArrays(1, &m_vao);
        glDeleteTextures(1, &m_texture_atlas);

        glDeleteBuffers(1, &m_left_panel_vbo_vert);
        glDeleteBuffers(1, &m_left_panel_vbo_tex);
        glDeleteBuffers(1, &m_left_panel_vbo_clr);
        glDeleteVertexArrays(1, &m_left_panel_vao);
    }
}


void EditorGUI::onFramebufferSizeUpdate(){
    m_fb_update = true;
}


void EditorGUI::updateBuffers(){
    float x_start, y_start;

    m_main_text->clearStrings();

    GLfloat vertex_buffer[2 * EDITOR_GUI_VERTEX_NUM];

    // left panel
    vertex_buffer[0] = 0.0;
    vertex_buffer[1] = 0.0;
    vertex_buffer[2] = 0.0;
    vertex_buffer[3] = m_fb_height;
    vertex_buffer[4] = EDITOR_GUI_LP_W;
    vertex_buffer[5] = 0.0;
    vertex_buffer[6] = EDITOR_GUI_LP_W;
    // top panel
    vertex_buffer[7] = m_fb_height;
    vertex_buffer[8] = 0.0;
    vertex_buffer[9] = m_fb_height - EDITOR_GUI_TP_H;
    vertex_buffer[10] = m_fb_width;
    vertex_buffer[11] = m_fb_height - EDITOR_GUI_TP_H;
    vertex_buffer[12] = m_fb_width;
    vertex_buffer[13] = m_fb_height;

    // buttons
    for(char i=0; i < EDITOR_GUI_N_TOP_BUTTONS; i++){
        x_start = (i+1) * BUTTON_PAD_X + i * BUTTON_SIZE_X;

        vertex_buffer[14 + i * 8] = x_start; //1
        vertex_buffer[15 + i * 8] = m_fb_height - BUTTON_PAD_Y;
        vertex_buffer[16 + i * 8] = x_start;
        vertex_buffer[17 + i * 8] = m_fb_height - BUTTON_PAD_Y - BUTTON_SIZE_Y;
        vertex_buffer[18 + i * 8] = x_start + BUTTON_SIZE_X;
        vertex_buffer[19 + i * 8] = m_fb_height - BUTTON_PAD_Y - BUTTON_SIZE_Y;
        vertex_buffer[20 + i * 8] = x_start + BUTTON_SIZE_X;
        vertex_buffer[21 + i * 8] = m_fb_height - BUTTON_PAD_Y;
    }

    // delete area
    int disp = 21 + (EDITOR_GUI_N_TOP_BUTTONS - 1) * 8;
    vertex_buffer[disp + 1] = DELETE_AREA_ORIGIN;
    vertex_buffer[disp + 2] = EDITOR_GUI_PP_MARGIN + EDITOR_GUI_PP_LOW_MARGIN - DELETE_AREA_MARGIN;
    vertex_buffer[disp + 3] = DELETE_AREA_ORIGIN;
    vertex_buffer[disp + 4] = DELETE_AREA_ORIGIN;
    vertex_buffer[disp + 5] = EDITOR_GUI_LP_W - DELETE_AREA_MARGIN;
    vertex_buffer[disp + 6] = DELETE_AREA_ORIGIN;
    vertex_buffer[disp + 7] = EDITOR_GUI_LP_W - DELETE_AREA_MARGIN;
    vertex_buffer[disp + 8] = EDITOR_GUI_PP_MARGIN + EDITOR_GUI_PP_LOW_MARGIN - DELETE_AREA_MARGIN;

    disp += 8;

    // tabs
    y_start = m_fb_height - EDITOR_GUI_TP_H - EDITOR_GUI_PP_MARGIN - TAB_HEIGTH;

    vertex_buffer[disp + 1] = EDITOR_GUI_PP_MARGIN;
    vertex_buffer[disp + 2] = y_start + TAB_HEIGTH;
    vertex_buffer[disp + 3] = EDITOR_GUI_PP_MARGIN;
    vertex_buffer[disp + 4] = y_start;
    vertex_buffer[disp + 5] = EDITOR_GUI_PP_MARGIN + TAB_WIDTH;
    vertex_buffer[disp + 6] = y_start + TAB_HEIGTH;
    vertex_buffer[disp + 7] = EDITOR_GUI_PP_MARGIN + TAB_WIDTH;
    vertex_buffer[disp + 8] = y_start;

    x_start = EDITOR_GUI_PP_MARGIN + TAB_WIDTH;
    vertex_buffer[disp + 9] = x_start;
    vertex_buffer[disp + 10] = y_start + TAB_HEIGTH;
    vertex_buffer[disp + 11] = x_start;
    vertex_buffer[disp + 12] = y_start;
    vertex_buffer[disp + 13] = x_start + TAB_WIDTH;
    vertex_buffer[disp + 14] = y_start + TAB_HEIGTH;
    vertex_buffer[disp + 15] = x_start + TAB_WIDTH;
    vertex_buffer[disp + 16] = y_start;

    m_main_text->addString(L"Parts", EDITOR_GUI_PP_MARGIN + TAB_WIDTH * 0.5f, y_start + TAB_HEIGTH * 0.5 + 5,
                            1, STRING_DRAW_ABSOLUTE_BL, STRING_ALIGN_CENTER_XY);
    m_main_text->addString(L"Staging", EDITOR_GUI_PP_MARGIN + TAB_WIDTH * 1.5f, y_start + TAB_HEIGTH * 0.5 + 5,
                            1, STRING_DRAW_ABSOLUTE_BL, STRING_ALIGN_CENTER_XY);

    m_render_context->bindVao(m_vao);

    glBindBuffer(GL_ARRAY_BUFFER, m_vbo_vert);
    glBufferData(GL_ARRAY_BUFFER, 2 * EDITOR_GUI_VERTEX_NUM * sizeof(GLfloat), vertex_buffer, GL_STATIC_DRAW);

    // parts panel
    GLfloat parts_panel_vert[12] = {EDITOR_GUI_PP_MARGIN, EDITOR_GUI_PP_MARGIN + EDITOR_GUI_PP_LOW_MARGIN,
                                    EDITOR_GUI_LP_W - EDITOR_GUI_PP_MARGIN, EDITOR_GUI_PP_MARGIN + EDITOR_GUI_PP_LOW_MARGIN,
                                    EDITOR_GUI_PP_MARGIN, m_fb_height - EDITOR_GUI_TP_H - EDITOR_GUI_PP_MARGIN - TAB_HEIGTH,
                                    EDITOR_GUI_LP_W - EDITOR_GUI_PP_MARGIN, EDITOR_GUI_PP_MARGIN + EDITOR_GUI_PP_LOW_MARGIN,
                                    EDITOR_GUI_LP_W - EDITOR_GUI_PP_MARGIN, m_fb_height - EDITOR_GUI_TP_H - EDITOR_GUI_PP_MARGIN - TAB_HEIGTH,
                                    EDITOR_GUI_PP_MARGIN, m_fb_height - EDITOR_GUI_TP_H - EDITOR_GUI_PP_MARGIN - TAB_HEIGTH};
    glBindBuffer(GL_ARRAY_BUFFER, m_left_panel_vbo_vert);
    glBufferData(GL_ARRAY_BUFFER, 12 * sizeof(GLfloat), parts_panel_vert, GL_STATIC_DRAW);
}


void EditorGUI::setButtonColor(float r ,float g, float b, float a, GLintptr offset){
    GLfloat new_color[16] = {r, g, b, a,
                             r, g, b, a,
                             r, g, b, a,
                             r, g, b, a};
    glBindBuffer(GL_ARRAY_BUFFER, m_vbo_clr);
    glBufferSubData(GL_ARRAY_BUFFER, offset, 16 * sizeof(GLfloat), new_color);
}


void EditorGUI::updateTabsColor(){
    char tab_mouseover = TAB_OPTION_NONE;
    if(m_mouse_x > EDITOR_GUI_PP_MARGIN && m_mouse_x < EDITOR_GUI_PP_MARGIN + TAB_WIDTH * 2.0f &&
       m_mouse_y > m_fb_height - EDITOR_GUI_TP_H - EDITOR_GUI_PP_MARGIN - TAB_HEIGTH &&
       m_mouse_y < m_fb_height - EDITOR_GUI_TP_H - EDITOR_GUI_PP_MARGIN){
        if(m_mouse_x < EDITOR_GUI_PP_MARGIN + TAB_WIDTH){
            tab_mouseover = TAB_OPTION_PARTS;
        }
        else{
            tab_mouseover = TAB_OPTION_STAGING;
        }
    }

    for(int i=0; i < TAB_NUMBER; i++){
        if(m_tab_option - 1 == i){
            setButtonColor(TAB_COLOR_SELECTED, ((7*4) + (4*16) + 16*i) * sizeof(GLfloat));
        }
        else{
            if(tab_mouseover -1 != i){
                setButtonColor(TAB_COLOR_UNSELECTED, ((7*4) + (4*16) + 16*i) * sizeof(GLfloat));
            }
            else{
                setButtonColor(TAB_COLOR_MOUSEOVER, ((7*4) + (4*16) + 16*i) * sizeof(GLfloat));
            }
        }
    }
}


void EditorGUI::updateDeleteArea(){
    bool delete_area_mouseover = false;
    delete_area_mouseover = m_mouse_x > DELETE_AREA_ORIGIN && m_mouse_y > DELETE_AREA_ORIGIN &&
                            m_mouse_x < EDITOR_GUI_LP_W - DELETE_AREA_MARGIN && 
                            m_mouse_y < EDITOR_GUI_PP_MARGIN + EDITOR_GUI_PP_LOW_MARGIN - DELETE_AREA_MARGIN;

    if(delete_area_mouseover){
        setButtonColor(DELETE_AREA_MOUSEOVER, ((7*4) + (3 * 16)) * sizeof(GLfloat));
    }
    else{
        setButtonColor(DELETE_AREA_COLOR, ((7*4) + (3 * 16)) * sizeof(GLfloat));
    }
}


void EditorGUI::updateTopButtons(){
    int button_mouseover = -1;
    if(m_mouse_y > m_fb_height - EDITOR_GUI_TP_H){ // mouse over top panel
        for(int i=0; i < EDITOR_GUI_N_TOP_BUTTONS; i++){
            float x_start = (i+1) * BUTTON_PAD_X + i * BUTTON_SIZE_X;
            
            if(m_mouse_x > x_start && m_mouse_x < x_start + BUTTON_SIZE_X && m_mouse_y > m_fb_height - 
               BUTTON_PAD_Y - BUTTON_SIZE_Y && m_mouse_y < m_fb_height - BUTTON_PAD_Y){
                button_mouseover = i;
            }
        }
    }

    for(int i=0; i < EDITOR_GUI_N_TOP_BUTTONS; i++){
        if(button_mouseover == i){
            setButtonColor(BUTTON_COLOR_MOUSEOVER, ((7*4) + (i * 16)) * sizeof(GLfloat));
        }
        else{
            setButtonColor(BUTTON_COLOR_DEFAULT, ((7*4) + (i * 16)) * sizeof(GLfloat));
        }
    }

    // update symmetry sides
    float row = m_symmetric_sides / 8;
    float col = ((m_symmetric_sides - 1) % 7) + 1;

    GLfloat new_coords[12] = {0.1367f * (col - 1), 0.2734f + 0.1367f * row, 1.0f,
                              0.1367f * (col - 1), 0.2734f + 0.1367f * (row + 1), 1.0f,
                              0.1367f * col, 0.2734f + 0.1367f * (row + 1), 1.0f,
                              0.1367f * col, 0.2734f + 0.1367f * row, 1.0f};

    glBindBuffer(GL_ARRAY_BUFFER, m_vbo_tex);
    glBufferSubData(GL_ARRAY_BUFFER, 7 * 3 * sizeof(GLfloat), 12 * sizeof(GLfloat), new_coords);

    // align button
    row = m_radial_align ? 2.0f : 1.0f;

    GLfloat new_coords2[12] = {0.1367f * row, 0.4101f, 1.0f,
                              0.1367f * row, 0.5468f, 1.0f,
                              0.1367f + 0.1367f * row, 0.5468f, 1.0f,
                              0.1367f + 0.1367f * row, 0.4101f, 1.0f};

    glBufferSubData(GL_ARRAY_BUFFER, (7 * 3 + 12) * sizeof(GLfloat), 12 * sizeof(GLfloat), new_coords2);

}


void EditorGUI::updateButtons(){ // used to update button colors
    updateTabsColor();
    updateDeleteArea();
    updateTopButtons();
}


void EditorGUI::render(){
    if(m_fb_update){
        m_render_context->getDefaultFbSize(m_fb_width, m_fb_height);
        m_main_text->onFramebufferSizeUpdate(m_fb_width, m_fb_height);
        m_parts_panel->onFramebufferSizeUpdate(EDITOR_GUI_LP_W - EDITOR_GUI_PP_MARGIN * 2,
                                               m_fb_height - EDITOR_GUI_TP_H - EDITOR_GUI_PP_MARGIN * 2 - EDITOR_GUI_PP_LOW_MARGIN - TAB_HEIGTH);
        m_staging_panel->onFramebufferSizeUpdate(EDITOR_GUI_LP_W - EDITOR_GUI_PP_MARGIN * 2, 
                                               m_fb_height - EDITOR_GUI_TP_H - EDITOR_GUI_PP_MARGIN * 2 - EDITOR_GUI_PP_LOW_MARGIN - TAB_HEIGTH);
        m_test_sprite->onFramebufferSizeUpdate();

        updateBuffers();
        m_fb_update = false;
    }
    updateButtons();

    m_render_context->useProgram(SHADER_GUI);
    glUniform2f(m_disp_location, 0.0, 0.0);

    m_render_context->bindVao(m_vao);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, m_texture_atlas);
    glDrawElements(GL_TRIANGLES, EDITOR_GUI_INDEX_NUM, GL_UNSIGNED_SHORT, NULL);

    if(m_tab_option == TAB_OPTION_PARTS){
        m_parts_panel->render();
        m_parts_panel->bindTexture();
    }
    else if(m_tab_option == TAB_OPTION_STAGING){
        m_staging_panel->render();
        m_staging_panel->bindTexture();
    }
    m_render_context->useProgram(SHADER_GUI);
    m_render_context->bindVao(m_left_panel_vao);
    glDrawArrays(GL_TRIANGLES, 0, 6);

    m_main_text->render();
    m_test_sprite->render();
}


int EditorGUI::update(){
    int lp_action;
    bool lmbpress = m_input->pressed_mbuttons[GLFW_MOUSE_BUTTON_1] & INPUT_MBUTTON_PRESS;
    bool rmbpress = m_input->pressed_mbuttons[GLFW_MOUSE_BUTTON_2] & INPUT_MBUTTON_PRESS;

    m_input->getMousePos(m_mouse_x, m_mouse_y);
    m_mouse_y = m_fb_height - m_mouse_y;

    if(m_tab_option == TAB_OPTION_PARTS){
        lp_action = m_parts_panel->update(m_mouse_x - EDITOR_GUI_PP_MARGIN, m_mouse_y - EDITOR_GUI_PP_MARGIN - EDITOR_GUI_PP_LOW_MARGIN); // transform coord origin
    }
    else{
        lp_action = PANEL_ACTION_NONE;
    }

    if(m_render_context->imGuiWantCaptureMouse()){
        return EDITOR_ACTION_NONE;
    }

    if(m_mouse_y > m_fb_height - EDITOR_GUI_TP_H){ // mouse over top panel
        for(int i=0; i < EDITOR_GUI_N_TOP_BUTTONS; i++){
            float x_start = (i+1) * BUTTON_PAD_X + i * BUTTON_SIZE_X;
            if(m_mouse_x > x_start && m_mouse_x < x_start + BUTTON_SIZE_X && m_mouse_y > m_fb_height - 
               BUTTON_PAD_Y - BUTTON_SIZE_Y && m_mouse_y < m_fb_height - BUTTON_PAD_Y && (lmbpress || rmbpress)){
                switch(i){
                    case BUTTON_SYMMETRIC_SIDES:
                        if(lmbpress && m_symmetric_sides < m_max_symmetric_sides){
                            m_symmetric_sides++;
                        }
                        else if(rmbpress && m_symmetric_sides > 1){
                            m_symmetric_sides--;
                        }
                        return EDITOR_ACTION_SYMMETRY_SIDES;
                    case BUTTON_SYMMETRY_MODE:
                        m_radial_align = !m_radial_align;
                        return EDITOR_ACTION_TOGGLE_ALIGN;
                    case BUTTON_CLEAR:
                        return EDITOR_ACTION_CLEAR_SCENE;
                }
                
            }
        }
    }

    if(m_mouse_x > EDITOR_GUI_PP_MARGIN && m_mouse_x < EDITOR_GUI_PP_MARGIN + TAB_WIDTH * 2.0f &&
       m_mouse_y > m_fb_height - EDITOR_GUI_TP_H - EDITOR_GUI_PP_MARGIN - TAB_HEIGTH &&
       m_mouse_y < m_fb_height - EDITOR_GUI_TP_H - EDITOR_GUI_PP_MARGIN){
        if(m_mouse_x < EDITOR_GUI_PP_MARGIN + TAB_WIDTH && lmbpress){
            m_tab_option = TAB_OPTION_PARTS;
        }
        else if(lmbpress){ // we only have 2
            m_tab_option = TAB_OPTION_STAGING;
        }
    }

    if(m_mouse_x > DELETE_AREA_ORIGIN && m_mouse_y > DELETE_AREA_ORIGIN && m_mouse_x < EDITOR_GUI_LP_W - DELETE_AREA_MARGIN && 
       m_mouse_y < EDITOR_GUI_PP_MARGIN + EDITOR_GUI_PP_LOW_MARGIN - DELETE_AREA_MARGIN){
        if(lmbpress){
            return EDITOR_ACTION_DELETE;
        }
    }

    // may change
    if(lp_action){
        return EDITOR_ACTION_OBJECT_PICK;
    }
    else{
        // focus check
        if(m_mouse_y > m_fb_height - EDITOR_GUI_TP_H || m_mouse_x < EDITOR_GUI_LP_W){
            return EDITOR_ACTION_FOCUS;
        }
        else{
            return EDITOR_ACTION_NONE;
        }
    }
}


void EditorGUI::setMasterPartList(const std::unordered_map<std::uint32_t, std::unique_ptr<BasePart>>* master_parts_list){
    m_parts_panel->setMasterPartList(master_parts_list);
}


const std::unique_ptr<BasePart>* EditorGUI::getPickedObject() const{
    return m_parts_panel->getPickedObject();
}


void EditorGUI::setSymmetrySides(int sides){
    m_symmetric_sides = sides;
}


int EditorGUI::getSymmetrySides() const{
    return m_symmetric_sides;
}


void EditorGUI::setRadialAlign(bool to_what){
    m_radial_align = to_what;
}


bool EditorGUI::getRadialAlign() const{
    return m_radial_align;
}

