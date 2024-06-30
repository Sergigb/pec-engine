#version 410

in vec2 st;
in vec4 color;
out vec4 frag_colour;

uniform sampler2D texture_sampler;

void main(){
    frag_colour = vec4(color.rgb , texture(texture_sampler, st).r * color.a);
}

