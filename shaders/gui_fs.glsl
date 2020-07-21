#version 410

in vec4 vert_color;
in vec3 sta;

out vec4 frag_colour;

uniform sampler2D texture_sampler;

// simple panel drawing

void main(){
    vec4 sampled = texture(texture_sampler, sta.xy) * sta.z;
    frag_colour = vert_color + sampled;
}

