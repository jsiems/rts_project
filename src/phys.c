
#include "phys.h"

#define min(x, y) (x < y ? x : y)
#define max(x, y) (!min(x, y))

// private global circle rendering variables
static struct SpriteRenderer sprite;
static struct Shader *shader;
static int circle_tex_id = 0;
static int rect_tex_id = 0;
static int renderer_initialized = 0;

// Always present forces
static float gravity = 0.5;
static float air_res = 0.9999;

// PRIVATE PROTOS
float dist(float x1, float y1, float x2, float y2);
float distSquared(float x1, float y1, float x2, float y2);
float lengthV2(struct v2 *v);
float lengthV2Squared(struct v2 *v);

// must be called before any circles are added
int initPhysRenderer(struct TexMan *texman, struct Shader *shdr) {

    // set shader
    shader = shdr;

    // get texture id
    circle_tex_id = getTextureId(texman, "circle");
    rect_tex_id = getTextureId(texman, "rect");

    // intialize sprite renderer
    initSpriteRenderer(&sprite);

    renderer_initialized = 1;
    return 0;
}

// initialize this game object
struct Node *addCircle(struct List *objects, float x, float y, float xv, float yv, float radius, float mass) {
    if(renderer_initialized == 0) {
        return 0;
    }

    struct Circle c;
    struct Node *new;

    c.pos.x = x;
    c.pos.y = y;
    c.vel.x = xv;
    c.vel.y = yv;

    c.radius = radius;
    c.color.x = 1.0f;
    c.color.y = 1.0f;
    c.color.z = 1.0f;

    c.mass = mass;
    if(c.mass == 0) {
        c.inv_mass = 0;
    }
    else {
        c.inv_mass = 1 / c.mass;
    }
    c.restitution = 0.7;

    new = insertNode(objects, &c, sizeof(struct Circle), CIRC_TYPE);

    return new;
}

// initialize this game object
int addRect(struct List *objects, float x, float y, float l, float h) {
    if(renderer_initialized == 0) {
        return 1;
    }

    struct Rect r;

    r.pos.x = x;
    r.pos.y = y;
    r.length = l;
    r.height = h;
    r.color.x = 1.0f;
    r.color.y = 1.0f;
    r.color.z = 1.0f;
    r.restitution = 1;

    insertNode(objects, &r, sizeof(struct Rect), RECT_TYPE);

    return 0;
}

int drawObjects(struct List *objects) {
    struct Node *node;
    node = objects->front;
    while(node != 0) {

        if(node->data_type == CIRC_TYPE) {
            drawCircle((struct Circle *)node->data);
        }
        else if(node->data_type == RECT_TYPE) {
            drawRect((struct Rect *)node->data);
        }
        else {
            printf("DRAW OBJECTS ERROR: Unkown type given: %d\n", node->data_type);
            exit(1);
        }

        node = node->next;
    }

    return 0;
}

// draw this object to the screen
int drawCircle(struct Circle *c) {
    drawSprite(&sprite, shader, circle_tex_id, 
    (vec2){c->pos.x - c->radius, c->pos.y - c->radius},     // position
    (vec2){c->radius * 2, c->radius * 2},                   // length, width
    0.0f, (vec3){c->color.x, c->color.y, c->color.z});

    return 0;
}

int drawRect(struct Rect *r) {
    drawSprite(&sprite, shader, rect_tex_id, 
    (vec2){r->pos.x, r->pos.y},     // position
    (vec2){r->length, r->height},                   // length, width
    0.0f, (vec3){r->color.x, r->color.y, r->color.z});

    return 0;
}

// update physics variables
int updateCircle(struct Circle *c, float dt) {

    // add gravity and air resistance
    if(c->mass > 0) {
        c->vel.y += gravity;

        c->vel.x *= air_res;
        c->vel.y *= air_res;

        c->pos.x += c->vel.x * dt;
        c->pos.y += c->vel.y * dt;
    }

    if(c->color.y < 1.0f) {
        c->color.y += dt / 4;
    }
    if(c->color.z < 1.0f) {
        c->color.z += dt / 4;
    }
    if(c->color.y > 1.0f) {
        c->color.y = 1.0f;
    }
    if(c->color.z > 1.0f) {
        c->color.z = 1.0f;
    }

    return 0;
}

