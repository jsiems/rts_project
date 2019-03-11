
#version 330 core

layout (location = 0) in vec2 pos;
layout (location = 1) in vec2 tex_coords_in;

out vec2 tex_coords;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main() {
    tex_coords = tex_coords_in;
    gl_Position = projection * view * model  * vec4(pos, 0.0, 1.0);
}

