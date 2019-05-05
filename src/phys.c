
#include "phys.h"

#define min(x, y) (x < y ? x : y)
#define max(x, y) (!min(x, y))
#define clamp(a, min, max) ((a < min ? min : a) == (a > max ? max : a) ? a : (a < min ? min : max))

// private global circle rendering variables
static struct SpriteRenderer sprite;
static struct Shader *shader;
static int circle_tex_id = 0;
static int rect_tex_id = 0;
static int renderer_initialized = 0;

// Always present forces
static float gravity = 80;

// PRIVATE PROTOS
float dist(float x1, float y1, float x2, float y2);
float distSquared(float x1, float y1, float x2, float y2);
float lengthV2(struct v2 *v);
float lengthV2Squared(struct v2 *v);

// static states
struct Node *render_node = 0;
struct Node *phys_node = 0;

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
    c.explosive = 0;

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

    printf("adding node at x: %.2f\ty: %.2f\n", x, y);

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

int drawObjects(struct List *objects, float runtime) {
    struct Node *start_node;
    float start_time = glfwGetTime();

    if(render_node == 0) {
        render_node = objects->front;
    }
    start_node = render_node->prev;

    while(render_node != start_node && glfwGetTime() - start_time < runtime) {
        if(render_node->data_type == CIRC_TYPE) {
            drawCircle((struct Circle *)render_node->data);
        }
        else if(render_node->data_type == RECT_TYPE) {
            drawRect((struct Rect *)render_node->data);
        }
        else {
            printf("DRAW OBJECTS ERROR: Unkown type given: %d\n", render_node->data_type);
            exit(1);
        }

        render_node = render_node->next;
        if(render_node == 0) {
            render_node = objects->front;
        }
        if(start_node == 0) {
            start_node = objects->front;
        }
    }

    // delay till our time slot is used up
    while(glfwGetTime() - start_time < runtime);

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
        c->vel.y += gravity * dt;

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

    // if offscreen, return 1
    if(c->pos.x + c->radius < 0 || c->pos.x - c->radius > SCREEN_WIDTH
        || c->pos.y + c->radius < 0 || c->pos.y - c->radius > SCREEN_HEIGHT) {
        printf("in update, this circle is off screen\n");
        printf("x: %.2f\ty: %.2f\n", c->pos.x, c->pos.y);
        return 1;
    }

    return 0;
}

