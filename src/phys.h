#ifndef PHYS_H
#define PHYS_H

#include <math.h>
#include <cglm/cglm.h>

#include "sprite.h"
#include "list.h"

#define CIRC_TYPE 0
#define RECT_TYPE 1

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

struct Rect {
    struct v3 color;

    struct v2 pos;
    float length;
    float height;

    float restitution;
};

// must be called before any objects are added
int initPhysRenderer(struct TexMan *texman, struct Shader *shdr);

// add circle to the world
int addCircle(struct List *objects, float x, float y, float xv, float yv, float radius, float mass);

// Add rectangle to objects list
int addRect(struct List *objects, float x, float y, float l, float h);



// Rendering stuff
// draws all objecs in list to the screen
int drawObjects(struct List *objects);
int drawCircle(struct Circle *c);
int drawRect(struct Rect *r);



// Physics stuff

// calculate distance between two circles
float distCirc(float x1, float y1, float x2, float y2);
// update physics variables
int updateCircle(struct Circle *c, float dt);
int updatePhysics(struct List *objects, float dt);
int isCollidingCircVCirc(struct Circle *a, struct Circle *b);
int collideCirc(struct Circle *a, struct Circle *b);


#endif