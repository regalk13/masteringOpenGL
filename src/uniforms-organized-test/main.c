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

#define DEFAULT_SCREEN_WIDTH 1600
#define DEFAULT_SCREEN_HEIGHT 900

char *slurp_file_into_malloced_cstr(const char *file_path) {
    FILE *f = NULL;
    char *buffer = NULL;

    f = fopen(file_path, "r");
    if (f == NULL) goto fail;
    if (fseek(f, 0, SEEK_END) < 0) goto fail;

    long size = ftell(f);
    if (size < 0) goto fail;

    buffer = malloc(size + 1);
    if (buffer == NULL) goto fail;

    if (fseek(f, 0, SEEK_SET) < 0) goto fail;

    fread(buffer, 1, size, f);
    if (ferror(f)) goto fail;

    buffer[size] = '\0';

    if (f) {
        fclose(f);
        errno = 0;
    }
    return buffer;
    fail:
    if (f) {
        int saved_errno = errno;
        fclose(f);
        errno = saved_errno;
    }
    if (buffer) {
        free(buffer);
    }
    return NULL;
}


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
    

    // glDeleteShader(vert_shader);
    // glDeleteShader(frag_shader);

    return program;
}

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
            "Two triangles", 
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

 
    const char *vertex_file_path = "shaders/uniforms/vertex.vert";
    const char *fragment_file_path = "shaders/uniforms/color.frag";
    const char *fragment_file_path2 = "shaders/uniforms/color2.frag";
    GLuint program;

    if (!load_shader_program(vertex_file_path, fragment_file_path, &program)) {
        exit(1);
    }
   
    GLuint program2;

    if (!load_shader_program(vertex_file_path, fragment_file_path2, &program2)) {
        exit(1);
    }

   unsigned int VAO, VAO2, VBO, VBO2;

    float vertices[] = {
    -0.9f, -0.5f, 0.0f,
     -0.0f, -0.5f, 0.0f,
     -0.45f,  0.5f, 0.0f  
    }; 
    
    float vertices2[] = {
     0.5f, -0.5f, 0.0f,
     0.9f, -0.5f, 0.0f,
     0.45f,  0.5f, 0.0f  
    }; 

    glGenVertexArrays(1, &VAO);
    glGenVertexArrays(1, &VAO2);

    glGenBuffers(1, &VBO);
    glGenBuffers(1, &VBO2);

    glBindVertexArray(VAO);

    
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
       
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    glBindVertexArray(VAO2);


    glBindBuffer(GL_ARRAY_BUFFER, VBO2);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices2), vertices2, GL_STATIC_DRAW);
  
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    while(!glfwWindowShouldClose(window)) {
        // input commands
        processInput(window);

        // Rendering commands here
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        
        float timeValue = glfwGetTime();
        float greenValue = (sin(timeValue) / 2.0f) + 0.5f;
        int vertexColorLocation = glGetUniformLocation(program, "vertexColor1");
        glUseProgram(program);
        glUniform4f(vertexColorLocation, 0.0f, greenValue, 0.0f, 1.0f);
        
        glBindVertexArray(VAO);
        glDrawArrays(GL_TRIANGLES, 0, 3);
        glBindVertexArray(0);

        int vertexColorLocation2 = glGetUniformLocation(program2, "vertexColor2");
        glUseProgram(program2);
        glUniform4f(vertexColorLocation2, greenValue, 0.0f, 0.0f, 1.0f);
        

        glBindVertexArray(VAO2);
        glDrawArrays(GL_TRIANGLES, 0, 3);
        glBindVertexArray(0);


        // Check and call events and swap the buffers
        glfwSwapBuffers(window);
        glfwPollEvents();

    }

    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteProgram(program);
    glDeleteProgram(program2);
    
    glfwTerminate();
    return 0;
}
