#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stdbool.h>
#include <string.h>
#include <errno.h>
#include <math.h>


#include "glad.c"

#define GLFW_INCLUDE_GLEXT
#include <GLFW/glfw3.h>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#define DEFAULT_SCREEN_WIDTH 1600
#define DEFAULT_SCREEN_HEIGHT 900


int file_size(FILE *file, size_t *size) {
    long saved = ftell(file);
    if (saved < 0) return -1;
   
    if (fseek(file, 0, SEEK_END) < 0) return-1;
    long result = ftell(file);
    if (result < 0) return -1;  
    if (fseek(file, saved, SEEK_SET) < 0) return -1;

    *size = (size_t) result;
    return 0;
}

// Function to read the shader file
char *slurp_file_into_malloced_cstr(const char *file_path) {
    // Defining variables
    FILE *f = NULL;
    
    char *buffer;
    
    f = fopen(file_path, "r");


    if (f == NULL) {
        return NULL;
    }
    
    size_t size; 
    file_size(f, &size);
     
    // Check for empty files
    if (size == 0) {
        int saved_errno = errno;
        fclose(f);
        errno = saved_errno;

        return NULL;
    }

    if (fseek(f, 0, SEEK_END) < 0) { 
        int saved_errno = errno;
        fclose(f);
        errno = saved_errno;
        return NULL;
    }
   
    // Set memory for the size of the file 
    buffer = malloc(size + 1);
    
    if (buffer == NULL) {

        int saved_errno = errno;
        fclose(f);
        errno = saved_errno;

        return NULL;

    }
 
    if (fseek(f, 0, SEEK_SET) < 0) {     
        int saved_errno = errno;
        fclose(f);
        errno = saved_errno;
        free(buffer);
        return NULL;
    }
   
  
    // Reading and copying the file in the buffer 
    fread(buffer, 1, size, f);

    if (ferror(f)) {
        int saved_errno = errno;
        fclose(f);
        errno = saved_errno; 
        free(buffer);
        return NULL;
    }
    
    // Add \0 to the end of the buffer 
    buffer[size] = '\0'; 
    fclose(f);

    // Return the buffer 
    return buffer;
 }

// Compiling the shaders
bool compile_shader_source(const GLchar *source, GLenum shader_type, GLuint *shader) {
    *shader = glCreateShader(shader_type);
    glShaderSource(*shader, 1, &source, NULL);
    glCompileShader(*shader);

    GLint compiled = 0;
    glGetShaderiv(*shader, GL_COMPILE_STATUS, &compiled);
    
    if (!compiled) {
        GLchar message[1024];
        GLsizei message_size;
        glGetShaderInfoLog(*shader, sizeof(message), &message_size, message);
        fprintf(stderr, "ERROR: could not compile %c\n", shader_type);
        fprintf(stderr, "%.*s\n", message_size, message);
        return false;
    }

    return true;
}

// Compile the shader by the file
bool compile_shader_file(const char *file_path, GLenum shader_type, GLuint *shader) {
    char *source = slurp_file_into_malloced_cstr(file_path);
    if (source == NULL) {
        fprintf(stderr, "ERROR: failed to read file `%s`: %s\n", file_path, strerror(errno));
        errno = 0;
        return false;
    }
    bool ok = compile_shader_source(source, shader_type, shader);
    if (!ok) {
        fprintf(stderr, "ERROR: failed to compile `%s` shader file\n", file_path);
    }
    free(source);
    return ok;
}

// Link the program shader with the shaders specified
bool link_program(GLuint *shaders, size_t shaders_count,  GLuint *program) {
    
    *program = glCreateProgram();
    
    for (size_t i = 0; i < shaders_count; ++i) {
        glAttachShader(*program, shaders[i]);
    }

    glLinkProgram(*program);

    GLint linked = 0;
    glGetProgramiv(*program, GL_LINK_STATUS, &linked);
    if (!linked) {
        GLsizei message_size = 0;
        GLchar message[1024];

        glGetProgramInfoLog(*program, sizeof(message), &message_size, message);
        fprintf(stderr, "Program Linking: %.*s\n", message_size, message);
    }
    
    // Clean the shaders from the memory
    for (size_t i = 0; i < shaders_count; ++i) {
        glDeleteShader(shaders[i]);
    }

    return program;
}

