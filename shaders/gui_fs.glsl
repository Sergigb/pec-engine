#version 410

in vec4 vert_color;
out vec4 frag_colour;

// simple panel drawing

void main(){
    frag_colour = vert_color;
}

