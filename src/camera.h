#ifndef CAMERA_H
#define CAMERA_H

#include <cglm/cglm.h>

//consider moving these to a global constants or macros header file
#define degToRad(deg) ((deg) * M_PI / 180.0)
#define radToDeg(rad) ((rad) * 180.0 / M_PI)

struct Camera {
    vec2 position;

    float movement_speed;
    float zoom;
};

enum camera_movement {
    cam_left,
    cam_right,
    cam_up,
    cam_down
};

void initializeCamera(struct Camera *cam, vec3 position, float movement_speed, float zoom);

void translateCamera(struct Camera *cam, enum camera_movement direction, float delta_time);

void zoomCamera(struct Camera *cam, float y_offset);

#endif