// Function called to create and sync the shader program
bool load_shader_program(const char *vertex_file_path,
                         const char *fragment_file_path,
                         GLuint *program) {
    GLuint vert = 0;
    if (!compile_shader_file(vertex_file_path, GL_VERTEX_SHADER, &vert)) {
        return false;
    }

    GLuint frag = 0;
    if (!compile_shader_file(fragment_file_path, GL_FRAGMENT_SHADER, &frag)) {
        return false;
    }

    GLuint shaders[2] = {vert, frag};
    if (!link_program(shaders, 2, program)) {
        return false;
    }

    return true;
}


void load_texture(GLuint *texture, char *image_path) {
    
    int width, height, nrChannels;

    unsigned char *data = stbi_load(image_path, &width, &height, &nrChannels, 0);
    
    glGenTextures(1, texture);    
    glBindTexture(GL_TEXTURE_2D, *texture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);	
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    
    if (data) {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
    } else {
         fprintf(stderr, "ERROR: could not load image %s: %s\n",
                image_path, strerror(errno));
        return;
    }

    stbi_image_free(data);
}

void load_texture_rgba(GLuint *texture, char *image_path) {
    
    int width, height, nrChannels;

    unsigned char *data = stbi_load(image_path, &width, &height, &nrChannels, 0);
    
    glGenTextures(1, texture);    
    glBindTexture(GL_TEXTURE_2D, *texture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);	
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    
    if (data) {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
    } else {
         fprintf(stderr, "ERROR: could not load image %s: %s\n",
                image_path, strerror(errno));
        return;
    }

    stbi_image_free(data);
}


void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
  glViewport(0, 0, width, height);
}

void processInput(GLFWwindow* window) {
  if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
    glfwSetWindowShouldClose(window, true);
}

int main(void) { 
    if (!glfwInit()) {
        fprintf(stderr, "ERROR: could not initialize GLFW\n");
        exit(1);
    }

    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    
    GLFWwindow * const window = glfwCreateWindow(
            DEFAULT_SCREEN_WIDTH, 
            DEFAULT_SCREEN_HEIGHT, 
            "Texture Units", 
            NULL, 
            NULL);
    
    if (window == NULL) {
        fprintf(stderr, "ERROR: the window could not be created");
        glfwTerminate();
        exit(1);
    }

    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        fprintf(stderr, "ERROR: could not initialize GLAD\n");
        exit(1);
    }
    printf("Opengl used in this platform (%s): \n", glGetString(GL_VERSION));
    glViewport(0, 0 , DEFAULT_SCREEN_WIDTH, DEFAULT_SCREEN_HEIGHT);
    
    const char *vertex_file_path = "shaders/texture-unit/vertex.vert";
    const char *fragment_file_path = "shaders/texture-unit/color.frag";
    GLuint program;

    if (!load_shader_program(vertex_file_path, fragment_file_path, &program)) {
        exit(1);
    }

    glUseProgram(program);
     
    GLuint VAO, VBO, EBO, texture1, texture2;
    float vertices[] = {
        // positions          // colors           // texture coords
        0.5f,  0.5f, 0.0f,   1.0f, 0.0f, 0.0f,   1.0f, 1.0f,   // top right
        0.5f, -0.5f, 0.0f,   0.0f, 1.0f, 0.0f,   1.0f, 0.0f,   // bottom right
        -0.5f, -0.5f, 0.0f,   0.0f, 0.0f, 1.0f,   0.0f, 0.0f,   // bottom left
        -0.5f,  0.5f, 0.0f,   1.0f, 1.0f, 0.0f,   0.0f, 1.0f    // top left 
    };

    GLuint indices[] = {
        0, 1, 3, // first triangle 
        1, 2, 3  // second triangle
    };

    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);
    char *image_path = "assets/container.jpg";
    char *image_path2 = "assets/awesomeface.png";
    load_texture(&texture1, image_path);
    stbi_set_flip_vertically_on_load(true);  
    load_texture_rgba(&texture2, image_path2);


    glBindVertexArray(VAO);

    
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
     
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
    
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

 
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3* sizeof(float)));
    glEnableVertexAttribArray(1);
 
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6* sizeof(float)));
    glEnableVertexAttribArray(2);



    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
    
    glUseProgram(program);
    glUniform1i(glGetUniformLocation(program, "texture1"), 0);
 
    glUniform1i(glGetUniformLocation(program, "texture2"), 1);


    while(!glfwWindowShouldClose(window)) {
        // input commands
        processInput(window);

        // Rendering commands here
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, texture1); 
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, texture2);
 
        glUseProgram(program);       
        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

        // Check and call events and swap the buffers
        glfwSwapBuffers(window);
        glfwPollEvents();

    }

    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteProgram(program);
    
    glfwTerminate();
    return 0;
}
