#version 410

layout(location = 0) in vec3 vertex_position;
layout(location = 1) in vec3 vertex_normal;
layout(location = 2) in vec2 texture_coord;

uniform mat4 view, proj, model;

out vec3 normal_eye, position_eye;
out vec2 st;

void main() {
    st = texture_coord;
    position_eye = vec3(view * model * vec4(vertex_position, 1.0));
    normal_eye = vec3(view * model * vec4(vertex_normal, 0.0));
    gl_Position = proj * view * model * vec4 (vertex_position, 1.0);
}
