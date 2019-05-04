
#include <stdio.h>
#include <math.h>

#include <glad/glad.h>              //defines opengl functions, etc
#include <GLFW/glfw3.h>             //used for window and input
#include <cglm/cglm.h>              //used for maths

#define STB_IMAGE_IMPLEMENTATION

#include "shader.h"
#include "camera.h"
#include "sprite.h"
#include "texman.h"
#include "phys.h"
#include "list.h"
#include "const.h"

//macros
#define degToRad(deg) ((deg) * M_PI / 180.0)
#define radToDeg(rad) ((rad) * 180.0 / M_PI)

void framebuffer_size_callback(GLFWwindow *window, int width, int height);
void processInput(GLFWwindow *window, struct Camera *cam, float dt);
void mouse_callback(GLFWwindow* window, double x_pos, double y_pos);
void scroll_callback(GLFWwindow* window, double x_offset, double y_offset);
void glfw_error_callback(int code, const char *err_str);
GLFWwindow *initializeWindow();
void updateDefaultUniforms(struct Shader *shader, struct Camera *cam);


float delta_time = 0.0f;
float last_frame = 0.0f;

float last_mouse_x = 400;
float last_mouse_y = 300;
uint8_t first_mouse = 1;

uint8_t draw_wireframe = 0;
uint8_t t_pressed = 0;

struct Camera cam;

struct Circle *mouse;

int fps_limit = 144;

float spawn_rate = 1; // how many circles to spawn per second
float last_spawn_time = 0;

