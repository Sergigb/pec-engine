#version 410
layout(location = 0) in vec2 vertex;
layout(location = 1) in vec2 tex_coord;
out vec2 st;

uniform mat4 projection;

void main(){
    st = tex_coord;
    gl_Position = projection * vec4(vertex, 0.0, 1.0); //should work
}
