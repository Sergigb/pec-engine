#version 410

uniform vec4 object_color;

out vec4 frag_colour;

void main() {
    frag_colour = vec4(object_color);
}