int main() {
    printf("running!\n");

    //initialize window
    GLFWwindow *window = initializeWindow();
    //load the opengl library
    if(!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        printf("Failed to initiate GLAD\n");
        return -1;
    }
    //enable depth test
    glEnable(GL_DEPTH_TEST);
    
    //initialize the camera
    initializeCamera(&cam, (vec2){0.0f, 0.0f}, 50.0f, 45.0f);

    // initialize default shader
    struct Shader shader;
    if(!initializeShader(&shader, "shaders/sprite_vs.glsl", "shaders/sprite_fs.glsl")) {
        printf("Error initializing shaders\n");
        exit(1);
    }

    // tell the shader which texture to use for which uniform
    // TODO: not sure where to put this...
    // used to be in model.init, but only needs to be called once!
    glUseProgram(shader.id);
    setInt(&shader, "image", 0);

    // initialize the texture manager
    struct TexMan texman;
    initTexMan(&texman);

    //keep track of FPS
    uint64_t total_frames = 0;
    float start_time = glfwGetTime();



    // ************* CIRCLE STUFF ************
    initPhysRenderer(&texman, &shader);

    struct List objects;
    initList(&objects);

    // add the mouse
    mouse = (struct Circle *)(addCircle(&objects, SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2, 0, 0, 20, 0))->data;
    // still objects
    addRect(&objects, 20, 100, 20, SCREEN_HEIGHT - 200);   // left box
    addRect(&objects, SCREEN_WIDTH - 40, 100, 20, SCREEN_HEIGHT - 200);   // right box
    addRect(&objects, 150, SCREEN_HEIGHT - 150, SCREEN_WIDTH - 300, 100);   // center box
    addCircle(&objects, SCREEN_WIDTH / 2, SCREEN_HEIGHT - 125, 0, 0, 100, 0);
    addCircle(&objects, SCREEN_WIDTH / 4, SCREEN_HEIGHT / 2, 0, 0, 75, 0);
    addCircle(&objects, 3 * SCREEN_WIDTH / 4, SCREEN_HEIGHT / 2, 0, 0, 75, 0);

    last_spawn_time = glfwGetTime();


    //Main loop
    while(!glfwWindowShouldClose(window)) {
        //wait for max FPS limit
        float min_frame_time = (float)(1 / (float)fps_limit);
        while(glfwGetTime() - last_frame < min_frame_time);

        //update time since last frame
        float current_frame = glfwGetTime();
        delta_time = current_frame - last_frame;
        last_frame = current_frame;
        total_frames ++;

        //process inputs
        processInput(window, &cam, delta_time);

        //rendering commands here
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // update camera uniforms
        updateDefaultUniforms(&shader, &cam);

        // update physics
        updatePhysics(&objects, delta_time);
        drawObjects(&objects);

        // spawn new circles
        float spawn_period = 1 / spawn_rate;
        if(glfwGetTime() - last_spawn_time > spawn_period) {
            while(glfwGetTime() - last_spawn_time > spawn_period) {
                last_spawn_time +=  spawn_period;
                int x_var = 800;
                int y_var = 100;
                x_var = rand() % x_var - x_var / 2;
                y_var = rand() % y_var - y_var / 2;
                addCircle(&objects, SCREEN_WIDTH / 2 + x_var, 100 + y_var, 0, 0, 10, 1);
            }
            last_spawn_time = glfwGetTime();
        }

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    destroyTexMan(&texman);

    printf("End of program\n\tframes: %I64d\n\tTime: %f\n\tFPS: %f", total_frames, glfwGetTime() - start_time, total_frames / (glfwGetTime() - start_time));

    glfwTerminate();
    return 0;
}

//should definitely use key callback for this
//  keycallback insures that will we handle the input
//  even if they release the key before we process the input
//  in the loop
void processInput(GLFWwindow *window, struct Camera *cam, float dt) {
    int escape = glfwGetKey(window, GLFW_KEY_ESCAPE);
    int w = glfwGetKey(window, GLFW_KEY_W);
    int a = glfwGetKey(window, GLFW_KEY_A);
    int s = glfwGetKey(window, GLFW_KEY_S);
    int d = glfwGetKey(window, GLFW_KEY_D);
    int t = glfwGetKey(window, GLFW_KEY_T);
    
    int p = glfwGetKey(window, GLFW_KEY_P);
    int f = glfwGetKey(window, GLFW_KEY_F);
    int up = glfwGetKey(window, GLFW_KEY_UP);
    int dn = glfwGetKey(window, GLFW_KEY_DOWN);

    //quit when escape is pressed
    if(escape == GLFW_PRESS)
        glfwSetWindowShouldClose(window, 1U);


    float acl = 1;
    if(a == GLFW_PRESS) {
        //translateCamera(cam, cam_left, delta_time);
        mouse->vel.x -= acl * dt;
    }
    if(d == GLFW_PRESS) {
        //translateCamera(cam, cam_right, delta_time);
        mouse->vel.x += acl * dt;
    }
    if(w == GLFW_PRESS) {
        //translateCamera(cam, cam_up, delta_time);
        mouse->vel.y -= acl * dt;
    }
    if(s == GLFW_PRESS) {
        //translateCamera(cam, cam_down, delta_time);
        mouse->vel.y += acl * dt;
    }

    if(t == GLFW_PRESS && !t_pressed) {
        t_pressed = 1;
        draw_wireframe = ~draw_wireframe;
        if(draw_wireframe) {
            // draw wireframes
            glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        }
        else {
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        }
    }
    else if(t == GLFW_RELEASE) {
        t_pressed = 0;
    }

    static double press_time = 0;
    if(p == GLFW_PRESS && glfwGetTime() - press_time > 1) {
        printf("c: x: %.2f\ty: %.2f\n", mouse->pos.x, mouse->pos.y);
        printf("p pressed\n");
        fflush(stdout);
        press_time = glfwGetTime();
    }

    if(f == GLFW_PRESS && glfwGetTime() - press_time > 1) {
        printf("Switching Frame Limit\n");
        press_time = glfwGetTime();
        if(fps_limit == 144) {
            printf("switching to 60\n");
            fps_limit = 60;
        }
        else if(fps_limit == 60) {
            printf("switching to 30\n");
            fps_limit = 30;
        }
        else if(fps_limit == 30) {
            printf("switching to 144\n");
            fps_limit = 144;
        }
        fflush(stdout);
    }

    if(up == GLFW_PRESS) {
        spawn_rate += 0.1;
    }

    if(dn == GLFW_PRESS) {
        spawn_rate -= 0.1;
        if(spawn_rate <= 0) {
            spawn_rate = 0;
        }
    }

}

void mouse_callback(GLFWwindow* window, double x_pos, double y_pos) {
    //prevent sudden camera jump when mouse enters window
    if(first_mouse) {
        last_mouse_x = x_pos;
        last_mouse_y = y_pos;
        first_mouse = 0;
    }

    float dx = x_pos - last_mouse_x;
    float dy = y_pos - last_mouse_y;
    last_mouse_x = x_pos;
    last_mouse_y = y_pos;

    mouse->pos.x += dx;
    mouse->pos.y += dy;
}

void scroll_callback(GLFWwindow* window, double x_offset, double y_offset) {
    zoomCamera(&cam, y_offset);
}

void framebuffer_size_callback(GLFWwindow *window, int width, int height) {
    glViewport(0, 0, width, height);
}

void glfw_error_callback(int code, const char *err_str) {
    printf("GLFW error: \n\tcode: 0x%x\n\t%s\n", code, err_str);
}

GLFWwindow *initializeWindow() {
    glfwSetErrorCallback(glfw_error_callback);

    if(!glfwInit()) {
        printf("error initializing GLFW\n");
        exit(1);
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow *window = glfwCreateWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "TITLE", NULL, NULL);

    if(window == NULL) {
        printf("Error creating window\n");
        glfwTerminate();
        exit(1);
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);

    return window;
}

// name WIP
// all models using default shader have these same uniforms. So just update them
// all here!
void updateDefaultUniforms(struct Shader *shader, struct Camera *cam) {
    // construct matrices for camera
    mat4 view, projection;
    glm_translate_make(view, (vec3){-1 * cam->position[0], cam->position[1], 0.0f});
    // This is where zoom would be incorporated (maybe??)
    glm_ortho(0.0, SCREEN_WIDTH, SCREEN_HEIGHT, 0.0, -1.0, 1.0, projection);
    setMat4(shader, "view", view);
    setMat4(shader, "projection", projection);
}
