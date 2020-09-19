#version 410

in vec3 normal_eye, position_eye;
in vec2 st;
out vec4 frag_colour;

uniform mat4 view;
uniform vec3 light_pos;
uniform sampler2D tex;

// fixed point light properties
vec3 Ls = vec3 (1.0, 1.0, 1.0); // specular colour
vec3 Ld = vec3 (.75, .75, .75); // diffuse light colour
vec3 La = vec3 (.75, .75, .75); // ambient colour

// surface reflectance
vec3 Ks = vec3 (.33, .33, .33); // specular light reflectance
vec3 Kd = vec3 (1.0, 1.0, 1.0); // diffuse surface reflectance
vec3 Ka = vec3 (0.33, 0.33, 0.33); // ambient light reflectance
float specular_exponent = 100.0; // specular 'power'


void main() {
    vec3 Ia = La * Ka;

    vec3 n_eye = normalize(normal_eye);

    vec3 light_position_eye = vec3(view * vec4(light_pos, 1.0));
    vec3 distance_to_light_eye = light_position_eye - position_eye;
    vec3 direction_to_light_eye = normalize(distance_to_light_eye);
    float dot_prod = dot(direction_to_light_eye, n_eye);
    dot_prod = max(dot_prod, 0.0);
    vec3 Id = Ld * Kd * dot_prod; // final diffuse intensity

    // specular intensity
    vec3 surface_to_viewer_eye = normalize (-position_eye);

    //blinn
    vec3 half_way_eye = normalize (surface_to_viewer_eye + direction_to_light_eye);
    float dot_prod_specular = max (dot (half_way_eye, n_eye), 0.0);
    float specular_factor = pow (dot_prod_specular, specular_exponent);

    vec3 Is = Ls * Ks * specular_factor; // final specular intensity

    vec4 texel = texture(tex, st);

    // final colour
    frag_colour = vec4(texel.xyz * (Is + Id + Ia), 1.0);
}
