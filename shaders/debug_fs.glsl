#version 410

out vec4 frag_colour;

uniform vec3 line_color;

void main() {
    frag_colour = vec4(line_color, 1.0);
}