// updates physics of all these objects
int updatePhysics(struct List *objects, float dt) {
    struct Node *node, *other;
    node = objects->front;
    while(node != 0) {
        other = objects->front;
        while(other != 0) {
            if(other != node) {
                if(node->data_type == CIRC_TYPE && other->data_type == CIRC_TYPE) {
                    struct Manifold m;
                    m.a = node->data;
                    m.b = other->data;
                    if(isCollidingCircVCirc(&m)){
                        collideCirc(&m);
                        posCorCircVCirc(&m);
                    }
                }
                if(node->data_type == CIRC_TYPE && other->data_type == RECT_TYPE) {
                    if(/*isCollidingCircVRect(current, (struct Rect *)other->data)*/0) {
                        //current->color = (struct v3){1.0f, 0.0f, 0.0f};
                    }
                    else {
                        //current->color = (struct v3){1.0f, 1.0f, 1.0f};
                    }
                }
            }
            other = other->next;
        }
        if(node->data_type == CIRC_TYPE) {
            updateCircle((struct Circle *)node->data, dt);
        }

        node = node->next;
    }
    return 0;
}

float dist(float x1, float y1, float x2, float y2) {
    return sqrt(pow(x1 - x2, 2) + pow(y1 - y2, 2));
}

float distSquared(float x1, float y1, float x2, float y2) {
    return pow(x1 - x2, 2) + pow(y1 - y2, 2);
}

float lengthV2(struct v2 *v) {
    return dist(v->x, v->y, 0, 0);
}

float lengthV2Squared(struct v2 *v) {
    return distSquared(v->x, v->y, 0, 0);
}

int isCollidingCircVCirc(struct Manifold *m) {

    struct Circle *a, *b;
    a = (struct Circle *)m->a;
    b = (struct Circle *)m->b;

    if(a->inv_mass == 0 && b->inv_mass == 0) {
        return 0;
    }

    // vector from a to b
    struct v2 n;
    n.x = b->pos.x - a->pos.x;
    n.y = b->pos.y - a->pos.y;

    float r = a->radius + b->radius;
    r *= r;

    // if dist from a to b squared is greater than r squared
    if(lengthV2Squared(&n) > r) {
        return 0;
    }

    float d = lengthV2(&n);

    // if circles are not at the exact same position
    if(d != 0) {
        m->penetration = a->radius + b->radius - d;

        m->norm.x = n.x / d;
        m->norm.y = n.y / d;
    }
    else {
        // circles are directly on top of eachother, just pick something
        m->penetration = a->radius;
        m->norm.x = 1.0f;
        m->norm.y = 0.0f;
    }

    return 1;
}

int isCollidingCircVRect(struct Manifold *m) {
    struct Circle *c = m->a;
    struct Rect *r = m->b;
    return dist(c->pos.x, c->pos.y, r->pos.x, r->pos.y) < c->radius;
}

// called when two circles are colliding
int collideCirc(struct Manifold *m) {

    struct Circle *a, *b;
    a = (struct Circle *)m->a;
    b = (struct Circle *)m->b;

    // don't let static objects collide
    if(a->inv_mass == 0 && b->inv_mass == 0) {
        return 1;
    }

    // relative velocity
    struct v2 rv;
    rv.x = b->vel.x - a->vel.x;
    rv.y = b->vel.y - a->vel.y;

    float vel_norm = rv.x * m->norm.x + rv.y * m->norm.y;

    // do not collide if velocities are separating
    if(vel_norm > 0) {
        return 2;
    }

    // update color for cool effects
    #define DV 10
    a->color.y -= abs(vel_norm / DV);
    a->color.z -= abs(vel_norm / DV);
    if(a->color.y < 0) {
        a->color.y = 0;
    }
    if(a->color.z < 0) {
        a->color.z = 0;
    }
    b->color.y -= abs(vel_norm / DV);
    b->color.z -= abs(vel_norm / DV);
    if(b->color.y < 0) {
        b->color.y = 0;
    }
    if(b->color.z < 0) {
        b->color.z = 0;
    }

    // use the lowest restitution (bounciness)
    float e = min(a->restitution, b->restitution);

    // calculate impulse scalar
    float j = -(1 + e) * vel_norm;
    j /= a->inv_mass + b->inv_mass;

    // Apply impulse
    struct v2 impulse;
    impulse.x = j * m->norm.x;
    impulse.y = j * m->norm.y;
    a->vel.x -= a->inv_mass * impulse.x;
    a->vel.y -= a->inv_mass * impulse.y;
    b->vel.x += b->inv_mass * impulse.x;
    b->vel.y += b->inv_mass * impulse.y;

    return 0;
}

int posCorCircVCirc(struct Manifold *m) {
    struct Circle *a, *b;
    a = (struct Circle *)m->a;
    b = (struct Circle *)m->b;

    float percent = 0.2;
    float slop = 0.1;
    struct v2 correction;
    float corr_factor = max(m->penetration - slop, 0.0f) / (a->inv_mass + b->inv_mass) * percent;
    correction.x = corr_factor * m->norm.x;
    correction.y = corr_factor * m->norm.y;

    a->pos.x -= a->inv_mass * correction.x;
    a->pos.y -= a->inv_mass * correction.y;
    b->pos.x += b->inv_mass * correction.x;
    b->pos.y += b->inv_mass * correction.y;

    return 0;
}
