#version 410

in vec2 st;
out vec4 frag_colour;

uniform sampler2D texture_sampler;
uniform float alpha;


void main(){
    vec4 sampled = texture(texture_sampler, st);
    frag_colour = vec4(sampled.rgb, sampled.a * alpha);
}
