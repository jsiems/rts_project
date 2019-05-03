
#version 330 core

// Uniforms
uniform sampler2D image;
uniform vec3 sprite_color;

// ***** inputs / outputs *****
out vec4 color;
in vec2 tex_coords;

void main() {
    vec4 texColor = vec4(sprite_color, 1.0) * texture(image, tex_coords);
    if(texColor.a < 0.1) {
        discard;
    }
    color = texColor;
}
