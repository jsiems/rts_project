
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
#include "circle.h"

//macros
#define degToRad(deg) ((deg) * M_PI / 180.0)
#define radToDeg(rad) ((rad) * 180.0 / M_PI)

#define SCREEN_WIDTH 800
#define SCREEN_HEIGHT 600

#define FPS_LIMIT 144.0f

void framebuffer_size_callback(GLFWwindow *window, int width, int height);
void processInput(GLFWwindow *window, struct Camera *cam);
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

struct Circle c, c2;

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

    // ************* CIRCLE STUFF ************
    initCircleRenderer(&texman, &shader);
    initCircle(&c, 100, 100, 0, 0, 20);
    initCircle(&c2, 300, 300, 0, 0, 25);

    //keep track of FPS
    uint64_t total_frames = 0;
    float start_time = glfwGetTime();
    float min_frame_time = 1 / FPS_LIMIT;


    float deg = 0;


    //Main loop
    while(!glfwWindowShouldClose(window)) {
        //wait for max FPS limit
        while(glfwGetTime() - last_frame < min_frame_time);

        //update time since last frame
        float current_frame = glfwGetTime();
        delta_time = current_frame - last_frame;
        last_frame = current_frame;
        total_frames ++;

        //process inputs
        processInput(window, &cam);

        //rendering commands here
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // update camera uniforms
        updateDefaultUniforms(&shader, &cam);


        // DRAW MUH CIRC
        float r = 150;
        deg += delta_time * 400;
        if(deg > 360) deg = 0;
        c2.pos.x = 400 + r * cos(degToRad(deg));
        c2.pos.y = 300 + r * sin(degToRad(deg));
        updateCircle(&c, delta_time);
        updateCircle(&c2, delta_time);
        if(isColliding(&c, &c2)) {
            c.color = (struct v3){1.0f, 0.5f, 0.5f};
        }
        else {
            c.color = (struct v3){1.0f, 1.0f, 1.0f};
        }
        drawCircle(&c);
        drawCircle(&c2);



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
void processInput(GLFWwindow *window, struct Camera *cam) {
    int escape = glfwGetKey(window, GLFW_KEY_ESCAPE);
    int w = glfwGetKey(window, GLFW_KEY_W);
    int a = glfwGetKey(window, GLFW_KEY_A);
    int s = glfwGetKey(window, GLFW_KEY_S);
    int d = glfwGetKey(window, GLFW_KEY_D);
    int t = glfwGetKey(window, GLFW_KEY_T);

    int p = glfwGetKey(window, GLFW_KEY_P);

    //quit when escape is pressed
    if(escape == GLFW_PRESS)
        glfwSetWindowShouldClose(window, 1U);


    if(a == GLFW_PRESS) {
        translateCamera(cam, cam_left, delta_time);
    }
    if(d == GLFW_PRESS) {
        translateCamera(cam, cam_right, delta_time);
    }
    if(w == GLFW_PRESS) {
        translateCamera(cam, cam_up, delta_time);
    }
    if(s == GLFW_PRESS) {
        translateCamera(cam, cam_down, delta_time);
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
        printf("c: x: %.2f\ty: %.2f\tr: %f\n", c.pos.x, c.pos.y, c.radius);
        printf("c2: x: %.2f\ty: %.2f\tr: %f\n", c2.pos.x, c2.pos.y, c2.radius);
        printf("distance: %.2f\n", distCirc(c.pos.x, c.pos.y, c2.pos.x, c2.pos.y));
        fflush(stdout);
        press_time = glfwGetTime();
    }

}

void mouse_callback(GLFWwindow* window, double x_pos, double y_pos) {
    //prevent sudden camera jump when mouse enters window
    if(first_mouse) {
        last_mouse_x = x_pos;
        last_mouse_y = y_pos;
        first_mouse = 0;
    }

    last_mouse_x = x_pos;
    last_mouse_y = y_pos;

    c.pos.x = x_pos;
    c.pos.y = y_pos;
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
