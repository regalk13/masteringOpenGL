#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stdbool.h>
#include <string.h>
#include <errno.h>

#include "glad.c"

#define GLFW_INCLUDE_GLEXT
#include <GLFW/glfw3.h>

#define DEFAULT_SCREEN_WIDTH 1600
#define DEFAULT_SCREEN_HEIGHT 900

// Vertex shader source
// this scope of GLSL code can modify the mutable parts of a vertex 
// in this case we just define the position of the vertices, returning a vector of 3 dimensions
const char *vertexShaderSource = "#version 330 core\n"
    "layout (location = 0) in vec3 aPos;\n"
    "void main()\n"
    "{\n"
    "   gl_Position = vec4(aPos.x, aPos.y, aPos.z, 1.0);\n"
    "}\0";

// Fragment shader source
// as the vertex shader in the render pipeline the fragment shader is a modify stage
// when the vertices are converted in fragments (pixels) we can modify their color and some attributes
const char *fragmentShaderSource = "#version 330 core\n"
  "out vec4 FragColor;\n" 
  "void main()\n"
  "{\n"
  "   FragColor = vec4(1.0f, 0.5f, 0.2f, 1.0f);\n"
  "}\0";

// Function to control the resize of the window and adjust the content with it 
void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
  glViewport(0, 0, width, height);
}

// Controlling the user input with GLFW
void processInput(GLFWwindow* window) {
  if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
    glfwSetWindowShouldClose(window, true);
}

int main(void) {
    // First to all we need to start the window context with glfw in this case 
    if (!glfwInit()) {
        fprintf(stderr, "ERROR: could not initialize GLFW\n");
        exit(1);
    }
    // Configuring and creating the window 
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    
    GLFWwindow * const window = glfwCreateWindow(
            DEFAULT_SCREEN_WIDTH, 
            DEFAULT_SCREEN_HEIGHT, 
            "Rectangle", 
            NULL, 
            NULL);
    
    // Checking errors in window creation 
    if (window == NULL) {
        fprintf(stderr, "ERROR: the window could not be created");
        glfwTerminate();
        exit(1);
    }

    // Create context and sset function of frame buffer resize
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    
    // load opengl via Glad 
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        fprintf(stderr, "ERROR: could not initialize GLAD\n");
        exit(1);
    }
    // Print the version used in the system of opengl 
    printf("Opengl used in this platform (%s): \n", glGetString(GL_VERSION));
    // Set viewport and max size OpenGL can use of the context display, in this case all the window 
    glViewport(0, 0, DEFAULT_SCREEN_WIDTH, DEFAULT_SCREEN_HEIGHT);

    // Vertex Shaders
    unsigned int vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
    glCompileShader(vertexShader);
    
    // Check for shader compile errors 
    int success; 
    char infoLog[512];
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
    if (!success) {
       glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
       fprintf(stderr, "ERROR: shader vertex compilation failed: %c\n", infoLog);
     }
    
    // Fragment Shaders
    unsigned int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
    glCompileShader(fragmentShader);
    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
    // Check for shader compile errors
    if (!success) {
      glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog); 
      fprintf(stderr, "ERROR: fragment shader compilation failed: %c\n", infoLog);
    }
     
    // Link shaders, shader program
    unsigned int shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);
    // check linking errors
    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);

    if (!success) {
      glGetShaderInfoLog(shaderProgram, 512, NULL, infoLog); 
      fprintf(stderr, "ERROR: shader program link failed\n");
    }

    // Delete shaders don't needed the program now have the memory itself
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);
    
    // Setup vertices, just use not overlap vertices once
    float vertices[] = {
        0.5f,  0.5f, 0.0f,  // top right
        0.5f, -0.5f, 0.0f,  // bottom right
        -0.5f, -0.5f, 0.0f,  // bottom left
        -0.5f,  0.5f, 0.0f 
    }; 

    unsigned int indices[] = {  // note that we start from 0!
        0, 1, 3,   // first triangle
        1, 2, 3    // second triangle
    };
    
    unsigned int VAO, VBO, EBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    // Bind the vertex array object, then bind and set the configuration in the buffer 
    glBindVertexArray(VAO);
    
    // Binding VBO and EBO this will be saved in VAO 
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    
    
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
    
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glBindBuffer(GL_ARRAY_BUFFER, 0);

     // You can unbind the VAO afterwards so other VAO calls won't accidentally modify this VAO, but this rarely happens. Modifying other
    // VAOs requires a call to glBindVertexArray anyways so we generally don't unbind VAOs (nor VBOs) when it's not directly necessary.
    // You will need bind again before draw command
    glBindVertexArray(0);
    
    // Wireframe mode
    // glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

    while(!glfwWindowShouldClose(window)) {
      // input commands
        processInput(window);

        // Rendering commands here
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        glUseProgram(shaderProgram);
        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
        
        // Check and call events and swap the buffers
        glfwSwapBuffers(window);
        glfwPollEvents();

    }
    // de-allocagte all resources, in this case this is optional but if you have multplie functions or rendering again after this
    // it's necessary to control the memory
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &EBO);
    glDeleteProgram(shaderProgram);
    
    // Terminate GLFW resources 
    glfwTerminate();
    return 0;
}
