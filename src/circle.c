
#include "circle.h"

#define min(x, y) (x < y ? x : y)

// private global circle rendering variables
static struct SpriteRenderer circle_sprite;
static struct Shader *circle_shader;
static int circle_tex_id = 0;
static int num_circles = 0; // current number of circles
static int circle_renderer_initialized = 0;

// must be called before any circles are added
int initCircleRenderer(struct TexMan *texman, struct Shader *shader) {

    // set shader
    circle_shader = shader;

    // get texture id
    circle_tex_id = getTextureId(texman, "circle");

    // intialize sprite renderer
    initSpriteRenderer(&circle_sprite);

    circle_renderer_initialized = 1;
    return 0;
}

// initialize this game object
int initCircle(struct Circle *c, float x, float y, float xv, float yv, float radius) {
    if(circle_renderer_initialized == 0) {
        return 1;
    }

    c->pos.x = x;
    c->pos.y = y;
    c->vel.x = xv;
    c->vel.y = yv;

    c->radius = radius;
    c->color.x = 1.0f;
    c->color.y = 1.0f;
    c->color.z = 1.0f;

    c->restitution = 1;

    num_circles ++;

    return 0;
}

// update physics variables
int updateCircle(struct Circle *c, float dt) {
    c->pos.x += c->vel.x * dt;
    c->pos.y += c->vel.y * dt;

    return 0;
}

// draw this object to the screen
int drawCircle(struct Circle *c) {
    drawSprite(&circle_sprite, circle_shader, circle_tex_id, 
    (vec2){c->pos.x - c->radius, c->pos.y - c->radius},     // position
    (vec2){c->radius * 2, c->radius * 2},                   // length, width
    0.0f, (vec3){c->color.x, c->color.y, c->color.z});

    return 0;
}

float distCirc(float x1, float y1, float x2, float y2) {
    return sqrt(pow(x1 - x2, 2) + pow(y1 - y2, 2));
}

int isColliding(struct Circle *a, struct Circle *b) {
    float r = (a->radius + b->radius);
    return distCirc(a->pos.x, a->pos.y, b->pos.x, b->pos.y) < r;
}

// called when two circles are colliding
int collide(struct Circle *a, struct Circle *b) {
    // see : https://gamedevelopment.tutsplus.com/tutorials/how-to-create-a-custom-2d-physics-engine-the-basics-and-impulse-resolution--gamedev-6331
    
    // relative velocity
    struct v2 rv;
    rv.x = b->vel.x - a->vel.x;
    rv.y = b->vel.y - a->vel.y;

    #include <stdio.h>
    printf("Relative Velocity: x: %.2f\ty: %.2f\n", rv.x, rv.y);

    // calculate normal vector from a to b
    // THIS normal calculation will likely be an input in the future
    struct v2 norm;
    norm.x = b->pos.x - a->pos.x;
    norm.y = b->pos.y - a->pos.y;
    float mag = distCirc(norm.x, 0, norm.y, 0);
    norm.x /= mag;
    norm.y /= mag;


    printf("norm vec: x: %.2f\ty: %.2f\n", norm.x, norm.y);

    float vel_norm = rv.x * norm.x + rv.y * norm.y;

    printf("norm vel: %.2f\n", vel_norm);

    // do not collide if velocities are separating
    if(vel_norm > 0) {
        return 1;
    }

    // use the lowest restitution (bounciness)
    float e = min(a->restitution, b->restitution);

    // calculate impulse scalar
    float j = -(1 + e) * vel_norm;
    j /= a->inv_mass + b->inv_mass;

    // Apply impulse
    struct v2 impulse;
    impulse.x = j * norm.x;
    impulse.y = j * norm.y;
    a->vel.x -= a->inv_mass * impulse.x;
    a->vel.y -= a->inv_mass * impulse.y;
    b->vel.x += b->inv_mass * impulse.x;
    b->vel.y += b->inv_mass * impulse.y;

    return 0;
}
