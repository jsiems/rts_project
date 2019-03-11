
#ifndef SPRITE_H
#define SPRITE_H

#include <cglm/cglm.h>

#include "texman.h"
#include "shader.h"

struct SpriteRenderer {
    unsigned int VAO;
};

void initSpriteRenderer(struct SpriteRenderer *sprite);

void drawSprite(struct SpriteRenderer *sprite, struct Shader *shader, 
                int texture_id, vec2 position, vec2 size, float rotation, vec3 color);

#endif