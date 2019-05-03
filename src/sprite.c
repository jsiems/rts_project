
#include "sprite.h"

#define FPV 4

void initSpriteRenderer(struct SpriteRenderer *sprite) {
    unsigned int VBO, VAO;
    // 6 vertices with 4 floats per vertex
    float verts[] = {
        0.0, 1.0, 0.0, 1.0,
        1.0, 1.0, 1.0, 1.0,
        0.0, 0.0, 0.0, 0.0,
        1.0, 1.0, 1.0, 1.0,
        0.0, 0.0, 0.0, 0.0,
        1.0, 0.0, 1.0, 0.0
    };
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(verts), verts, GL_STATIC_DRAW);

    // vertex buffer attributes
    // 2 floats for positional data, starts at 0
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, FPV * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    // 2 floats for texture coordinates, starts at 2
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, FPV * sizeof(float), (void*)(2 * sizeof(float)));
    glEnableVertexAttribArray(1);

    sprite->VAO = VAO;

    return;
}

void drawSprite(struct SpriteRenderer *sprite, struct Shader *shader, 
                int texture_id, vec2 position, vec2 size, float rotation, vec3 color) {
    // bind the texture
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture_id);

    glUseProgram(shader->id);
    glBindVertexArray(sprite->VAO);

    mat4 model;

    glm_translate_make(model, (vec3){position[0], position[1], 0.0});
    glm_translate(model, (vec3){0.5 * size[0], 0.5 * size[1], 0.0});
    glm_rotate(model, rotation, (vec3){0.0, 0.0, 1.0});
    glm_translate(model, (vec3){-0.5 * size[0], -0.5 * size[1], 0.0});
    glm_scale(model, (vec3){size[0], size[1], 1.0});

    setMat4(shader, "model", model);
    setVec3(shader, "sprite_color", color);

    glDrawArrays(GL_TRIANGLES, 0, 6);

    return;
}