// updates physics of all these objects
int updatePhysics(struct List *objects, float runtime) {
    struct Node *start_node, *other;
    float start_time = glfwGetTime();

    if(phys_node == 0) {
        phys_node = objects->front;
    }
    start_node = phys_node->prev;

    while(phys_node != start_node && glfwGetTime() - start_time < runtime) {
        other = objects->front;
        while(other != 0) {
            if(other != phys_node) {
                for(int i = 0; i < 2000; i ++);
                if(phys_node->data_type == CIRC_TYPE && other->data_type == CIRC_TYPE) {
                    struct Manifold m;
                    m.a = phys_node->data;
                    m.b = other->data;
                    if(isCollidingCircVCirc(&m)){
                        collideCirc(&m);
                        posCorCircVCirc(&m);
                    }
                }
                if(phys_node->data_type == CIRC_TYPE && other->data_type == RECT_TYPE) {
                    struct Manifold m;
                    m.a = phys_node->data;
                    m.b = other->data;
                    if(isCollidingCircVRect(&m)) {
                        collideCircVRect(&m);
                        posCorCircVRect(&m);
                    }
                }
            }
            other = other->next;
        }
        if(phys_node->data_type == CIRC_TYPE) {
            float dt = glfwGetTime() - ((struct Circle *)phys_node->data)->last_update_time;
            if(updateCircle((struct Circle *)phys_node->data, dt)) {
                struct Node *temp = phys_node;
                // update render_node if it is being removed
                if(phys_node == render_node) {
                    render_node = phys_node->next;
                    if(render_node == 0) {
                        render_node = objects->front;
                    }
                }
                phys_node = phys_node->next;
                if(phys_node == 0) {
                    phys_node = objects->front;
                }
                printf("REMOVING NODE\n");
                printf("x: %.2f\ty: %.2f\n", ((struct Circle *)phys_node->data)->pos.x, ((struct Circle *)phys_node->data)->pos.y);
                removeNode(objects, temp);
                continue;
            }
            else {
                ((struct Circle *)phys_node->data)->last_update_time = glfwGetTime();
            }
        }

        phys_node = phys_node->next;
        if(phys_node == 0) {
            phys_node = objects->front;
        }
        if(start_node == 0) {
            start_node = objects->front;
        }
    }

    // delay till our time slot is used up
    while(glfwGetTime() - start_time < runtime);

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
    struct Circle *c = (struct Circle *)m->a;
    struct Rect *r = (struct Rect *)m->b;

    if(c->inv_mass == 0) {
        //return 0;
    }

    // vector from a to b
    struct v2 n;
    n.x = c->pos.x - r->pos.x;
    n.y = c->pos.y - r->pos.y;

    // closest point on a to center of b
    struct v2 closest;
    closest.x = clamp(c->pos.x, r->pos.x, r->pos.x + r->length);
    closest.y = clamp(c->pos.y, r->pos.y, r->pos.y + r->height);

    int inside = 0;

    // if the center of the circle is in the rectangle
    if(c->pos.x == closest.x && c->pos.y== closest.y) {
        inside = 1;
        // find the closest edge and set closest to be there
        // (Probably a better way to do this...)
        float dtl, dtr, dtt, dtb;
        dtl = n.x;
        dtr = r->length - n.x;
        dtt = n.y;
        dtb = r->height - n.y;
        if(dtl < dtr && dtl < dtt && dtl < dtb) {
            closest.x = r->pos.x;
        }
        else if(dtr < dtt && dtr < dtb) {
            closest.x = r->pos.x + r->length;
        }
        else if(dtt < dtb) {
            closest.y = r->pos.y;
        }
        else {
            closest.y = r->pos.y + r->height;
        }
    }

    struct v2 normal;
    normal.x = closest.x - c->pos.x;
    normal.y = closest.y - c->pos.y;
    float d = lengthV2Squared(&normal);

    // circle not in rectangle
    if(!inside && d > c->radius * c->radius) {
        return 0;
    }

    d = sqrt(d);

    if(d < 0.000000001 || isnan(d)) {
        d = 0.0001;
    }

    if(inside) {
        m->norm.x = (-1 * normal.x) / d;
        m->norm.y = (-1 * normal.y) / d;
        m->penetration = c->radius - d;
    } else {
        m->norm.x = normal.x / d;
        m->norm.y = normal.y / d;
        m->penetration = c->radius - d;
    }

    if(isnan(m->norm.x) || isnan(m->norm.y)) {
        m->norm.x = 1.0f;
        m->norm.y = 0.0f;
    }

    return 1;
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
    if(a->explosive || b->explosive) {
        vel_norm -= 250;
    }

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

// called when two circles are colliding
int collideCircVRect(struct Manifold *m) {
    struct Circle *c;
    c = (struct Circle *)m->a;

    // don't let static objects collide
    if(c->inv_mass == 0) {
        return 1;
    }

    // relative velocity
    struct v2 rv;
    rv.x = -1 * c->vel.x;
    rv.y = -1 * c->vel.y;

    float vel_norm = rv.x * m->norm.x + rv.y * m->norm.y;

    // do not collide if velocities are separating
    if(vel_norm > 0) {
        return 2;
    }

    // update color for cool effects
    #define DV 10
    c->color.y -= abs(vel_norm / DV);
    c->color.z -= abs(vel_norm / DV);
    if(c->color.y < 0) {
        c->color.y = 0;
    }
    if(c->color.z < 0) {
        c->color.z = 0;
    }

    // use the lowest restitution (bounciness)
    float e = min(c->restitution, 1.0f);

    // calculate impulse scalar
    float j = -(1 + e) * vel_norm;
    j /= c->inv_mass;

    // Apply impulse
    struct v2 impulse;
    impulse.x = j * m->norm.x;
    impulse.y = j * m->norm.y;
    c->vel.x -= c->inv_mass * impulse.x;
    c->vel.y -= c->inv_mass * impulse.y;

    return 0;
}

int posCorCircVCirc(struct Manifold *m) {
    struct Circle *a, *b;
    a = (struct Circle *)m->a;
    b = (struct Circle *)m->b;

    float percent = 0.8;
    float slop = 0.01;
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

int posCorCircVRect(struct Manifold *m) {
    struct Circle *c;
    c = (struct Circle *)m->a;

    if(c->inv_mass == 0) {
        return 1;
    }

    float percent = 0.8;
    float slop = 0.01;
    struct v2 correction;
    float corr_factor = max(m->penetration - slop, 0.0f) / (c->inv_mass) * percent;
    correction.x = corr_factor * m->norm.x;
    correction.y = corr_factor * m->norm.y;

    c->pos.x -= c->inv_mass * correction.x;
    c->pos.y -= c->inv_mass * correction.y;

    return 0;
}
