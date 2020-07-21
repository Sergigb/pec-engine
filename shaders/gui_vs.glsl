#version 410
layout(location = 0) in vec2 vertex;
layout(location = 1) in vec4 color;
layout(location = 2) in vec3 tex_coord_alpha;

uniform mat4 projection;

out vec4 vert_color;
out vec3 sta;

// simple panel drawing

void main(){
    vert_color = color;
    sta = tex_coord_alpha;
    gl_Position = projection * vec4(vertex, 0.0, 1.0); //should work
}
