
#version 330 core

layout (location = 0) in vec3 pos;
layout (location = 1) in vec3 normal_in;
layout (location = 2) in vec2 tex_coords_in;

out vec3 frag_pos;
out vec3 normal_vec;
out vec2 tex_coords;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main() {
    frag_pos = vec3(model * vec4(pos, 1.0));
    //calculating normals is HIGHLY INNEFFICIENT and should be done on CPU
    //  and sent down using uniforms
    normal_vec = /*mat3(transpose(inverse(model))) * */normal_in;

    tex_coords = tex_coords_in;

    gl_Position = projection * view * model * vec4(pos, 1.0);
}

