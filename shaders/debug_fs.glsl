#version 410

out vec4 frag_colour;

uniform vec3 line_color;
uniform float alpha;

void main() {
    frag_colour = vec4(line_color, alpha);
}
