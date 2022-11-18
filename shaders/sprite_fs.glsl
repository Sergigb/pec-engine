#version 410

in vec2 st;
out vec4 frag_colour;

uniform sampler2D texture_sampler;

void main(){
    frag_colour = vec4(texture(texture_sampler, st).xyz, 1.0);
}
