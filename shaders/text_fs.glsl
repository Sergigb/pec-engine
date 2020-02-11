#version 410

in vec2 st;
out vec4 frag_colour;

uniform sampler2D texture_sampler;
uniform vec3 text_color;

void main(){
    vec4 sampled = vec4(1.0, 1.0, 1.0, texture(texture_sampler, st).r);
    frag_colour = vec4(text_color, 1.0) * sampled;
}

