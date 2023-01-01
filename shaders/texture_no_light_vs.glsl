#version 410

layout(location = 0) in vec3 vertex_position;
layout(location = 1) in vec2 texture_coord;

uniform mat4 view, proj, model;

out vec2 st;

void main() {
    st = texture_coord;
    gl_Position = proj * view * model * vec4 (vertex_position, 1.0);
}
