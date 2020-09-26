#version 410

layout(location = 0) in vec3 vertex_position;
layout(location = 1) in vec3 vertex_normal;

uniform mat4 view, proj, model, relative_planet;
uniform vec2 tex_shift;
uniform float planet_radius, patch_scale;
uniform sampler2D elevation;

out vec3 normal_eye, position_eye;
out vec2 st;

void main() {
    st = ((-vertex_position.zy) * patch_scale) - (tex_shift.yx - 0.5);
    float height = texture(elevation, st).r * 6400; // 6400 max elevation heightmap, change to uniform

    vec3 position_eye_norm = normalize(vec3(relative_planet * vec4(vertex_position, 1.0))) * (planet_radius + height);
    position_eye = vec3(view * model * vec4(position_eye_norm, 1.0));
    normal_eye = vec3(view * model * vec4(position_eye_norm, 0.0));
    gl_Position = proj * vec4 (position_eye, 1.0);
}
