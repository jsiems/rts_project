
#include "camera.h"

 void initializeCamera(struct Camera *cam, vec3 position, float movement_speed, float zoom) {
    glm_vec_copy(position, cam->position);
    cam->movement_speed = movement_speed;
    cam->zoom = zoom;

}

void translateCamera(struct Camera *cam, enum camera_movement direction, float delta_time) {
    float velocity = (cam->movement_speed + 2.0f * cam->movement_speed) * delta_time;

    if(direction == cam_left) {
        cam->position[0] -= velocity;
    } 

    if(direction == cam_right) {
        cam->position[0] += velocity;
    }

    if(direction == cam_up) {
        cam->position[1] += velocity;
    }

    if(direction == cam_down) {
        cam->position[1] -= velocity;
    }
}

void zoomCamera(struct Camera *cam, float y_offset) {
    //can move these constraints to constants
    if(cam->zoom >= 1.0f && cam->zoom <= 45.0f)
        cam->zoom -= y_offset;
    if(cam->zoom <= 1.0f)
        cam->zoom = 1.0f;
    if(cam->zoom >= 45.0f)
        cam->zoom = 45.0f;
}
