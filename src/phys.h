#ifndef PHYS_H
#define PHYS_H

#include <math.h>
#include <cglm/cglm.h>

#include "sprite.h"
#include "list.h"
#include "const.h"

#define CIRC_TYPE 0
#define RECT_TYPE 1

// Inspiration:
// https://gamedevelopment.tutsplus.com/tutorials/how-to-create-a-custom-2d-physics-engine-the-basics-and-impulse-resolution--gamedev-6331

extern double glfwGetTime();

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
    int explosive;

    float last_update_time;
};

struct Rect {
    struct v3 color;

    struct v2 pos;
    float length;
    float height;

    float restitution;
};

struct Manifold {
    void *a;
    void *b;
    float penetration;
    struct v2 norm;
};

// must be called before any objects are added
int initPhysRenderer(struct TexMan *texman, struct Shader *shdr);

// add circle to the world
struct Node *addCircle(struct List *objects, float x, float y, float xv, float yv, float radius, float mass);

// Add rectangle to objects list
int addRect(struct List *objects, float x, float y, float l, float h);



// Rendering stuff
// draws all objecs in list to the screen
int drawObjects(struct List *objects, float runtime);
int drawCircle(struct Circle *c);
int drawRect(struct Rect *r);



// Physics stuff

// returns 1 if this circle is offscreen, 0 otherwise
int updateCircle(struct Circle *c, float dt);
int updatePhysics(struct List *objects, float runtime);
int isCollidingCircVCirc(struct Manifold *m);
int isCollidingCircVRect(struct Manifold *m);
int collideCirc(struct Manifold *m);
int collideCircVRect(struct Manifold *m);
int posCorCircVCirc(struct Manifold *m);
int posCorCircVRect(struct Manifold *m);


#endif