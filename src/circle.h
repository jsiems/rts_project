#ifndef CIRCLE_H
#define CIRCLE_H

#include <math.h>
#include <cglm/cglm.h>

#include "sprite.h"

struct v2 {
    float x;
    float y;
};

struct v3 {
    float x;
    float y;
    float z;
};

struct Circle {
    // rendering variables
    struct v3 color;

    // physics variables 
    struct v2 pos;
    struct v2 vel;
    float radius;
    float mass;
    float inv_mass;    // calculate in init
    float restitution; // == bounciness
    
};

// must be called before any circles are added
int initCircleRenderer(struct TexMan *texman, struct Shader *shader);

// initialize this game object
int initCircle(struct Circle *c, float x, float y, float xv, float yv, float radius);

float distCirc(float x1, float y1, float x2, float y2);

// update physics variables
int updateCircle(struct Circle *c, float dt);

// draw this object to the screen
int drawCircle(struct Circle *c);

// returns true if two circles are colliding
int isColliding(struct Circle *a, struct Circle *b);

// called when two circles are colliding
int collide(struct Circle *a, struct Circle *b);


#endif