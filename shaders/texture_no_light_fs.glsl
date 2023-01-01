#version 410

in vec2 st;
out vec4 frag_colour;

uniform mat4 view;
uniform sampler2D tex;

void main() {
    vec4 texel = texture(tex, st);
    
    // final colour
    frag_colour = vec4 (texel.xyz, 1.0);
}