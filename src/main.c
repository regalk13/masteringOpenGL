#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stdbool.h>
#include <string.h>
#include <errno.h>
#include <math.h>

#define GLFW_INCLUDE_GLEXT
#include <GLFW/glfw3.h>

#define DEFAULT_SCREEN_WIDTH 1600
#define DEFAULT_SCREEN_HEIGHT 900

#include "gloader.c"

int main(void) {
    
    if (!glfwInit()) {
        fprintf(stderr, "ERROR: could not initilize GLFW\n");
        exit(1);
    }

    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    
    GLFWwindow * const window = glfwCreateWindow(
            DEFAULT_SCREEN_WIDTH, 
            DEFAULT_SCREEN_HEIGHT, 
            "Fractal", 
            NULL, 
            NULL);
    
    if (window == NULL) {
        fprintf(stderr, "ERROR: the window could not be created");
        glfwTerminate();
        exit(1);
    }
    glfwMakeContextCurrent(window);
    printf("Hello World!\n");
    return 0;
}
